#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "urlwhitelist.pb-c.h"

int main(void)
{
	int fd = open("abc.bin", O_RDONLY);
	char buf[1024];
	struct stat stat;
	URLWhitelist *uw;
	int i;

	if (fd < 0) {
		printf("open abc.bin failed\n");
		return 0;
	}

	fstat(fd, &stat);
	read(fd, buf, stat.st_size);
	uw = urlwhitelist__unpack(NULL, stat.st_size, buf);
	if (!uw) {
		printf("unpack url white list failed\n");
		return 0;
	}

	for (i=0; i<uw->n_url; i++) {
		printf("%s\n", uw->url[i]);
	}

	urlwhitelist__free_unpacked(uw, NULL);
	return 0;
}
