#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <asm-generic/atomic-long.h>

static void show_task_rss(void)
{
	struct task_struct *task;
	rcu_read_lock();
	for_each_process(task) {
		struct mm_struct *mm = get_task_mm(task);
		unsigned long fpages;
		if (!mm)
			continue;
		fpages = atomic_long_read(&mm->rss_stat.count[MM_FILEPAGES]);
		pr_info("%s\t\t%lu Kb.\n", task->comm, fpages * 4);
	}
	rcu_read_unlock();
}

static __init int ps_rss_init(void)
{
	show_task_rss();

	return 0;
}

static __exit void ps_rss_exit(void)
{

}

module_init(ps_rss_init);
module_exit(ps_rss_exit);
MODULE_LICENSE("GPL");
