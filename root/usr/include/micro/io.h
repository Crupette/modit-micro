#ifndef LIB_MICRO_IO_H
#define LIB_MICRO_IO_H 1

#include "api/syscall.h"

DECL_SYSCALL1(print, char*)

int reqio(unsigned short base, unsigned short len);
int blkio(unsigned short base, unsigned short len);

unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char byte);

#endif
