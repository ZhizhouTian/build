#include <linux/init.h>
#include <linux/module.h>
#include <net/net_namespace.h>
#include <linux/netdevice.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>

#include "gv.h"

MODULE_LICENSE("GPL");

/*
  SN: serial number
  MC: machine code
 */
static struct list_head g_match_algorithm_head;
spinlock_t match_algorithm_lock;
static atomic_t g_is_genuine;
static atomic_t g_is_validating;

struct match_algorithm {
	char ikversion[MAX_IKVERSION_LEN];
	struct match_operations *ops;
	struct list_head next;
};

void generate_machine_code(char mcbuf[MC_LEN])
{
	struct net_device *dev;
	int i = 0;
	unsigned char ms[6];

	memset(mcbuf, 0, MC_LEN);
	memset(ms, 0, sizeof(ms));

	for_each_netdev(&init_net, dev) {
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
	char mc[MC_LEN];
	bool ret = false;

	generate_machine_code(mc);
	spin_lock_bh(&match_algorithm_lock);
	list_for_each_entry(ma, &g_match_algorithm_head, next) {
		if (!strncmp(ikversion, ma->ikversion, sizeof(ma->ikversion)))
		{
			ret = ma->ops->match(mc, sn);
			break;
		}
	}
	spin_unlock_bh(&match_algorithm_lock);

	return ret;
}

#define GV_PROC_NAME "gv"
static int gv_proc_open (struct inode *inode, struct file *filp)
{
	return single_open(filp, NULL, NULL);
}

static int strim_len(const char __user * ubuff, unsigned int maxlen)
{
	int i;

	for (i = 0; i < maxlen; i++) {
		char c;
		if (get_user(c, &ubuff[i]))
			return -EFAULT;
		switch (c) {
		case '\"':
		case '\n':
		case '\r':
		case '\t':
		case ' ':
			goto done_str;
		default:
			break;
		}
	}
done_str:
	return i;
}

static int count_trails(const char __user * ubuff, unsigned int maxlen)
{
	int i;

	for (i = 0; i < maxlen; i++) {
		char c;
		if (get_user(c, &ubuff[i]))
			return -EFAULT;
		switch (c) {
		case '\"':
		case '\n':
		case '\r':
		case '\t':
		case ' ':
		case '=':
			break;
		default:
			goto done;
		}
	}
done:
	return i;
}

static ssize_t gv_proc_write(struct file *filp,
		const char __user *buf, size_t size, loff_t *offset)
{
	int start, len;
	int err = 0;
#define BUFFSIZE (MAX_IKVERSION_LEN + SN_LEN + 2)
	char ikver_sn[BUFFSIZE];
	char *ikver = NULL, *sn = NULL, *match, *pikver_sn = ikver_sn;

	atomic_inc(&g_is_validating);

	if (atomic_read(&g_is_genuine) == 1)
		goto end;

	memset(ikver_sn, 0, sizeof(ikver_sn));
	start = count_trails(buf, size);
	len = strim_len(&buf[start], sizeof(ikver_sn) - 1);
	if (copy_from_user(ikver_sn, &buf[start], len)) {
		err = -ENOMEM;
		goto err;
	}

	match = strsep(&pikver_sn, ",");
	if (!pikver_sn) {
		err = -EINVAL;
		goto err;
	}

	ikver = match;
	sn = pikver_sn;

	/* 正版检查 */
	if (genuine_verification(ikver, sn)) {
		cancel_punishment();
		atomic_set(&g_is_genuine, 1);
	}

end:
	atomic_dec(&g_is_validating);
	*offset += size;
	return size;
err:
	atomic_dec(&g_is_validating);
	*offset += size;
	return err;
}

enum gv_status {
	GV_NOT_GENUINE   = '0', /* 非正版 */
	GV_IS_GENUINE    = '1', /* 正版 */
	GV_IS_VALIDATING = '2', /* 正在验证中 */
};

static ssize_t gv_proc_read(struct file *file, char __user *buf,
		size_t size, loff_t *ppos)
{
	char message[2] = {0};
	int is_genuine;

	if (atomic_read(&g_is_validating)) {
		message[0] = GV_IS_VALIDATING;
		goto end;
	}

	is_genuine = atomic_read(&g_is_genuine);
	if (is_genuine == 0)
		message[0] = GV_NOT_GENUINE;
	else if (is_genuine == 1)
		message[0] = GV_IS_GENUINE;

end:
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

	if (!ikversion)
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
	proc_create_data(MC_PROC_NAME, 0400,
			NULL, &mc_proc_ops, NULL);

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
	remove_proc_entry(MC_PROC_NAME, NULL);
}

module_init(gv_init);
module_exit(gv_exit);
