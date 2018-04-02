#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>
#include <linux/seq_file.h>
#include <linux/netfilter.h>
#include <linux/list.h>
#include <asm/cmpxchg.h>
#include <asm-generic/atomic-long.h>

#include "url_secure.h"
#include "ik_notify.h"

//TODO
//#define IK_URL_SECURE_DEBUG

/* url secure hash entry */
struct us_check_entry {
	unsigned long id;
	char *host;
	struct ik_conn *ik_conn;
	unsigned long timeout;
	atomic_t ref;
	struct list_head next;
};

/* url secure hash bucket */
struct us_bucket {
	struct list_head bucket_head;
	spinlock_t bucket_lock;
} __attribute__((aligned(64)));

/* url secure hash table */
struct us_hashtab {
	unsigned int nbucket;
	struct us_bucket us_bkt[0];
};

static struct us_hashtab *g_hashtab;
static atomic_long_t g_us_check_id = ATOMIC_LONG_INIT(0);

static char* us_dup_host_no_port(const char* host, u32 host_len, u32* ret_len)
{
	int i;
	char *host_no_port = NULL;

	if (!host || !ret_len || host_len == 0 || host_len > MAX_URL_LEN)
		return NULL;

	*ret_len = host_len;
	for (i = 0; i < host_len; i++) {
		if (host[i] == ':') {
			*ret_len = i;
			break;
		}
	}

	host_no_port = kmalloc(*ret_len + 1, GFP_KERNEL);
	if (!host_no_port) {
		pr_info("alloc host_no_port failed\n");
		return NULL;
	}

	strncpy(host_no_port, host, *ret_len); 
	host_no_port[*ret_len] = '\0';

	return host_no_port;
}

#define HASH2BKT(htp, h) (&(htp)->us_bkt[h % (htp)->nbucket])

static void hashtab_lock(struct us_hashtab *htp, unsigned long hash)
{
	spin_lock(&HASH2BKT(htp, hash)->bucket_lock);
}

static void hashtab_unlock(struct us_hashtab *htp, unsigned long hash)
{
	spin_unlock(&HASH2BKT(htp, hash)->bucket_lock);
}

void ik_url_secure_query_send(struct us_check_entry *entry)
{
#define BUFSIZE (MAX_URL_LEN + sizeof(unsigned long) + 1)
	char msg[BUFSIZE];
	int len = scnprintf(msg, BUFSIZE, "%s %lu", entry->host, entry->id);

	ik_kernel_notify(IK_NOTIFY_MODULE_HTTP_APP, IK_HTTP_APP_URL_SECURE_QUERY, (u8*)&msg, len);
}

static void ik_url_secure_check_async(struct ik_conn *ik_conn,
		struct ik_skb *ik_skb, char *host, u32 host_len)
{
	struct us_check_entry *entry;

	entry = kmalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry) {
		pr_info("Allocate us_check_entry fail");
		return;
	}

	entry->id = atomic_long_inc_return(&g_us_check_id);
	entry->ik_conn = ik_conn;
	entry->host = host;
	entry->timeout = jiffies + HZ/10; //100ms
	atomic_set(&entry->ref, 2);
	mb();

	hashtab_lock(g_hashtab, entry->id);
	list_add_tail(&entry->next, &HASH2BKT(g_hashtab, entry->id)->bucket_head);
	hashtab_unlock(g_hashtab, entry->id);
	ik_conn->us_entry = entry;

	ik_url_secure_query_send(entry);
}

void ik_url_secure_check(struct ik_conn* ik_conn, struct ik_skb *ik_skb)
{
	char *host, *host_no_port;
	u32 host_len, host_no_port_len;

	if (ik_conn->is_url_secure_checked)
		return;

	if (IK_REV_DIR == ik_skb->dir)
		return;

	if (skb_is_nonlinear(ik_skb->skb))
		return;

	if (!is_latest_http_parser_info(ik_skb))
		ik_parse_http_header(ik_skb, IK_PARSE_HTTP_REQ);

	host = ik_get_http_header(ik_skb, HTTP_Host, &host_len);
	if (!(host))
		return;

	if (xchg(&ik_conn->is_url_secure_checked, true))
		return;

#ifdef IK_URL_SECURE_DEBUG
	atomic_inc(&ik_conn->cnt);
	pr_info("[%s:%d] ik_conn->cnt:%d\n", __func__, __LINE__, atomic_read(&ik_conn->cnt));
#endif

	host_no_port = us_dup_host_no_port(host, host_len, &host_no_port_len);
	if (!host_no_port)
		return;
#if 0
	if (us_select_whitelist(host_no_port)) {
		kfree(host_no_port);
		return;
	}
#endif

	pr_info("[URL_SECURE]: %s %p\n", host_no_port, ik_conn);
	ik_url_secure_check_async(ik_conn, ik_skb, host_no_port, host_no_port_len);
}

void ik_url_secure_result_recv(struct url_secure_result *usr, void *data, u32 len)
{
	struct us_check_entry *entry, *tmp;
	struct ik_conn *ik_conn = NULL;

	pr_info("[URL_SECURE]: %lu result:%u\n", id, usr->url_type);

	hashtab_lock(g_hashtab, usr->id);
	list_for_each_entry_safe(entry, tmp, &HASH2BKT(g_hashtab, usr->id)->bucket_head, next) {
		ik_conn = entry->ik_conn;
		list_del(&entry->next);
		us_check_entry_free(entry);
	}
	hashtab_unlock(g_hashtab, usr->id);

	if (!ik_conn)
		return;

	switch(usr->url_type) {
	case US_RESULT_UNKNOW0:
	case US_RESULT_UNKNOW1:
		pr_info("[URL SECURE UNKNOW]:\n");
		break;
	case US_RESULT_ILLEGAL:
		pr_info("[URL SECURE ILLEGAL]:\n");
		ik_conn->need_reset = true;
		break;
	case US_RESULT_OK:
		pr_info("[URL SECURE OK]:\n");
		break;
	default:
		break;
	}
}

static inline void us_check_entry_free(struct us_check_entry *entry)
{
	if (!entry)
		return;

	if (atomic_dec_and_test(&entry->ref)) {
		kfree(entry->host);
		kfree(entry);
	}
}

static void us_check_entry_list_release_all(void)
{
	int hash;
	struct us_check_entry *entry, *tmp;

	for (hash = 0; hash < g_hashtab->nbucket; hash++) {
		hashtab_lock(g_hashtab, hash);
		list_for_each_entry_safe(entry, tmp, &HASH2BKT(g_hashtab, hash)->bucket_head, next) {
			list_del(&entry->next);
			us_check_entry_free(entry);
		}
		hashtab_unlock(g_hashtab, hash);
	}
}

void ik_conn_free_url_secure_entry(struct ik_conn *ik_conn)
{
	struct us_check_entry *entry;
	unsigned long hash;

	if (!ik_conn)
		return;

	if (!ik_conn->us_entry)
		return;

	hash = entry->id;
	hashtab_lock(g_hashtab, hash);
	if (!ik_conn->us_entry) {
		hashtab_unlock(g_hashtab, hash);
		return;
	}

	pr_info("[URL SECURE]: ik_conn:%p freed\n", ik_conn);
	list_del(&ik_conn->entry->next);
	us_check_entry_free(ik_conn->entry);

	hashtab_unlock(g_hashtab, hash);
}

void ik_url_secure_check_timeout(void)
{
	int hash;
	struct us_check_entry *entry, *tmp;

	for (hash = 0; hash < g_hashtab->nbucket; hash++) {
		hashtab_lock(g_hashtab, hash);
		list_for_each_entry_safe(entry, tmp, &HASH2BKT(g_hashtab, id)->bucket_head, next) {
			if (time_after(jiffies, entry->timeout)) {
				list_del(&entry->next);
				us_check_entry_free(entry);
			}
		}
		hashtab_unlock(g_hashtab, hash);
	}
}

static int us_hashtab_alloc(void)
{
	unsigned int i;
	unsigned int nbucket;
	u32 total_mem;
	u32 size;

	total_mem = totalram_pages / 1024 * 4;

	if (total_mem >= 8*1024) 	// above 8G
		nbucket = 1024;
	if (total_mem >= 4*1024) 	// above 4G
		nbucket = 512;
	else if (total_mem >= 2*1024)	// above 2G
		nbucket = 256;
	else if (total_mem >= 1024) 	// above 1G
		nbucket = 128;
	else if (total_mem >= 512) 	// above 512 M
		nbucket = 64;
	else 				// It shouldn't be x86
		nbucket = 32;

	size = sizeof(struct us_hashtab) + nbucket * sizeof(struct us_bucket);
	g_hashtab = kmalloc(size, GFP_KERNEL);
	if (!g_hashtab) {
		pr_info("[URL SECURE] alloc memory for bucket error\n");
		return -ENOMEM;
	}

	pr_info("[URL SECURE] memtotal(%uMb) nbucket(%u) spend(%uKb)\n", total_mem, nbucket, size/1024);

	for (i = 0; i < nbucket; i++) {
		struct us_bucket *bucket = &g_hashtab->us_bkt[i];
		INIT_LIST_HEAD(&bucket->bucket_head);
		spin_lock_init(&bucket->bucket_lock);
	}
	g_hashtab->nbucket = nbucket;

	return 0;
}

int ik_url_secure_init(void)
{
	int err;

	if ((err = us_hashtab_alloc()) != 0)
		goto err;

	//url_secure_whitelist_init();
	g_ik_func_switch.url_secure_enable = true;

	return 0;
err:
	return err;
}

void ik_url_secure_exit(void)
{
	g_ik_func_switch.url_secure_enable = false;
	us_check_entry_list_release_all();
	kfree(g_hashtab);
	//url_secure_whitelist_exit();
}
