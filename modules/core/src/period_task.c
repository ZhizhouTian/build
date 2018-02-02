#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include "period_task.h"
#include "debug.h"

MODULE_LICENSE("GPL");

#if 0
typedef void(*period_task)(u32);
struct period_task_item {
	period_task check_fp;
	bool is_run_on_all_cpu;
};

static struct period_task_item ptitems[] = {
	{
		.check_fp = stat_period_task,
		.is_run_on_all_cpu = true,
	},
};

void period_timer_func(unsigned long data)
{

}

#endif
static DEFINE_PER_CPU(struct task_struct *, g_percpu_daemon);

int percpu_daemon_thread(void *data)
{
	uint32_t loop_cnt = 0;
	uint32_t remaining = 0;

	while (!kthread_should_stop()) {
		msleep(100);
		loop_cnt++;
		remaining = loop_cnt % 10;

		switch(remaining) {
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		}
	}

	return 0;
}

void percpu_daemon_init(void)
{
	int cpu;

	for_each_online_cpu(cpu) {
		struct task_struct *p = per_cpu(g_percpu_daemon, cpu);
		if (!p) {
			p = kthread_create(percpu_daemon_thread, NULL,
					"daemon/%d", cpu);
			if (!p) {
				pr_info_log("create percpu daemon failed.\n");
				break;
			}
			kthread_bind(p, cpu);
			per_cpu(g_percpu_daemon, cpu) = p;
			wake_up_process(p);
		}
	}
}

void percpu_daemon_exit(void)
{
	int cpu;

	for_each_online_cpu(cpu) {
		struct task_struct *p = per_cpu(g_percpu_daemon, cpu);
		if (p) {
			kthread_stop(p);
			per_cpu(g_percpu_daemon, cpu) = NULL;
		}
	}
}
