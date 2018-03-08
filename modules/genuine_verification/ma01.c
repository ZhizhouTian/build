#include <linux/time.h>
#include <linux/netdevice.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>

#include "gv.h"

#define MA01_IKVERSION "3.1.0"

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

static int str2md5hex(char *sn, unsigned char *sn_md5)
{
	int i;

	for (i = 0; i < MD5_LENGTH; i++) {
		char s[3];
		s[0] = sn[2 * i];
		s[1] = sn[2 * i + 1];
		s[2] = '\0';
		if (kstrtou8(s, 16, &sn_md5[i]) < 0)
			return -1;
	}

	return 0;
}

static bool ma01_match(char* mc, char *sn)
{
	unsigned char mc_md5[MD5_LENGTH];
	unsigned int i;
	unsigned char sn_md5[MD5_LENGTH];

	if (get_md5(mc, mc_md5) < 0)
		return false;

	if (strlen(sn) < MD5_LENGTH * 2)
		return false;

	if (str2md5hex(sn, sn_md5) < 0)
		return false;

	for (i = 0; i < 10; i++) {
		if (sn_md5[i] != mc_md5[i])
			return false;
	}

	for (i = 10; i < MD5_LENGTH; i++) {
		if (sn_md5[i] != salt_md5[i])
			return false;
	}

	return true;
}

struct match_operations ma01_ops = {
	.match = ma01_match,
};

int init_ma01(void)
{
	unsigned char salt[] = {118,118,118,45,104,106,116,96,104,55,45,98,110,108,0};
	int i;

	for (i=0; i<sizeof(salt)-1; i++)
		salt[i]+=1;
	memset(salt_md5, 0x0, sizeof(salt_md5));
	if (get_md5(salt, salt_md5) < 0)
		return -1;
	return register_match_algorithm(MA01_IKVERSION, &ma01_ops);
}

void exit_ma01(void)
{
	unregister_match_algorithm(MA01_IKVERSION);
}
