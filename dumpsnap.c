#include "common.h"

struct snapshot_info {
	int fd;
	int64_t devsize;
	uint32_t magic, valid, version;
	int chunk_sectors, chunk_size;
	int exc_per_area, area_size, num_areas;
};

inline int64_t area_to_chunk(struct snapshot_info *info, int64_t area)
{
	return (info->exc_per_area + 1) * area + 1;
}

int read_chunk(struct snapshot_info *info, int64_t chunk, void *buf)
{
	uint8_t buffer[info->chunk_size+ALIGN];
	off64_t offset = chunk * info->chunk_size;
	void *p = align_ptr(buffer, ALIGN);
	int n, len = info->chunk_size;

	n = len % ALIGN;
	if (n) len += ALIGN - n;
	n = offset % ALIGN;
	if (n) offset -= n;

	if (lseek64(info->fd, offset, SEEK_SET) < 0) {
		perror("lseek64");
		return -1;
	}

	n = read(info->fd, p, len);
	if (n != len) {
		fprintf(stderr, "read returns %d (expecting %d)\n", n, len);
		return -1;
	}

	memcpy(buf, p + (chunk * info->chunk_size - offset), info->chunk_size);

	return 0;
}

int read_header(struct snapshot_info *info)
{
	uint8_t buf[4096];

	info->devsize = dev_get_size(info->fd);
	if (info->devsize < 0) return -1;

	info->chunk_size = sizeof buf;
	if (read_chunk(info, 0, buf) < 0) return -1;

	memcpy(&info->magic, buf+0, 4);
	memcpy(&info->valid, buf+4, 4);
	memcpy(&info->version, buf+8, 4);
	memcpy(&info->chunk_sectors, buf+12, 4);

	printf("header magic: %08x\n", info->magic);
	printf("header valid: %d\n", info->valid);
	printf("header version: %d\n", info->version);
	printf("chunk_sectors: %d\n", info->chunk_sectors);

	if (info->magic != 0x70416e53) return -1;
	if (info->valid != 1) return -1;
	if (info->version != 1) return -1;

	info->chunk_size = info->chunk_sectors << 9;
	info->exc_per_area = info->chunk_size / 16;
	info->area_size = (info->exc_per_area + 1) * info->chunk_size;
	info->num_areas = info->devsize / info->area_size - 1;

	printf("chunk_size: %d\n", info->chunk_size);
	printf("exc_per_area: %d\n", info->exc_per_area);
	printf("area_size: %d\n", info->area_size);
	printf("num_areas: %d\n", info->num_areas);

	return 0;
}

int dump_area(struct snapshot_info *info, int64_t area)
{
	uint8_t data[info->chunk_size];
	int64_t chunk = area_to_chunk(info, area);
	int64_t *old_chunk, *new_chunk;
	int i, num_excs;

	if (read_chunk(info, chunk, data) < 0) return -1;

	for (i = 0, num_excs = 0;  i < info->exc_per_area * 16;  i += 16, ++ num_excs) {
		old_chunk = (int64_t *)(data + i);
		new_chunk = (int64_t *)(data + i + 8);
		if (*new_chunk == 0) break;
		printf("COW %lld => %lld\n", *old_chunk, *new_chunk);
	}

	return num_excs;
}

int main(int argc, char *argv[])
{
	const char *filename = argv[1];
	int64_t size, area, num_excs;
	struct snapshot_info info;
	int n;

	info.fd = open(filename, O_RDONLY|O_DIRECT);
	if (info.fd < 0) {
		perror("open");
		return 1;
	}

	read_header(&info);
	for (area = 0, num_excs = 0;  area < info.num_areas;  ++ area) {
		n = dump_area(&info, area);
		if (n > 0) num_excs += n;
		if (n != info.exc_per_area) break;
	}
	printf("Total exceptions: %lld\n", num_excs);

	close(info.fd);

	return 0;
}
