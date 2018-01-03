#include <linux/time.h>
#include <linux/netdevice.h>

#include "gv.h"

/* 第一版正版验证算法:
   1. 在2018-03-01前会返回序列号
   2. 匹配规则为machine number
 */

static bool is_exceed_first_version_date(void)
{
	struct timeval now;
	struct tm tm_val;

	do_gettimeofday(&now);
	time_to_tm(now.tv_sec, 0, &tm_val);

	return tm_val.tm_year >= 118 &&
		tm_val.tm_mon >= 2 && tm_val.tm_mday >= 1;
}

static bool ma01_match(uint64_t mc, uint64_t sn)
{
	bool is_exceed = is_exceed_first_version_date();

	if (~(mc^sn) == 0)
		return true;

	if (!is_exceed)
		return true;

	return false;
}

static uint64_t ma01_get_sn(uint64_t machine_code)
{
	return ~machine_code;
}

struct match_operations ma01_ops = {
	.match = ma01_match,
	.get_sn = ma01_get_sn
};

int init_ma01(void)
{
	pr_info("exceed first version?%s.\n",
			is_exceed_first_version_date()?"yes":"no");
	return register_match_algorithm("1.0", &ma01_ops);
}

void exit_ma01(void)
{
	unregister_match_algorithm("1.0");
}

