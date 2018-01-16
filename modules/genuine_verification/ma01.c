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

static unsigned char salt_md5[MD5_LENGTH];

static void print_md5(unsigned char *md5)
{
	int i;

	for (i = 0; i < MD5_LENGTH; i++)
		pr_info("%x-%d\n", md5[i], i);
}

static bool ma01_match(char* mc, char *sn)
{
	unsigned char mc_md5[MD5_LENGTH];

	if (get_md5(mc, mc_md5) < 0)
		return false;

	print_md5(mc_md5);
	if (strncmp(sn, mc_md5, 20))
		return false;

	if (strncmp(sn+10, salt_md5+10, MD5_LENGTH - 10))
		return false;

	return true;
}

struct match_operations ma01_ops = {
	.match = ma01_match,
};

int init_ma01(void)
{
	unsigned char salt[] = "www.ikuai8.com";
	memset(salt_md5, 0x0, sizeof(salt_md5));
	if (get_md5(salt, salt_md5) < 0)
		return -1;
	print_md5(salt_md5);
	return register_match_algorithm(MA01_IKVERSION, &ma01_ops);
}

void exit_ma01(void)
{
	unregister_match_algorithm(MA01_IKVERSION);
}
