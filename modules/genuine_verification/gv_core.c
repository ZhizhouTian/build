#include <linux/init.h>
#include <linux/module.h>
#include <net/net_namespace.h>
#include <linux/netdevice.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>

#include "gv.h"

MODULE_LICENSE("GPL");

static struct list_head g_match_algorithm_head;
spinlock_t match_algorithm_lock;

#define MAX_IKVERSION_LEN 32
#define MAX_SN_LEN 20

static atomic_t g_is_genuine;
static atomic_t g_is_validating;

struct match_algorithm {
	char ikversion[MAX_IKVERSION_LEN];
	struct match_operations *ops;
	struct list_head next;
};

static uint64_t generate_machine_code(void)
{
	struct net_device *dev;
	int i = 0;
	union {
		uint64_t machine_code;
		char arr[6];
	} mc = {0};

	for_each_netdev(&init_net, dev) {
		for (i=0; i<6; i++) {
			mc.arr[i] += dev->dev_addr[i];
		}
	}

	return mc.machine_code;
}

struct timer_list g_punishment_timer;

static void punishment(unsigned long data)
{
	panic("盗版重启");
}

void start_punishment(void)
{
	pr_emerg("开启防盗版惩罚，五小时后重启.\n");
	init_timer(&g_punishment_timer);
	g_punishment_timer.expires = jiffies + HZ*60*60*5;
	g_punishment_timer.function = punishment;
	add_timer(&g_punishment_timer);
}

void cancel_punishment(void)
{
	pr_emerg("正版验证成功，停止盗版惩罚.\n");
	del_timer_sync(&g_punishment_timer);
}

static bool genuine_verification(char *ikversion, char *sn)
{
	struct match_algorithm *ma;
	uint64_t mc;
	bool ret = false;

	mc = generate_machine_code();
	spin_lock_bh(&match_algorithm_lock);
	list_for_each_entry(ma, &g_match_algorithm_head, next) {
		if (0 == strncmp(ikversion, ma->ikversion, sizeof(ma->ikversion)))
			ret = ma->ops->match(mc, sn);
	}
	spin_unlock_bh(&match_algorithm_lock);

	return ret;
}

#define GV_PROC_NAME "gv"
static int gv_proc_open (struct inode *inode, struct file *filp)
{
	return single_open(filp, NULL, NULL);
}

static ssize_t gv_proc_write(struct file *filp,
		const char __user *buf, size_t size, loff_t *offset)
{
	int ret = 0;
#define BUFFSIZE (MAX_IKVERSION_LEN + MAX_SN_LEN + 2)
	char ikver_sn[BUFFSIZE];
	char *ikver = NULL, *sn = NULL, *match, *pikver_sn = ikver_sn;

	atomic_inc(&g_is_validating);
	if (size >= BUFFSIZE)
		return -EINVAL;

	if ((ret = copy_from_user(ikver_sn, buf, size)) < 0) {
		atomic_dec(&g_is_validating);
		return -ENOMEM;
	}

	ikver_sn[size] = '\0';

	match = strsep(&pikver_sn, " ");
	if (!pikver_sn)
		return -EINVAL;

	ikver = match;
	sn = pikver_sn;

	/* 正版检查 */
	if (genuine_verification(ikver, sn)) {
		atomic_set(&g_is_genuine, 1);
		cancel_punishment();
	}

	atomic_dec(&g_is_validating);
	*offset += size;
	return size;
}

static ssize_t gv_proc_read(struct file *file, char __user *buf,
		size_t size, loff_t *ppos)
{
	char message[2] = {0};
	if (atomic_read(&g_is_validating))
		return 0;

	if (atomic_read(&g_is_genuine))
		message[0] = '1';
	else
		message[0] = '0';
	message[1] = '\0';

	return simple_read_from_buffer(buf, size, ppos, message,
			sizeof(message));
}

static const struct file_operations gv_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = gv_proc_open,
	.read    = gv_proc_read,
	.write   = gv_proc_write,
	.release = seq_release
};

int register_match_algorithm(const char *ikversion,
		struct match_operations *ops)
{
	struct match_algorithm *ma = NULL;

	if (!ma || !ikversion)
		return -EINVAL;

	ma = kzalloc(sizeof(struct match_algorithm), GFP_KERNEL);
	if (ma == NULL)
		return -ENOMEM;

	strlcpy(ma->ikversion, ikversion, sizeof(ma->ikversion));
	ma->ops = ops;

	spin_lock_bh(&match_algorithm_lock);
	list_add_tail(&ma->next, &g_match_algorithm_head);
	spin_unlock_bh(&match_algorithm_lock);

	return 0;
}

void unregister_match_algorithm(const char *ikversion)
{
	struct match_algorithm *ma, *tmp;

	spin_lock_bh(&match_algorithm_lock);
	list_for_each_entry_safe(ma, tmp, &g_match_algorithm_head, next) {
		if (!strncmp(ma->ikversion,ikversion, sizeof(ma->ikversion)))
			list_del(&ma->next);
	}
	spin_unlock_bh(&match_algorithm_lock);
}

static int __init gv_init(void)
{
	proc_create_data(GV_PROC_NAME, 0644,
			NULL, &gv_proc_ops, NULL);
	INIT_LIST_HEAD(&g_match_algorithm_head);
	spin_lock_init(&match_algorithm_lock);

	atomic_set(&g_is_genuine, 0);
	atomic_set(&g_is_validating, 0);

	init_ma01();

	start_punishment();
	return 0;
}

static void __exit gv_exit(void)
{
	remove_proc_entry(GV_PROC_NAME, NULL);
}

module_init(gv_init);
module_exit(gv_exit);
