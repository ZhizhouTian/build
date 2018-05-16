#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "main.h"

#define NETLINK_TEST 20
#define MSG_LEN 100

int main(int argc, char* argv[])
{
	int fd = open("urllist.bin", O_RDONLY);
	char data[1024];
	struct stat stat;

	struct sockaddr_nl local, dest;
	int skfd, ret;
	unsigned int datalen, addrlength;
	struct nlmsghdr *msg_out, *msg_in;
	struct cntl_msg *cm;

	skfd = socket(PF_NETLINK, SOCK_RAW, NETLINK_TEST);
	if(skfd < 0){
		perror("create socket failed");
		return 0;
	}

	if (fd < 0) {
		printf("open urllist.bin failed\n");
		return 0;
	}

	fstat(fd, &stat);
	read(fd, data, stat.st_size);

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

	datalen = sizeof(*cm) + stat.st_size;
	msg_out = (struct nlmsghdr*)malloc(NLMSG_SPACE(datalen));
	memset(msg_out, 0, NLMSG_SPACE(datalen));
	msg_out->nlmsg_len = NLMSG_SPACE(datalen);
	msg_out->nlmsg_flags = 0;
	msg_out->nlmsg_type = 0;
	msg_out->nlmsg_seq = 0;
	msg_out->nlmsg_pid = local.nl_pid;

	cm = NLMSG_DATA(msg_out);
	cm->len = stat.st_size;
	memcpy(cm->data, data, stat.st_size);
	ret = sendto(skfd, msg_out, msg_out->nlmsg_len, 0, (struct sockaddr*)&dest, sizeof(struct sockaddr_nl));
	if(!ret){
		perror("send failed");
		free(msg_out);
		close(skfd);
		return 0;
	}
	free(msg_out);

#if 0
	msg_in = (struct nlmsghdr*)malloc(NLMSG_SPACE(MSG_LEN));
	memset(msg_in, 0, NLMSG_SPACE(MSG_LEN));

	addrlength = sizeof(struct sockaddr_nl);
	ret = recvfrom(skfd, msg_in, NLMSG_SPACE(MSG_LEN), 0, (struct sockaddr*)&dest, &addrlength);
	if(!ret){
		perror("recv failed");
		free(msg_in);
		close(skfd);
		return 0;
	}

	printf("message received from kernel: %s\n", (char*)NLMSG_DATA(msg_in));
	free(msg_in);
#endif
	close(skfd);

	return 0;
}
