#ifndef __NETLINK_CNTL_H__
#define __NETLINK_CNTL_H__

struct netlink_cntl {
	unsigned int data_len;
	char data[0];
};

#endif
