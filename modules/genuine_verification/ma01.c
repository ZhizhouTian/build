#include <linux/time.h>
#include <linux/netdevice.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>

#include "gv.h"

#define MA01_IKVERSION "3.0.3"

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

static void ma01_generate_serial_number(char *mc, char *sn)
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

static bool ma01_match(char* mc, char *sn)
{
	char snbuf[SN_LEN];

	ma01_generate_serial_number(mc, snbuf);

	if (!strncmp(snbuf, sn, MC_LEN))
		return true;

	return false;
}

struct match_operations ma01_ops = {
	.match = ma01_match,
};

int init_ma01(void)
{
	return register_match_algorithm(MA01_IKVERSION, &ma01_ops);
}

void exit_ma01(void)
{
	unregister_match_algorithm(MA01_IKVERSION);
}
