#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

#define BYPASS_PROC_NAME "bypass"
static DEFINE_MUTEX(bypass_mutex);

#define	PORT_INDEX	0x2E
#define	PORT_DATA	0x2F

uint8_t	READ_ISA(uint8_t idx)
{
	uint8_t	dat;

	outb(idx, PORT_INDEX);
	dat = inb(PORT_DATA);

	return dat;
}

void WRITE_ISA(uint8_t idx, uint8_t dat)
{
	outb(idx, PORT_INDEX);
	outb(dat, PORT_DATA);
}

bool get_bypass(void)
{
	//Open Port
	outb(0x87, PORT_INDEX);
	outb(0x01, PORT_INDEX);
	outb(0x55, PORT_INDEX);
	outb(0x55, PORT_INDEX);

	WRITE_ISA(0x07, 0x07);
	return READ_ISA(0x25) == 0x1;
}

void set_bypass(int opt)
{
	//Open Port
	outb(0x87, PORT_INDEX);
	outb(0x01, PORT_INDEX);
	outb(0x55, PORT_INDEX);
	outb(0x55, PORT_INDEX);

	WRITE_ISA(0x07, 0x07);

	if (opt) //turn gpio10 to high
		WRITE_ISA(0x25, 0x01);
	else
		WRITE_ISA(0x25, 0x00);

	//Exit Port
	outb(0x02, PORT_INDEX);
	outb(0x02, PORT_DATA);
}

static int bypass_proc_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, NULL, NULL);
}

static ssize_t bypass_proc_read(struct file *filp, char __user *userbuf,
		size_t size, loff_t *offset)
{
	char message[2];

	mutex_lock(&bypass_mutex);

	if (get_bypass())
		message[0] = '1';
	else
		message[0] = '0';
	message[1] = '\0';

	mutex_unlock(&bypass_mutex);

	return simple_read_from_buffer(userbuf, size, offset,
		message, sizeof(message));
}

static ssize_t bypass_proc_write(struct file *filp, const char __user *userbuf,
		size_t size, loff_t *offset)
{
	char input[2] = {0};

	mutex_lock(&bypass_mutex);

	if (copy_from_user(input, userbuf, 1)) {
		mutex_unlock(&bypass_mutex);
		return -EFAULT;
	}


	if (input[0] == '1') {
		set_bypass(1);
	} else {
		set_bypass(0);
	}

	mutex_unlock(&bypass_mutex);
	return size;
}

static const struct file_operations bypass_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = bypass_proc_open,
	.read    = bypass_proc_read,
	.write   = bypass_proc_write,
	.release = seq_release
};

static __init int bypass_init(void)
{
	proc_create_data(BYPASS_PROC_NAME, 0644,
			NULL, &bypass_proc_ops, NULL);
	return 0;
}

static __exit void bypass_exit(void)
{
	remove_proc_entry(BYPASS_PROC_NAME, NULL);
}

module_init(bypass_init);
module_exit(bypass_exit);
