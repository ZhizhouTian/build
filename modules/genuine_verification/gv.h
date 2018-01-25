#ifndef __GV_H__
#define __GV_H__

#include <linux/types.h>

#define MAX_IKVERSION_LEN 32
#define MAX_SERIAL_NUMBER_LEN 32
#define MC_LEN 32

struct match_operations {
	bool (*match) (char*, char*);
};

void generate_machine_code(char mcbuf[MC_LEN]);
int register_match_algorithm(const char *, struct match_operations *);
void unregister_match_algorithm(const char *);

int init_ma01(void);
void exit_ma01(void);
#endif
