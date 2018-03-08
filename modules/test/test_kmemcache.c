#include <linux/module.h>
#include <linux/slab.h>

struct kmem_cache *my_kmem_cache = NULL;

struct my_data{
	int value;
};

#define MY_DATA_CNT 32768
struct my_data *mydata[MY_DATA_CNT];

static int __init test_kmem_cache_init(void)
{
	int i = 0;

	my_kmem_cache =
		kmem_cache_create("my_kmem_cache", sizeof(struct my_data),
		0, SLAB_HWCACHE_ALIGN | SLAB_NOTRACK | IK_SLAB_NOMERGE, NULL);

	pr_info("cachep : %p\n", my_kmem_cache);

	for (i=0; i<MY_DATA_CNT; i++) {
		mydata[i] = kmem_cache_alloc(my_kmem_cache, GFP_KERNEL);
		if (!mydata[i]) {
			pr_info("alloc from my_kmem_cache failed\n");
			return -1;
		}
		mydata[i]->value = 1;
	}

	return 0;
}

static void __exit test_kmem_cache_exit(void)
{
	int i = 0;

	for (i=0; i<MY_DATA_CNT; i++) {
		if (mydata[i] != NULL)
			kmem_cache_free(my_kmem_cache, mydata[i]);
	}
	kmem_cache_destroy(my_kmem_cache);
}

module_init(test_kmem_cache_init);
module_exit(test_kmem_cache_exit);
MODULE_LICENSE("GPL");
