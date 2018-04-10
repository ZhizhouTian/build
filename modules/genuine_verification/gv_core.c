#include <linux/init.h>
#include <linux/module.h>
#include <net/net_namespace.h>
#include <linux/netdevice.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>

#include "gv.h"

#define MAX_SN_LEN 65
#define GV_PROC_NAME "genuine_verification"

static atomic_t g_is_genuine;

void generate_machine_code(char *mcbuf)
{
	struct net_device *dev;
	int i = 0;
	unsigned char ms[6];

	memset(mcbuf, 0, MC_LEN);
	memset(ms, 0, sizeof(ms));

	for_each_netdev(&init_net, dev) {
		if (!strncmp(dev->name, "eth0", 4)) {
			for (i=0; i<6; i++) {
				ms[i] += dev->dev_addr[i];
			}
			break;
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
	g_punishment_timer.expires = jiffies + HZ*60*60*24*7;
	g_punishment_timer.function = punishment;
	add_timer(&g_punishment_timer);
}

void cancel_punishment(void)
{
	del_timer_sync(&g_punishment_timer);
}

static int genuine_verification(char *input)
{
	char mc[MC_LEN];

	generate_machine_code(mc);
	if (ma01_match(mc, input)) {
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
	char input[MAX_SN_LEN];
	int i, retval = 0;

	if (atomic_read(&g_is_genuine) == 1) {
		goto end;
	}

	size = (size > MAX_SN_LEN-1)?(MAX_SN_LEN-1):(size);

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

	/* echo sn > /proc/genuine_verification */
	retval = genuine_verification(input);

end:
	*offset += size;
	if (retval == 0)
		return size;
	return retval;
}

static const struct file_operations ik_gv_proc_ops = {
	.owner	 = THIS_MODULE,
	.open	 = ik_gv_proc_open,
	.read	 = seq_read,
	.write	 = ik_gv_proc_write,
	.release = seq_release,
};

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
