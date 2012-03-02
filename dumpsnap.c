#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

typedef long long int64;

int64 dev_get_size(int fd)
{
	int64 size;
	if (ioctl(fd, BLKGETSIZE64, &size) < 0) {
		perror("ioctl BLKGETSIZE64");
		return -1;
	}
	return size;
}

int dev_read(int fd, off64_t pos, void *buf, int len)
{
	void *p = buf;
	int n;

	if (lseek64(fd, pos, SEEK_SET) < 0) {
		perror("lseek64");
		return -1;
	}

	while (len > 0) {
		n = read(fd, p, len);
		if (n <= 0) break;
		p += n;
		len -= n;
	}

	if (p > buf) return p - buf;
	return n;
}

int main(int argc, char *argv[])
{
	char *filename;
	int fd;

	filename = argv[1];
	fd = open(filename, O_RDONLY|O_DIRECT);
	if (fd < 0) {
		perror("open");
		return 1;
	}

	printf("%lld\n", dev_get_size(fd));

	close(fd);

	return 0;
}
