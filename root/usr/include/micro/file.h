#ifndef MICRO_FILE_H
#define MICRO_FILE_H 1

#include <stdint.h>
#include <stddef.h>

#include "kernel/initrd.h"

#define FD_PRINT 0xFFFFFFFF
#define FD_MAX  0xFFFFFFFE

#define FILE_FLG_INITRD (1 << 7)
#define FILE_FLG_PRINT (1 << 6)

struct FILE {
    size_t fd;

    uint8_t flg;
    size_t line;

    char *buf;
    size_t bufi;
    size_t bufsz;

    initrd_file_t desc;
};

int micro_write(struct FILE *file, const char *buffer, size_t len);

struct FILE *micro_open(const char *restrict path, uint32_t flags);

#endif
