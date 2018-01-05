#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/types.h>
#include <linux/export.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");

static __init int test_strsep_init(void)
{
	char *str = "ik3.0.0";
	char *sn = NULL, *ikver = NULL, *match;

	match = strsep(&str, " ");
	if (str) {
		ikver = match;
		sn = str;
		match = NULL;
	}

	pr_info("str:%s, ikver:%s, sn:%s.\n", str, ikver, sn);
	return 0;
}

static __exit void test_strsep_exit(void)
{
}

module_init(test_strsep_init);
module_exit(test_strsep_exit);
