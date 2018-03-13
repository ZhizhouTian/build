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

#include "url_secure.h"
#include "ik_notify.h"

//TODO
//#define IK_URL_SECURE_DEBUG

struct us_check_entry {
	char *host;
	u32 host_len;
	struct ik_conn *ik_conn;
	unsigned long timeout;
	struct list_head next;
};

static LIST_HEAD(g_us_check_list);
static spinlock_t g_us_check_list_lock;

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

static inline void us_check_entry_free(struct us_check_entry *entry)
{
	struct ik_conn *ik_conn;

	if (!entry)
		return;

	ik_conn = entry->ik_conn;
	xchg(&ik_conn->us_entry, NULL);

	kfree(entry->host);
	kfree(entry);
}

static void us_check_entry_list_release_all(void)
{
	struct us_check_entry *entry, *tmp;

	spin_lock(&g_us_check_list_lock);
	list_for_each_entry_safe(entry, tmp, &g_us_check_list, next) {
		list_del(&entry->next);
		us_check_entry_free(entry);
	}
	spin_unlock(&g_us_check_list_lock);
}

void ik_conn_free_url_secure_entry(struct ik_conn *ik_conn)
{
	struct us_check_entry *entry;
	if (!ik_conn || !ik_conn->us_entry)
		return;

	spin_lock(&g_us_check_list_lock);

	pr_info("[URL SECURE]: ik_conn:%p freed\n", ik_conn);
	entry = ik_conn->us_entry;
	list_del(&entry->next);
	us_check_entry_free(entry);

	spin_unlock(&g_us_check_list_lock);
}

void ik_url_secure_check_timeout(void)
{
	struct us_check_entry *entry, *tmp;

	spin_lock(&g_us_check_list_lock);
	list_for_each_entry_safe(entry, tmp, &g_us_check_list, next) {
		if (time_after(jiffies, entry->timeout)) {
			list_del(&entry->next);
			us_check_entry_free(entry);
		}
	}
	spin_unlock(&g_us_check_list_lock);
}

void ik_url_secure_query_send(struct us_check_entry *entry)
{
	//TODO
	char msg[512];
	int len = sprintf(msg, "%s %p", entry->host, entry->ik_conn);

	ik_kernel_notify(IK_NOTIFY_MODULE_HTTP_APP, IK_HTTP_APP_URL_SECURE_QUERY, (u8*)&msg, len);
}

void ik_url_secure_result_recv(struct url_secure_result *us_result,
		void *data, unsigned int data_len)
{
	struct us_check_entry *entry, *tmp;
	struct ik_conn *ik_conn = NULL;

	if (!us_result)
		return;

	pr_info("[URL_SECURE]: %p result:%u\n", (void*)us_result->id, us_result->url_type);
	spin_lock(&g_us_check_list_lock);
	list_for_each_entry_safe(entry, tmp, &g_us_check_list, next) {
		if ((unsigned long)entry->ik_conn == us_result->id) {
			ik_conn = entry->ik_conn;
			list_del(&entry->next);
			us_check_entry_free(entry);
		}
	}
	spin_unlock(&g_us_check_list_lock);

	if (!ik_conn)
		return;

	switch(us_result->url_type) {
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

static void ik_url_secure_check_async(struct ik_conn *ik_conn,
		struct ik_skb *ik_skb, char *host, u32 host_len)
{
	struct us_check_entry *entry;

	entry = kmalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry) {
		pr_info("Allocate us_check_entry fail");
		return;
	}

	entry->ik_conn = ik_conn;
	entry->host = host;
	entry->host_len = host_len;
	entry->timeout = jiffies + HZ/10; //100ms
	mb();
	spin_lock(&g_us_check_list_lock);
	list_add_tail(&entry->next, &g_us_check_list);
	xchg(&ik_conn->us_entry, entry);
	spin_unlock(&g_us_check_list_lock);

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

int ik_url_secure_init(void)
{
	spin_lock_init(&g_us_check_list_lock);
	g_ik_func_switch.url_secure_enable = true;
	//url_secure_whitelist_init();
	return 0;
}

void ik_url_secure_exit(void)
{
	g_ik_func_switch.url_secure_enable = false;
	us_check_entry_list_release_all();
	//url_secure_whitelist_exit();
}
