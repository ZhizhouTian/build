#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>

#define SHA1_LENGTH     10

static int __init sha1_init(void)
{
	struct scatterlist sg;
	struct crypto_hash *tfm;
	struct hash_desc desc;
	unsigned char output[SHA1_LENGTH];
	unsigned char buf[10];
	int i;

	printk(KERN_INFO "sha1: %s\n", __FUNCTION__);

	memset(buf, 'A', 10);
	memset(output, 0x00, SHA1_LENGTH);

	tfm = crypto_alloc_hash("sha1", 0, CRYPTO_ALG_ASYNC);

	desc.tfm = tfm;
	desc.flags = 0;

	sg_init_one(&sg, buf, 10);
	crypto_hash_init(&desc);

	crypto_hash_update(&desc, &sg, 10);
	crypto_hash_final(&desc, output);

	for (i = 0; i < SHA1_LENGTH; i++) {
		printk(KERN_ERR "%d-%d\n", output[i], i);
	}

	crypto_free_hash(tfm);

	return 0;
}

static void __exit sha1_exit(void)
{
	printk(KERN_INFO "sha1: %s\n", __FUNCTION__);
}

module_init(sha1_init);
module_exit(sha1_exit);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Me");
