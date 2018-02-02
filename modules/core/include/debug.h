#ifndef __DEBUG_H__
#define __DEBUG_H__

#define DEBUG_CORE

#ifdef DEBUG_CORE
#define pr_info_log(fmt, ...) \
	pr_info("[core:%s] "fmt, __func__, ##__VA_ARGS__)
#else
#define pr_info_log(fmt, ...)
#endif

#endif
