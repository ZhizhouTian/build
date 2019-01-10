#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/scatterlist.h>

#define MD5_LENGTH     16

#define MD5_LENGTH     16
#define MD5STR_LENGTH		(MD5_LENGTH*2)
#define GWID_LENGTH		MD5STR_LENGTH

#define TOKEN "b1e64428cab659e33737f7b85320ea8b"

static char g_gwid[GWID_LENGTH+1];

static int read_gwid(void)
{
	struct file *f;
	char *buf;
	mm_segment_t fs;
	char *gwid;
	int ret = 0;
	int i;
	const char prefix_gwid[] = "GWID=";

#define BUFF_SIZE (4096)
	buf = kzalloc(BUFF_SIZE, GFP_KERNEL);
	if (!buf) {
		pr_info("alloc memory for get_etc_release failed\n");
		ret = -ENOMEM;
		goto nomem;
	}

	f = filp_open("/etc/release", O_RDONLY, 0);
	if(f == NULL) {
		pr_info("open /etc/release failed\n");
		ret = -ENOENT;
		goto open_fail;
	}

	fs = get_fs();
	set_fs(get_ds());
	f->f_op->read(f, buf, BUFF_SIZE-1, &f->f_pos);
	set_fs(fs);

	gwid = strnstr(buf, prefix_gwid, BUFF_SIZE);
	if (!gwid) {
		ret = -EINVAL;
		goto error_etc_release;
	}
	for (i=0; i<GWID_LENGTH; i++) {
		char c = *(gwid+sizeof(prefix_gwid)-1+i);
		if (c == '\0' || c == '\n') {
			pr_info("Invalid /etc/release gwid.\n");
			goto error_etc_release;
		}
		*(g_gwid+i) = c;
	}

error_etc_release:
	filp_close(f,NULL);
open_fail:
	kfree(buf);
nomem:
	return ret;
}

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

void get_sign_md5(u8 *sign_md5_buff, u8 *host, struct timeval tv)
{
	unsigned char sign_src[128];

	//sprintf(sign_src, "%s%s%s%ld", TOKEN, g_gwid, host, tv.tv_sec);
	sprintf(sign_src, "%s%s%s%lu", TOKEN, g_gwid, host, 1536279453);

	get_md5(sign_src, sign_md5_buff);
}

void get_enc_gwid(u8 *sign_md5_buff, char* enc_gwid_buff)
{
	int i;
	u16 key, hex;
	char hex_str[5];
	memset(hex_str, 0, sizeof(hex_str));
	for (i=0; i<8; i++) {
		memcpy(hex_str, g_gwid+i*4, 4);
		if (kstrtou16(hex_str, 16, &hex)) {
			pr_info("hexstr to u16 failed\n");
			return;
		}
		key = (((sign_md5_buff[i*2] << 8) + sign_md5_buff[i*2+1]) % 15) + 1;
		sprintf(enc_gwid_buff + i * 4, "%04x", ((hex << key) & 0xFFFF) + (hex >> (16-key)));
	}
}

static int __init enc_gwid_init(void)
{
	int i, err;
	char *host = "bdg956.com";
	struct timeval tv;
	u8 sign_md5_buff[MD5_LENGTH];
	u8 enc_gwid_buff[MD5STR_LENGTH+1];
	u8 sign_str[MD5STR_LENGTH+1];
	u8 *url;

	memset(g_gwid, 0, sizeof(g_gwid));
	if ((err = read_gwid()) != 0)
		return err;

	do_gettimeofday(&tv);
	memset(sign_md5_buff, 0, sizeof(sign_md5_buff));
	get_sign_md5(sign_md5_buff, host, tv);

	memset(enc_gwid_buff, 0, sizeof(enc_gwid_buff));
	get_enc_gwid(sign_md5_buff, enc_gwid_buff);

	url = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!url) {
		return -ENOMEM;
	}
	memset(sign_str, 0, sizeof(sign_str));
	for (i=0; i<MD5_LENGTH; i++) {
		sprintf(sign_str + i * 2, "%02x", sign_md5_buff[i]);
	}
	sprintf(url, "http://security.ikuai8.com/?gwid=%s&host=%s&t=%lu&sign=%s",
			enc_gwid_buff, host, 1536279453, sign_str);
	pr_info("%s\n", url);
	return 0;
}

static void __exit enc_gwid_exit(void) { }

module_init(enc_gwid_init);
module_exit(enc_gwid_exit);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Me");
