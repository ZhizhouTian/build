#include <linux/time.h>
#include <linux/netdevice.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>

#include "gv.h"

#define MD5_LENGTH     16

static unsigned char salt[] = {0x51,0x43,0x2d,0x48,0x6a,0x74,0x60,0x68,0x37,0x63,0x6e,0x73,0x42,0x6e,0x6c};

static int get_md5(char *input, size_t input_len, unsigned char *output)
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

	sg_init_one(&sg, input, input_len);
	crypto_hash_init(&desc);

	crypto_hash_update(&desc, &sg, input_len);
	crypto_hash_final(&desc, output);

	crypto_free_hash(tfm);

	return 0;
}

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

bool ma01_match(char* mc, char *sn)
{
	unsigned char mc_md5[MD5_LENGTH];
	unsigned int i;
	unsigned char sn_md5[MD5_LENGTH];
	unsigned char input[MC_LEN + sizeof(salt) + 1];
	size_t mc_len = strnlen(mc, MC_LEN);

	strncpy(input, mc, mc_len);
	strncpy(input+mc_len, salt, sizeof(salt));
	input[mc_len+sizeof(salt)] = '\0';

	if (get_md5(input, mc_len+sizeof(salt), mc_md5) < 0)
		return false;

	if (strlen(sn) < MD5_LENGTH * 2)
		return false;

	if (str2md5hex(sn, sn_md5) < 0)
		return false;

	for (i = 0; i < MD5_LENGTH; i++) {
		if (sn_md5[i] != mc_md5[i])
			return false;
	}

	return true;
}

int init_ma01(void)
{
	int i;

	/* adjust salt */
	for (i=0; i<sizeof(salt); i++)
		salt[i]+=1;
	return 0;
}

void exit_ma01(void)
{
}
