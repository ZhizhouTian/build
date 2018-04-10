#ifndef __GV_H__
#define __GV_H__

#include <linux/types.h>

#define MC_LEN 21

void generate_machine_code(char mcbuf[MC_LEN]);
bool ma01_match(char* mc, char *sn);
int init_ma01(void);
void exit_ma01(void);
#endif
