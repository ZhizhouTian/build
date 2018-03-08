#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/crypto.h>
#include <linux/err.h>

struct data {
	int cpuid;
};

struct percpu_data {
	struct data *data;
};

struct percpu_data __percpu *datas;

static int __init percpu_init(void)
{
	int cpu;

	datas = alloc_percpu(struct percpu_data);

	for_each_online_cpu(cpu) {
		struct data *d = kzalloc(sizeof(struct data), GFP_KERNEL);

		if (!d) {
			pr_emerg("kzalloc percpu data failed\n");
			return -1;
		}

		d->cpuid = cpu;
		per_cpu_ptr(datas, cpu)->data = d;
	}

	for_each_online_cpu(cpu) {
		pr_info("cpu(%d) id:%d\n",
			cpu, per_cpu_ptr(datas, cpu)->data->cpuid);
	}

	return 0;
}

static void __exit percpu_exit(void)
{

}

module_init(percpu_init);
module_exit(percpu_exit);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Me");
