#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <net/net_namespace.h>
#include <linux/netdevice.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>

#include <asm/uaccess.h>

#include "gv.h"

MODULE_LICENSE("GPL");

static LIST_HEAD(g_match_algorithm_head);
static spinlock_t match_algorithm_lock;
static atomic_t g_is_genuine;
static atomic_t g_is_validating;

struct match_algorithm {
	char ikversion[MAX_IKVERSION_LEN];
	struct match_operations *ops;
	struct list_head next;
};

void generate_machine_code(char *mcbuf)
{
	struct net_device *dev;
	int i = 0;
	unsigned char ms[6];

	memset(mcbuf, 0, MC_LEN);
	memset(ms, 0, sizeof(ms));

	for_each_netdev(&init_net, dev) {
		if (strncmp(dev->name, "eth", 3))
			continue;

		for (i=0; i<6; i++) {
			ms[i] += dev->dev_addr[i];
		}
	}
	sprintf(mcbuf, "%02x%02x%02x%02x%02x%02x",
			ms[0],ms[1],ms[2],ms[3],ms[4],ms[5]);
}

struct timer_list g_punishment_timer;

static void punishment(unsigned long data)
{
	// do nothing
	//panic("盗版重启");
}

void start_punishment(void)
{
	pr_emerg("iKuai正版验证模块启动.\n");
	init_timer(&g_punishment_timer);
	g_punishment_timer.expires = jiffies + HZ*60*60*5;
	g_punishment_timer.function = punishment;
	add_timer(&g_punishment_timer);
}

void cancel_punishment(void)
{
	pr_emerg("iKuai正版验证成功.\n");
	del_timer_sync(&g_punishment_timer);
}

static bool genuine_verification(char *ikversion, char *sn)
{
	struct match_algorithm *ma;
	char mc[MC_LEN];
	bool ret = false;

	generate_machine_code(mc);
	spin_lock_bh(&match_algorithm_lock);
	list_for_each_entry(ma, &g_match_algorithm_head, next) {
		if (!strncmp(ikversion, ma->ikversion, sizeof(ma->ikversion))) {
			ret = ma->ops->match(mc, sn);
			break;
		}
	}
	spin_unlock_bh(&match_algorithm_lock);

	return ret;
}

#define GV_PROC_NAME "genuine_verification"
static int gv_proc_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, NULL, NULL);
}

static ssize_t gv_proc_write(struct file *filp, const char __user *userbuf,
		size_t size, loff_t *offset)
{
#define INPUT_SIZE (MAX_IKVERSION_LEN + MAX_SERIAL_NUMBER_LEN + 2)
	char input[INPUT_SIZE];
	char *ikver = NULL, *sn = NULL, *match = NULL, *pinput = input;
	int i, err;

	if (atomic_read(&g_is_validating) == 1)
		goto end;
	atomic_set(&g_is_validating, 1);

	if (atomic_read(&g_is_genuine) == 1)
		goto end;

	size = (size > INPUT_SIZE-1)?(INPUT_SIZE-1):(size);

	if (copy_from_user(input, userbuf, size)) {
		err = -EFAULT;
		goto err;
	}

	input[size] = '\0';

	i = size - 1;
	while (i >= 0 && (input[i] == '\n' || input[i] == ' ' ||
			  input[i] == '\t' || input[i] == '\r')) {
		input[i] = '\0';
		i--;
	}

	/* echo ikversion,sn > /proc/genuine_verification */
	match = strsep(&pinput, ",");
	if (!pinput) {
		err = -EINVAL;
		goto err;
	}
	ikver = match;
	sn = pinput;

	if (genuine_verification(ikver, sn)) {
		atomic_set(&g_is_genuine, 1);
	}

end:
	atomic_set(&g_is_validating, 0);
	*offset += size;
	return size;
err:
	atomic_set(&g_is_validating, 0);
	return err;
}

static ssize_t gv_proc_read(struct file *filp, char __user *userbuf,
		size_t size, loff_t *offset)
{
	char message[2];

	if (atomic_read(&g_is_validating) == 1) {
		message[0] = '2';
		goto end;
	}

	if (atomic_read(&g_is_genuine) == 0)
		message[0] = '0';
	else
		message[0] = '1';

end:
	message[1] = '\0';
	return simple_read_from_buffer(userbuf, size, offset,
		message, sizeof(message));
}

static struct file_operations gv_proc_ops = {
	.owner = THIS_MODULE,
	.open = gv_proc_open,
	.read = gv_proc_read,
	.write = gv_proc_write,
};

#define MC_PROC_NAME "mc"
static int mc_proc_open (struct inode *inode, struct file *filp)
{
	return single_open(filp, NULL, NULL);
}

static ssize_t mc_proc_read(struct file *file, char __user *buf,
		size_t size, loff_t *ppos)
{
	char message[MC_LEN] = {0};
	generate_machine_code(message);

	return simple_read_from_buffer(buf, size, ppos, message,
			sizeof(message));
}

static const struct file_operations mc_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = mc_proc_open,
	.read    = mc_proc_read,
	.release = seq_release
};

int register_match_algorithm(const char *ikversion,
		struct match_operations *ops)
{
	struct match_algorithm *ma = NULL;
	int err = -ENOMEM;

	if (!ikversion)
		return -EINVAL;

	spin_lock_bh(&match_algorithm_lock);
	list_for_each_entry(ma, &g_match_algorithm_head, next) {
		if (!strncmp(ma->ikversion, ikversion, sizeof(ma->ikversion))) {
			err = -EINVAL;
			goto end;
		}
	}

	ma = kmalloc(sizeof(struct match_algorithm), GFP_KERNEL);
	if (ma == NULL)
		goto end;
	ma->ops = ops;
	strlcpy(ma->ikversion, ikversion, sizeof(ma->ikversion));
	list_add_tail(&ma->next, &g_match_algorithm_head);
	err = 0;
end:
	spin_unlock_bh(&match_algorithm_lock);
	return err;
}

void unregister_match_algorithm(const char *ikversion)
{
	struct match_algorithm *ma, *tmp;

	spin_lock_bh(&match_algorithm_lock);
	list_for_each_entry_safe(ma, tmp, &g_match_algorithm_head, next) {
		if (!strncmp(ma->ikversion, ikversion, sizeof(ma->ikversion))) {
			list_del(&ma->next);
			kfree(ma);
		}
	}
	spin_unlock_bh(&match_algorithm_lock);
}

static __init int gvcore_init(void)
{
	proc_create_data(GV_PROC_NAME, 0644, NULL, &gv_proc_ops, NULL);
	proc_create_data(MC_PROC_NAME, 0400, NULL, &mc_proc_ops, NULL);
	spin_lock_init(&match_algorithm_lock);
	atomic_set(&g_is_genuine, 0);
	atomic_set(&g_is_validating, 0);
	init_ma01();
	start_punishment();

	return 0;
}

static __exit void gvcore_exit(void)
{
	remove_proc_entry(GV_PROC_NAME, NULL);
	remove_proc_entry(MC_PROC_NAME, NULL);
	exit_ma01();
	cancel_punishment();
}

module_init(gvcore_init);
module_exit(gvcore_exit);
