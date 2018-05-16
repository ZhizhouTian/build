#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>
#include <linux/seq_file.h>

#include "ik_utils.h"
#include "ik_high_prio_url.h"
#include "urllist.pb-c.h"

static AC_TRIE_t *g_trie;

/* 接收protobuf数据并进行处理 */
void ik_high_prio_url_recv(void *data, unsigned int data_len)
{
	URLList *ul;

	ul = urllist__unpack(NULL, data_len, data);
	if (!ul) {
		pr_info("fail to unpack urllist protoc data\n");
		return;
	}

	{
		int i = 0;
		for (i=0; i<ul->n_url; i++) {
			pr_info("%s\n", ul->url[i]);
		}
	}
	ik_url_trie_update(&g_trie, ul->url, ul->n_url);
	urllist__free_unpacked(ul, NULL);
}

int __init high_prio_url_init(void)
{
	return 0;
}

void __exit high_prio_url_exit(void)
{
	ik_url_trie_release(g_trie);
}
