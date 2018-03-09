#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>
#include <linux/seq_file.h>

#include "url_secure.h"

static struct task_struct *g_us_protoc_handle_thread;

struct us_protoc_data {
	void *protoc_data;
	uint32_t data_len;
};

struct secure_url {
	char *content;
	uint32_t length;
};

struct secure_url_list {
	struct rcu_head rcu;
	uint32_t url_cnt;
	struct secure_url urls[0];
};

struct secure_url_list *g_secure_url_list;

/* 查询某个URL是否在白名单中 */
bool us_select_whitelist(const char *url)
{
	int i = 0;
	bool result = false;
	struct secure_url_list *urllist = rcu_dereference(g_secure_url_list);

	if (!urllist || !url)
		return false;

	rcu_read_lock();
	for (i = 0; i < urllist->url_cnt; i++) {
		struct secure_url *surl = &urllist->urls[i];
		if (!strncmp(surl->content, url, surl->length)) {
			result = true;
			break;
		}
	}
	rcu_read_unlock();

	return result;
}

/* 打印白名单所有条目 */
void us_whitelist_show(struct seq_file *s)
{
	int i = 0;
	struct secure_url_list *urllist = rcu_dereference(g_secure_url_list);

	if (!urllist)
		return;

	rcu_read_lock();
	for (i = 0; i < urllist->url_cnt; i++) {
		struct secure_url *surl = &urllist->urls[i];
		seq_printf(s, "%s\n", surl->content);
	}
	rcu_read_unlock();
}

/* 释放白名单 */
static void us_destroy_whitelist(struct secure_url_list *urllist)
{
	int i = 0;

	if (!urllist)
		return;

	for (i = 0; i < urllist->url_cnt; i++)
		kfree(urllist->urls[i].content);
	kfree(urllist);
}

/* 更新白名单 */
static void us_update_whitelist(struct secure_url_list *new_urllist)
{
	struct secure_url_list *old_urllist = rcu_dereference(g_secure_url_list);

	rcu_assign_pointer(g_secure_url_list, new_urllist);
	synchronize_rcu();
	us_destroy_whitelist(old_urllist);
}

/* 创建并填充白名单 */
static int us_create_whitelist(URLWhitelist *uw)
{
	struct secure_url_list *surllist;
	int i = 0;
	int err = -ENOMEM;

	surllist = kmalloc(sizeof(*surllist) + sizeof(struct secure_url) * uw->n_url, GFP_KERNEL);
	if (!surllist) {
		pr_info("Faile to allocate url whitelist\n");
		goto end1;
	}

	surllist->url_cnt = uw->n_url;

	for (i = 0; i < uw->n_url; i++) {
		struct secure_url *surl = &surllist->urls[i];
		int urllen = strnlen(uw->url[i], URL_MAX_LEN);

		surl->content = kzalloc(urllen + 1, GFP_KERNEL);
		if (!surl->content) {
			pr_info("Fail to allocate url\n");
			goto end2;
		}
		strncpy(surl->content, uw->url[i], urllen);
		surl->length = urllen;
	}
	us_update_whitelist(surllist);

	err = 0;
	return err;
end2:
	kfree(surllist);
end1:
	return err;
}

/* 白名单解析线程的执行函数 */
static int us_protoc_data_handler(void *uspd_data)
{
	URLWhitelist *uw;
	struct us_protoc_data *uspd = uspd_data;

	if (kthread_should_stop())
		goto end;

	uw = urlwhitelist__unpack(NULL, uspd->data_len, uspd->protoc_data);
	if (!uw) {
		pr_info("Fail to unpack url whitelist protoc data\n");
		return 0;
	}

	us_create_whitelist(uw);
	urlwhitelist__free_unpacked(uw, NULL);
end:
	kfree(uspd);
	g_us_protoc_handle_thread = NULL;
	return 0;
}

/* 创建内核线程，解析上层传来的URL白名单数据 */
void us_start_protoc_data_handle_thread(void *data, uint32_t data_len)
{
	struct us_protoc_data *uspd;

	if (g_us_protoc_handle_thread) {
		pr_info("There is a url secure protoc data handle thread running already\n");
		return;
	}

	uspd = kmalloc(sizeof(struct us_protoc_data), GFP_KERNEL);
	if (!uspd) {
		pr_info("Fail to allocate memory for us_protoc_data\n");
		return;
	}
	uspd->protoc_data = data;
	uspd->data_len = data_len;

	g_us_protoc_handle_thread = kthread_run(us_protoc_data_handler, uspd, "url_protoc_handler");
	if (IS_ERR(g_us_protoc_handle_thread)) {
		pr_info("Fail to start url proto data handle thread\n");
		g_us_protoc_handle_thread = NULL;
	}
}

void us_secure_init(void)
{
}

void us_secure_exit(void)
{
	kthread_stop(g_us_protoc_handle_thread);
}
