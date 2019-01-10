#include <linux/version.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/if_arp.h>
#include <linux/skbuff.h>
#include <linux/ratelimit.h>
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <net/arp.h>
#include <linux/inetdevice.h>
#include "ik_br_private.h"

static inline struct net_device * bridge_parent(const struct net_device *dev)
{
	struct net_bridge_port *port;

	port = br_port_get_rcu(dev);
	return port ? port->br->dev : NULL;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,14)
static unsigned int ikbr_auth_hook(unsigned int hook, struct sk_buff *skb,
		const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff*))
#else
static unsigned int ikbr_auth_hook(const struct nf_hook_ops *ops, struct sk_buff *skb,
	const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff*))
#endif
{
	struct iphdr _iph, *iph;
	struct net_device *dev;
	struct in_device *in_dev;

	if (!(dev = bridge_parent(in)))
		goto accept;

	if (unlikely(skb->protocol != cpu_to_be16(ETH_P_IP)))
		goto accept;

	if (unlikely(!(iph = skb_header_pointer(skb, 0, sizeof(_iph), &_iph))))
		goto accept;

	pr_info("dev name:%s\n", dev->name);
	if (likely(in_dev = in_dev_get(dev))) {
		for_ifa(in_dev) {
			pr_info("ip: %pI4\n", &ifa->ifa_address);
		} endfor_ifa(in_dev);
		in_dev_put(in_dev);
	}

accept:
	return NF_ACCEPT;
}

static struct nf_hook_ops ikbr_auth_ops __read_mostly = {
	.hook           = ikbr_auth_hook,
	.pf             = NFPROTO_BRIDGE,
	.hooknum        = NF_BR_PRE_ROUTING,
};

int __init ikbr_auth_init(void)
{
	return nf_register_hook(&ikbr_auth_ops);
}

void __exit ikbr_auth_exit(void)
{
	nf_unregister_hook(&ikbr_auth_ops);
}

module_init(ikbr_auth_init);
module_exit(ikbr_auth_exit);
