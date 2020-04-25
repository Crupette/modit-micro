#ifndef MICRO_FILE_H
#define MICRO_FILE_H 1

#include <stdint.h>
#include <stddef.h>

#define FD_PRINT 0xFFFFFFFF
#define FD_MAX  0xFFFFFFFE

struct FILE {
    size_t fd;

    uint8_t flg;
    size_t line;

    char *buf;
    size_t bufi;
    size_t bufsz;
};

int micro_write(struct FILE *file, const char *buffer, size_t len);

#endif
