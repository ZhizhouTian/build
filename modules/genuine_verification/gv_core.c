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
char g_ikversion[MAX_IKVERSION_LEN];

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

static bool genuine_verification(char *ikversion, uint64_t serial_number)
{
	struct match_algorithm *ma;
	uint64_t mc;
	bool ret = false;

	mc = generate_machine_code();
	spin_lock_bh(&match_algorithm_lock);
	list_for_each_entry(ma, &g_match_algorithm_head, next) {
		if (0 == strncmp(ikversion, ma->ikversion, sizeof(ma->ikversion)))
			ret = ma->ops->match(mc, serial_number);
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
	size_t copy_len = size;

	atomic_inc(&g_is_validating);
	if (size >= MAX_IKVERSION_LEN)
		copy_len = MAX_IKVERSION_LEN - 1;

	if ((ret = copy_from_user(g_ikversion, buf, copy_len)) < 0) {
		atomic_dec(&g_is_validating);
		return -ENOMEM;
	}

	g_ikversion[copy_len] = '\0';

	/* 正版检查 */
	if (genuine_verification(g_ikversion, serial_number))
		atomic_set(&g_is_genuine, 1);

	atomic_dec(&g_is_validating);
	*offset += size;
	return size;
}

static ssize_t gv_proc_read(struct file *filp, char __user *buff,
		size_t size, loff_t *offset)
{
	if (atomic_read(&g_is_validating) == 1)
		return 0;

	if (size <= 2)
		return 0;

	if (atomic_read(&g_is_genuine) == 0)
		copy_to_user(buf, "0", 2);
	else
		copy_to_user(buf, "1", 2);

	*offset += 2;
	return 2;
}

static const struct file_operations gv_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = gv_proc_open,
	.read    = gv_proc_read,
	.write   = gv_proc_write,
	.release = seq_release
};

int register_match_algorithm(const char *ikversion, struct match_operations *ops)
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

	return 0;
}

static void __exit gv_exit(void)
{
	remove_proc_entry(SET_IKVERSION_PROC_NAME, NULL);
}

module_init(gv_init);
module_exit(gv_exit);
