#include "common.h"

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

	p = align_ptr(buf, ALIGN);
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
