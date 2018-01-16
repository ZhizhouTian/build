#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>

#define MD5_LENGTH     16

static int get_md5(char *input, unsigned char *output)
{
	struct scatterlist sg;
	struct crypto_hash *tfm;
	struct hash_desc desc;

	memset(output, 0x0, MD5_LENGTH);
	tfm = crypto_alloc_hash("md5", 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(tfm))
		return -1;

	desc.tfm = tfm;
	desc.flags = 0;

	sg_init_one(&sg, input, strlen(input));
	crypto_hash_init(&desc);

	crypto_hash_update(&desc, &sg, strlen(input));
	crypto_hash_final(&desc, output);

	crypto_free_hash(tfm);

	return 0;
}

static int __init md5_init(void)
{
	int i;
	char salt[] = "www.ikuai8.com";
	unsigned char output[MD5_LENGTH];

	memset(output, 0x0, sizeof(output));
	get_md5(salt, output);

	for (i = 0; i < MD5_LENGTH; i++)
		printk(KERN_ERR "%x-%d\n", output[i], i);

	return 0;
}

static void __exit md5_exit(void)
{
	printk(KERN_INFO "md5: %s\n", __FUNCTION__);
}

module_init(md5_init);
module_exit(md5_exit);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Me");
