#include <linux/kernel.h>
#include <linux/module.h>
#include <net/net_namespace.h>
#include <linux/netlink.h>
#include <net/sock.h>

#define netlink 20

#include "main.h"
#include "ik_high_prio_url.h"
static struct sock *netlink_sk;

static void netlink_rcv(struct sk_buff *skb)
{
	struct sk_buff *skb_in;
	struct nlmsghdr *nlh;
	struct cntl_msg *cm;

	skb_in = skb_get(skb);
	if (skb_in->len >= nlmsg_total_size(0)) {
		nlh = nlmsg_hdr(skb_in);

		cm = (struct cntl_msg*) NLMSG_DATA(nlh);
		ik_high_prio_url_recv(cm->data, cm->len);
#if 0
		sprintf(message, "hello, process %d", pid);
		datalength = strlen(message) + 1;

		skb_out = nlmsg_new(NLMSG_LENGTH(datalength), GFP_KERNEL);

		nlh = nlmsg_put(skb_out, pid, seq, 0, NLMSG_ALIGN(datalength), 0);

		memcpy(nlmsg_data(nlh), message, datalength);

		netlink_unicast(netlink_sk, skb_out, pid, MSG_DONTWAIT);
#endif
	}
	kfree_skb(skb_in);
}

int __init netlink_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.groups  = 0,
		.input  = netlink_rcv,
	};

	netlink_sk = netlink_kernel_create(&init_net, netlink, &cfg);
	if (!netlink_sk)
		return -ENOMEM;

	return 0;
}

void __exit netlink_exit(void)
{
	netlink_kernel_release(netlink_sk);
	high_prio_url_exit();
}

module_init(netlink_init);
module_exit(netlink_exit);
MODULE_LICENSE("GPL");
