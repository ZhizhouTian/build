#include <linux/kernel.h>
#include <linux/module.h>
#include <net/net_namespace.h>
#include <linux/netlink.h>
#include <net/sock.h>

#include "url_secure.h"
#include "netlink_cntl.h"

#define NETLINK_CTL 30

static struct sock *netlink_sk;

#if 0
static void netlink_response(struct sk_buff *skb_in, void *data, uint32_t len)
{
	struct sk_buff *skb_out;
	int seq, pid;
	struct nlmsghdr *nlh;
	nlh = nlmsg_hdr(skb_in);
	pid = nlh->nlmsg_pid;
	seq = nlh->nlmsg_seq;

	skb_out = nlmsg_new(NLMSG_LENGTH(len), GFP_KERNEL);
	nlh = nlmsg_put(skb_out, pid, seq, 0, NLMSG_ALIGN(len), 0);
	memcpy(nlmsg_data(nlh), data, len);
	netlink_unicast(netlink_sk, skb_out, pid, MSG_DONTWAIT);
}

static void show_data(void *data, uint32_t len)
{
	int i = 0;
	char *pdata = data;

	pr_info("data length:%d\n", len);
	for (i = 0; i < len; i++) {
		pr_info("%x ", pdata[i]);
		if (i % 10 == 0)
			pr_info("\n");
	}
}
#endif

static void netlink_rcv(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	struct netlink_cntl *cntl_msg;

	nlh = nlmsg_hdr(skb);
	if (skb->len < NLMSG_SPACE(0) || skb->len < nlh->nlmsg_len)
		goto end;

	skb = skb_clone(skb, GFP_KERNEL);
	if (skb == NULL)
		goto end;

	nlh = nlmsg_hdr(skb);

	cntl_msg = (struct netlink_cntl*) NLMSG_DATA(nlh);
	us_start_protoc_data_handle_thread(cntl_msg->data, cntl_msg->data_len);

end:
	return;
}

#define US_WHITELIST_PROC_NAME "url_secure_whitelist"

static int us_seq_show(struct seq_file *s, void *v)
{
	us_whitelist_show(s);
	return 0;
}

static int us_proc_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, us_seq_show, NULL);
}

static const struct file_operations us_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = us_proc_open,
	.read    = seq_read,
	.release = seq_release,
};

int __init main_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.groups  = 0,
		.input  = netlink_rcv,
	};

	proc_create_data(US_WHITELIST_PROC_NAME, 0400, NULL, &us_proc_ops, NULL);
	netlink_sk = netlink_kernel_create(&init_net, NETLINK_CTL, &cfg);
	if (!netlink_sk)
		return -ENOMEM;

	return 0;
}

void __exit main_exit(void)
{
	netlink_kernel_release(netlink_sk);
}

module_init(main_init);
module_exit(main_exit);
MODULE_LICENSE("GPL");
