#include <linux/init.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/ip.h>
#include <linux/if_ether.h>
#include "hashtable.h"

struct ikbr_host {
	struct ht_elem ikh_hte;
	struct kref ikh_kref;
	uint8_t ikh_mac[ETH_ALEN];
	__be32 ikh_ip;
};

struct hashtab *g_ikbr_host_htp;

unsigned long mac2hash(uint8_t *mac)
{
	unsigned long hash = 0;

	hash = mac[0];
	hash = (hash << 8) + mac[1];
	hash = (hash << 8) + mac[2];
	hash = (hash << 8) + mac[3];
	return hash;
}

struct ikbr_host* ikbr_host_get_by_mac(uint8_t *mac)
{
	struct ht_elem *hte = NULL;
	unsigned long hash = 0;
	struct ikbr_host *host = NULL;

	hash = mac2hash(mac);

	hte = hashtab_lookup(g_ikbr_host_htp, hash, mac);
	if (!hte)
		return NULL;

	host = container_of(hte, struct ikbr_host, ikh_hte);
	if (!kref_get_unless_zero(&host->ikh_kref))
		host = NULL;

	return host;
}

void ikbr_host_release_rcu(struct kref *kref)
{
	struct ikbr_host *host = container_of(kref, struct ikbr_host, ikh_kref);

	hashtab_lock_mod(g_ikbr_host_htp, host->ikh_hte.hte_hash);
	hashtab_del(&host->ikh_hte);
	hashtab_unlock_mod(g_ikbr_host_htp, host->ikh_hte.hte_hash);

	synchronize_rcu();
	kfree(host);
}

struct ikbr_host* __ikbr_host_alloc_init(uint8_t *mac, __be32 ip, gfp_t flags)
{
	struct ikbr_host *host = NULL;
	struct ht_elem *hte = NULL;
	unsigned long hash = 0;

	if (!mac)
		return NULL;

	hash = mac2hash(mac);

	hashtab_lock_mod(g_ikbr_host_htp, hash);
	/* Maybe added somewhere else, so find it first without kref_get */
	hte = hashtab_lookup(g_ikbr_host_htp, hash, mac);
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

	hashtab_add(g_ikbr_host_htp, hash, &host->ikh_hte);
end:
	hashtab_unlock_mod(g_ikbr_host_htp, hash);
	return host;
}

struct ikbr_host* ikbr_host_alloc_init(struct sk_buff *skb, gfp_t flags)
{
	struct iphdr *iph = ip_hdr(skb);
	struct ethhdr *ethh = eth_hdr(skb);

	return __ikbr_host_alloc_init(ethh->h_source, iph->saddr, flags);
}

static int __init ikbr_host_init(void)
{
	return 0;
}

static void __exit ikbr_host_exit(void)
{
}

module_init(ikbr_host_init);
module_exit(ikbr_host_exit);
