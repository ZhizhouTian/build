#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include <linux/netlink.h>

#include "../netlink_cntl.h"

#define NETLINK_CTL 30

int netlink_msg_send(void *data, int datalength)
{
	int skfd;
	int ret;
	unsigned int addrlength;
	struct sockaddr_nl local;
	struct sockaddr_nl dest;
	struct nlmsghdr *msg_out;

	skfd = socket(PF_NETLINK, SOCK_RAW, NETLINK_CTL);
	if(skfd < 0){
		perror("create socket failed");
		return 0;
	}

	memset(&local, 0, sizeof(struct sockaddr_nl));
	local.nl_family = AF_NETLINK;
	local.nl_pid = getpid();
	local.nl_groups = 0;
	if(bind(skfd, (struct sockaddr *)&local, sizeof(struct sockaddr_nl)) != 0){
		perror("bind failed");
		close(skfd);
		return 0;
	}


	memset(&dest, 0, sizeof(struct sockaddr_nl));
	dest.nl_family = AF_NETLINK;
	dest.nl_pid = 0;
	dest.nl_groups = 0;

	msg_out = (struct nlmsghdr*)malloc(NLMSG_SPACE(datalength));
	memset(msg_out, 0, NLMSG_SPACE(datalength));
	msg_out->nlmsg_len = NLMSG_SPACE(datalength);
	msg_out->nlmsg_flags = 0;
	msg_out->nlmsg_type = 0;
	msg_out->nlmsg_seq = 0;
	msg_out->nlmsg_pid = local.nl_pid;

	memcpy(NLMSG_DATA(msg_out), data, datalength);
	ret = sendto(skfd, msg_out, msg_out->nlmsg_len, 0, (struct sockaddr*)&dest, sizeof(struct sockaddr_nl));
	if(!ret){
		perror("send failed");
		free(msg_out);
		close(skfd);
		return 0;
	}
	free(msg_out);
	close(skfd);
	return 0;
}

int main(int argc, char* argv[])
{
	int fd = 0;
	struct netlink_cntl *cntl_buff = NULL;
	unsigned int buff_size;
	struct stat stat;

	fd = open("abc.bin", O_RDONLY);
	if (fd < 0) {
		printf("open abc.bin failed\n");
		return 0;
	}

	fstat(fd, &stat);
	buff_size = sizeof(cntl_buff) + stat.st_size;
	cntl_buff = malloc(buff_size);
	if (!cntl_buff) {
		printf("alloc cntl buffer failed\n");
		return 0;
	}

	cntl_buff->data_len = stat.st_size;
	read(fd, &cntl_buff->data, stat.st_size);
	netlink_msg_send(cntl_buff, buff_size);

	free(cntl_buff);
	close(fd);

	return 0;
}
