#include <linux/time.h>
#include <linux/netdevice.h>

#include "gv.h"

#define MA01_IKVERSION "3.0.0"

static void ma01_generate_serial_number(char *mc, char *sn)
{
	strncpy(sn, mc, SN_LEN);
}

static bool ma01_match(char* mc, char *sn)
{
	char snbuf[SN_LEN];

	ma01_generate_serial_number(mc, snbuf);

	if (!strncmp(snbuf, sn, MC_LEN))
		return true;

	return false;
}

struct match_operations ma01_ops = {
	.match = ma01_match,
};

int init_ma01(void)
{
	return register_match_algorithm(MA01_IKVERSION, &ma01_ops);
}

void exit_ma01(void)
{
	unregister_match_algorithm(MA01_IKVERSION);
}
