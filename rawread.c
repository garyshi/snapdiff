#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

const int ALIGN = 4096;

const void *align_ptr(const void *p, int alignment)
{
	const void *q = p + alignment - 1;
	int mod = ((uint64_t)q) % alignment;
	return q - mod;
}

int64_t dev_get_size(int fd)
{
	int64_t size;
	if (ioctl(fd, BLKGETSIZE64, &size) < 0) {
		perror("ioctl BLKGETSIZE64");
		return -1;
	}
	return size;
}

int main(int argc, char *argv[])
{
	char *filename, *pc;
	int fd, n;
	int64_t size, offset;
	uint8_t buf[ALIGN*2], *p;
	FILE *fpout;

	size = strtoull(argv[3], &pc, 10);
	if (*pc == 'k' || *pc == 'K') size <<= 10;
	else if (*pc == 'm' || *pc == 'M') size <<= 20;
	else if (*pc == 'g' || *pc == 'G') size <<= 30;
	printf("size=%lld, pc=%d\n", size, *pc);

	fd = open(argv[1], O_RDONLY|O_DIRECT);
	if (fd < 0) {
		perror("open");
		return 1;
	}

	fpout = fopen(argv[2], "wb");
	if (fpout == NULL) {
		printf("Error: failed open %s for write\n", argv[2]);
		return 1;
	}

	p = (void *)align_ptr(buf, ALIGN);
	for (offset = 0; offset < size; offset += ALIGN) {
		n = read(fd, p, ALIGN);
		if (n != ALIGN) {
			printf("Error: read returns %d\n", n);
			break;
		}
		fwrite(p, ALIGN, 1, fpout);
	}

	fclose(fpout);
	close(fd);

	return 0;
}
