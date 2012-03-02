#include "common.h"

void *align_ptr(const void *p, int alignment)
{
	const void *q = p + alignment - 1;
	int mod = ((uint64_t)q) % alignment;
	return (void *)(q - mod);
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
