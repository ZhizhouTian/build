#include <linux/init.h>
#include <linux/module.h>
#include <net/net_namespace.h>
#include <linux/netdevice.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>

#include "gv.h"

#define GV_PROC_NAME "genuine_verification"

static LIST_HEAD(g_match_algorithm_head);
static DEFINE_MUTEX(g_match_algorithm_mutex);
static atomic_t g_is_genuine;

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
	panic("盗版重启");
}

void start_punishment(void)
{
	init_timer(&g_punishment_timer);
	g_punishment_timer.expires = jiffies + HZ*60*60*24*3;
	g_punishment_timer.function = punishment;
	add_timer(&g_punishment_timer);
}

void cancel_punishment(void)
{
	del_timer_sync(&g_punishment_timer);
}

static int reset_punishment(char *input)
{
	unsigned long type = 0;

	/* echo punishment:x > /proc/genuine_verification */
	strsep(&input, ":");
	if (!input)
		return -EINVAL;

	if (kstrtol(input, 10, &type) < 0)
		return -EINVAL;
	/* TODO: reset punishment */
	return 0;
}

static bool _genuine_verification(char *ikversion, char *sn)
{
	struct match_algorithm *ma;
	char mc[MC_LEN];
	bool ret = false;

	generate_machine_code(mc);

	if (mutex_lock_interruptible(&g_match_algorithm_mutex))
		return -ERESTARTSYS;

	list_for_each_entry(ma, &g_match_algorithm_head, next) {
		if (!strncmp(ikversion, ma->ikversion, sizeof(ma->ikversion))) {
			ret = ma->ops->match(mc, sn);
			break;
		}
	}
	mutex_unlock(&g_match_algorithm_mutex);

	return ret;
}

static int genuine_verification(char *input)
{
	char *ikver = NULL, *sn = NULL, *match = NULL, *pinput = input;

	/* echo ikversion,sn > /proc/genuine_verification */
	match = strsep(&pinput, ",");
	if (!pinput)
		return -EINVAL;

	ikver = match;
	sn = pinput;

	if (_genuine_verification(ikver, sn)) {
		atomic_set(&g_is_genuine, 1);
		cancel_punishment();
	}

	return 0;
}

static int ik_gv_seq_show(struct seq_file *s, void *v)
{
	if (atomic_read(&g_is_genuine) == 0) {
		char mc[MC_LEN];
		generate_machine_code(mc);

		seq_printf(s, "machine_code=%s\n", mc);
		seq_printf(s, "genuine=0\n");
		seq_printf(s, "reboot_time=%lu\n",
			(g_punishment_timer.expires-jiffies) / HZ);
		return 0;
	}

	seq_printf(s, "machine_code=0\n");
	seq_printf(s, "genuine=1\n");
	seq_printf(s, "reboot_time=0\n");

	return 0;
}

static int ik_gv_proc_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, ik_gv_seq_show, NULL);
}

static ssize_t ik_gv_proc_write(struct file *filp, const char __user *userbuf,
		size_t size, loff_t *offset)
{
#define INPUT_SIZE (MAX_IKVERSION_LEN + SN_LEN + 2)
	char input[INPUT_SIZE];
	int i, retval = 0;

	if (atomic_read(&g_is_genuine) == 1)
		goto end;

	size = (size > INPUT_SIZE-1)?(INPUT_SIZE-1):(size);

	if (copy_from_user(input, userbuf, size)) {
		retval = -EFAULT;
		goto end;
	}

	input[size] = '\0';

	i = size - 1;
	while (i >= 0 && (input[i] == '\n' || input[i] == ' ' ||
			  input[i] == '\t' || input[i] == '\r')) {
		input[i] = '\0';
		i--;
	}

	if (!strncmp(input, "punishment", sizeof("punishment") - 1))
		retval = reset_punishment(input);
	else
		retval = genuine_verification(input);

	if (retval != 0)
		return retval;
end:
	*offset += size;
	retval = size;
	return retval;
}

static const struct file_operations ik_gv_proc_ops = {
	.owner	 = THIS_MODULE,
	.open	 = ik_gv_proc_open,
	.read	 = seq_read,
	.write	 = ik_gv_proc_write,
	.release = seq_release,
};

int register_match_algorithm(const char *ikversion,
		struct match_operations *ops)
{
	struct match_algorithm *ma = NULL;
	int err = -ENOMEM;

	if (!ikversion)
		return -EINVAL;

	mutex_lock(&g_match_algorithm_mutex);
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
	mutex_unlock(&g_match_algorithm_mutex);
	return err;
}

void unregister_match_algorithm(const char *ikversion)
{
	struct match_algorithm *ma, *tmp;

	mutex_lock(&g_match_algorithm_mutex);
	list_for_each_entry_safe(ma, tmp, &g_match_algorithm_head, next) {
		if (!strncmp(ma->ikversion, ikversion, sizeof(ma->ikversion))) {
			list_del(&ma->next);
			kfree(ma);
		}
	}
	mutex_unlock(&g_match_algorithm_mutex);
}

static int __init ik_gv_init(void)
{
	proc_create_data(GV_PROC_NAME, 0644, NULL, &ik_gv_proc_ops, NULL);

	atomic_set(&g_is_genuine, 0);
	init_ma01();
	start_punishment();

	return 0;
}

static void __exit ik_gv_exit(void)
{
	remove_proc_entry(GV_PROC_NAME, NULL);

	exit_ma01();
	cancel_punishment();
}

module_init(ik_gv_init);
module_exit(ik_gv_exit);
MODULE_LICENSE("GPL");
