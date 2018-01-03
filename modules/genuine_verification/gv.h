#ifndef __GV_H__
#define __GV_H__

#include <linux/types.h>

struct match_operations {
	bool (*match) (uint64_t , uint64_t);
	uint64_t (*get_sn) (uint64_t);
};

int register_match_algorithm(const char *, struct match_operations *);
void unregister_match_algorithm(const char *);

int init_ma01(void);
void exit_ma01(void);
#endif
