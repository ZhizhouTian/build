#include <linux/init.h>
#include <linux/module.h>

#include "http_parser.h"

static int __init ik_http_init(void)
{
	return 0;
}

static void __exit ik_http_exit(void)
{

}

module_init(ik_http_init);
module_exit(ik_http_exit);
