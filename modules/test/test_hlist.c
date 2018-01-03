#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/types.h>
#include <linux/export.h>

MODULE_LICENSE("GPL");

void test_abc(void)
{
	int** a = kmalloc(sizeof(int*) * 1, GFP_KERNEL | __GFP_ZERO);
	*a[0] = 1;
}
EXPORT_SYMBOL(test_abc);

static __init int test_hlist_init(void)
{
	pr_emerg("hello world.\n");

	test_abc();
	return 0;
}

static __exit void test_hlist_exit(void)
{
	pr_emerg("bye world.\n");
}

module_init(test_hlist_init);
module_exit(test_hlist_exit);
