#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "urllist.pb-c.h"

int main(void)
{
	int fd = open("urllist.bin", O_RDONLY);
	char buf[1024];
	struct stat stat;
	URLList *uw;
	int i;

	if (fd < 0) {
		printf("open urllist.bin failed\n");
		return 0;
	}

	fstat(fd, &stat);
	read(fd, buf, stat.st_size);
	uw = urllist__unpack(NULL, stat.st_size, buf);
	if (!uw) {
		printf("unpack url list failed\n");
		return 0;
	}

	for (i=0; i<uw->n_url; i++) {
		printf("%s\n", uw->url[i]);
	}

	urllist__free_unpacked(uw, NULL);
	return 0;
}
