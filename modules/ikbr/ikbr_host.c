#include <linux/kref.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/timer.h>
#include "ik_hashtable.h"

struct ikbr_host {
	struct ht_elem ikh_hte;
	uint8_t ikh_mac[6];
	__be32 ikh_ip;
	unsigned long access_time;
	struct kref ikh_kref;
	bool ikh_is_authed;
	unsigned long ikh_auth_timeout;
};

static struct hashtab *g_ikh_htp;
static struct timer_list g_ikh_gc_timer;

static int ikbr_host_cmp(struct ht_elem *htep, void *key)
{
	struct ikbr_host *h = NULL;
	uint8_t *mac = key;

	h = container_of(htep, struct ikbr_host, ikh_hte);
	return (mac[4] == h->ikh_mac[4] && mac[5] == h->ikh_mac[5]);
}

struct ikbr_host *__ikbr_host_alloc_init(uint8_t *mac, __be32 ip, gfp_t flags)
{
	struct ikbr_host *host = NULL;
	struct ht_elem *hte = NULL;
	unsigned long hash = 0;

	if (!mac)
		return NULL;

	hash = mac2hash(mac);

	hashtab_lock_mod(g_ikh_htp, hash);
	/* Maybe added somewhere else, so find it first without kref_get */
	hte = hashtab_lookup(g_ikh_htp, hash, mac);
	if (hte) {
		host = container_of(hte, struct ikbr_host, ikh_hte);
		goto end;
	}

	host = kmalloc(sizeof(*host), flags);
	if (!host)
		goto end;

	kref_init(&host->ikh_kref);

	host->ikh_hte.hte_hash = hash;
	memcpy(&host->ikh_mac, mac, ETH_ALEN);
	host->ikh_ip = ip;

	hashtab_add(g_ikh_htp, hash, &host->ikh_hte);

	pr_info("[ikbr host]: alloc ikh with ip:%pI4 mac: %pM\n", &host->ikh_ip, host->ikh_mac);
end:
	hashtab_unlock_mod(g_ikh_htp, hash);
	return host;
}

inline struct ikbr_host *ikbr_host_alloc_init(struct sk_buff *skb, gfp_t flags)
{
	struct iphdr *iph = ip_hdr(skb);
	struct ethhdr *ethh = eth_hdr(skb);

	return __ikbr_host_alloc_init(ethh->h_source, iph->saddr, flags);
}

void ikbr_host_release(struct ikbr_host *host)
{
	kfree(host);
}

static void ikbr_host_release_kref(struct kref *kref)
{
	struct ikbr_host *host = container_of(kref, struct ikbr_host, ikbr_host_kref);

	hashtab_lock_mod(g_ikh_htp, host->ikh_hte.hte_hash);
	hashtab_del(&host->ikh_hte);
	hashtab_unlock_mod(g_ikh_htp, host->ikh_hte.hte_hash);

	synchronize_rcu();
	ikbr_host_release(host);
}

inline void ikbr_host_get(struct ikbr_host *h)
{
	kref_get(&h->ikh_kref);
}

static inline int __must_check ikbr_host_get_unless_zero(struct ikbr_host* h)
{
	return kref_get_unless_zero(&h->ikh_kref);
}

inline int ikbr_host_put(struct ikbr_host *h)
{
	return kref_put(&h->ikh_kref, ikh_release_kref);
}

struct ikbr_host* ikbr_host_get_by_mac(uint8_t* mac)
{
	unsigned long mac_hash = mac2hash(mac);
	struct ht_elem *htep = NULL;
	struct ikbr_host *h = NULL;

	hashtab_lock_lookup(g_ikh_htp, mac_hash);
	htep = hashtab_lookup(g_ikh_htp, mac_hash, (void *)mac);
	if (!htep)
		goto end;
	h = container_of(htep, struct ikbr_host, ikh_hte);

	if (!ikbr_host_get_unless_zero(h))
		h = NULL;
end:
	hashtab_unlock_lookup(g_ikh_htp, mac_hash);
	return h;
}

int ikbr_host_iter_by_ip_helper(struct ht_elem *hte, void *data)
{
	struct ikbr_host *host = container_of(hte, struct ikbr_host, ikh_hte);
	return host->ikh_ip == *(__be32*)data;
}

struct ikbr_host* ikbr_host_get_by_ip(__be32 ip)
{
	uint32_t bkt_idx = 0;
	struct ikbr_host* h = NULL;
	struct ht_elem *htep = NULL;

	hashtab_lock_lookup(g_ikh_htp, 0);
	uint8_t *mac = key;
	htep = hashtab_iter_all(g_ikh_htp, ikbr_host_iter_by_ip_helper, &ip, &bkt_idx);
	if (!htep)
		goto end;
	h = container_of(htep, struct ikbr_host, ikh_hte);

	if (!ikbr_host_get_unless_zero(h))
		h = NULL;

end:
	hashtab_unlock_lookup(g_ikh_htp, 0);
	return h;
}

void ikbr_host_touch(struct ikbr_host *h, struct sk_buff *skb)
{
	if (!h || !skb)
		return;
	h->access_time = jiffies;
}

#define IKH_GC_INTERVAL (5 * HZ)
#define IKH_GC_MAX_HOST_ONCE (1024)
#define IKH_GC_TIMEOUT (5 * 60 * HZ)

int ikbr_host_gc_helper(struct ht_elem *hte, void *data)
{
	struct ikbr_host *host = container_of(hte, struct ikbr_host, ikh_hte);

	if (time_after(host->access_time + IKH_GC_TIMEOUT, jiffies))
		return 1;
	if (atomic_read(&host->ikh_kref.refcount) < 2)
		ikbr_host_put(host);
	return 0;
}

static uint32_t g_btk_idx;

void ikbr_host_gc(unsigned long data)
{
	g_btk_idx %= g_ikh_htp->ht_nbuckets;

	hashtab_iter_one_bucket(g_ikh_htp, ikbr_host_gc_helper, NULL, g_btk_idx);
	g_btk_idx++;

	mod_timer(&g_ikbr_host_gc_timer, jiffies + IKH_GC_INTERVAL);
}

#define IKH_TEST
#ifdef IKH_TEST
#define IKH_MAX_NBUCKETS 512

static int __init ikbr_host_init(void)
{
	g_ikh_htp = hashtab_alloc(IKH_MAX_NBUCKETS, ikh_cmp);
	setup_timer(&g_ikh_gc_timer, ikh_gc, 0);
	mod_timer(&g_ikh_gc_timer, jiffies + IKH_GC_INTERVAL);

	return 0;
}

static void __exit ikbr_host_exit(void)
{
	del_timer_sync(&g_ikh_gc_timer);
	hashtab_free(g_ikh_htp);
}

module_init(ikbr_host_init);
module_exit(ikbr_host_exit);
#endif
