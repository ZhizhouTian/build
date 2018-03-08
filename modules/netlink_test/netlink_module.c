#include <linux/kernel.h>
#include <linux/module.h>
#include <net/net_namespace.h>
#include <linux/netlink.h>
#include <net/sock.h>

#define NETLINK_TEST 20

static struct sock *netlink_test_sk;

static void netlink_test_rcv(struct sk_buff *skb)
{
	struct sk_buff *skb_in, *skb_out;
	struct nlmsghdr *nlh;
	char message[32] = {0};
	int seq, pid;
	unsigned int datalength;

	skb_in = skb_get(skb);
	if (skb_in->len >= nlmsg_total_size(0)) {
		nlh = nlmsg_hdr(skb_in);

		pid = nlh->nlmsg_pid;
		seq = nlh->nlmsg_seq;
		printk("message received from process %d: %s\n", pid, (char*)NLMSG_DATA(nlh));

		sprintf(message, "hello, process %d", pid);
		datalength = strlen(message) + 1;

		skb_out = nlmsg_new(NLMSG_LENGTH(datalength), GFP_KERNEL);

		nlh = nlmsg_put(skb_out, pid, seq, 0, NLMSG_ALIGN(datalength), 0);

		memcpy(nlmsg_data(nlh), message, datalength);


		netlink_unicast(netlink_test_sk, skb_out, pid, MSG_DONTWAIT);
	}
	kfree_skb(skb_in);
}

int __init netlink_test_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.groups  = 0,
		.input  = netlink_test_rcv,
	};

	netlink_test_sk = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
	if (!netlink_test_sk)
		return -ENOMEM;

	return 0;
}

void __exit netlink_test_exit(void)
{
	netlink_kernel_release(netlink_test_sk);
}

module_init(netlink_test_init);
module_exit(netlink_test_exit);
MODULE_LICENSE("GPL");
