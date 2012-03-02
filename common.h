#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#define ALIGN 4096

void *align_ptr(const void *p, int alignment);
int64_t dev_get_size(int fd);
