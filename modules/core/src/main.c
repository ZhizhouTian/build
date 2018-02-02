#include <linux/module.h>
#include <linux/version.h>
#include <linux/percpu.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#include "period_task.h"
#include "debug.h"

MODULE_LICENSE("GPL");

static inline unsigned int prerouting_slow_path(struct sk_buff *skb,
				const struct net_device *in,
				int (*okfn)(struct sk_buff *))
{
	int ret = NF_ACCEPT;
	pr_info("[%s->%s] executed.\n", __FILE__, __func__);
	return ret;
}

static inline unsigned int prerouting_fast_path(struct sk_buff *skb,
				const struct net_device *in,
				int (*okfn)(struct sk_buff *))
{
	int ret = NF_ACCEPT;
	pr_info("[%s->%s] executed.\n", __FILE__, __func__);
	return ret;
}

static unsigned int prerouting_first_hook(const struct nf_hook_ops *ops,
				struct sk_buff *skb,
				const struct net_device *in,
				const struct net_device *out,
				int (*okfn)(struct sk_buff *))
{
	int ret = NF_ACCEPT;

	if (unlikely(skb->is_first)) {
		ret = prerouting_slow_path(skb, in, okfn);
	} else {
		ret = prerouting_fast_path(skb, in, okfn);
	}

	return ret;
}

static struct nf_hook_ops prerouting_first_ops __read_mostly = {
	.hook     = prerouting_first_hook,
	.pf       = NFPROTO_IPV4,
	.hooknum  = NF_INET_PRE_ROUTING,
	.priority = NF_IP_PRI_CONNTRACK + 1
};

int __init hook_init(void)
{
	int ret;

	ret = nf_register_hook(&prerouting_first_ops);
	if (ret) {
		pr_info("fail to register first prerouting hook.\n");
		goto err_prerouting_first_ops;
	}
	return 0;

err_prerouting_first_ops:
	return ret;
}

void __exit hook_exit(void)
{
	nf_unregister_hook(&prerouting_first_ops);
}

static __init int main_init(void)
{
	hook_init();
	percpu_daemon_init();
	return 0;
}

static __exit void main_exit(void)
{
	hook_exit();
	percpu_daemon_exit();
}

module_init(main_init);
module_exit(main_exit);
