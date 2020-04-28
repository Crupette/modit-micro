#include "micro/io.h"

DEFN_SYSCALL1(print, SYSCALL_PRINT, const char*)

DEFN_SYSCALL2(reqio, SYSCALL_REQIO, unsigned short, unsigned short)
DEFN_SYSCALL2(blkio, SYSCALL_BLKIO, unsigned short, unsigned short)

int reqio(unsigned short base, unsigned short len){
    return syscall_reqio(base, len);
}

int blkio(unsigned short base, unsigned short len){
    return syscall_blkio(base, len);
}

unsigned char inb(unsigned short port){
    unsigned char r = 0;
    asm volatile("inb %1, %0": "=a"(r): "dN"(port));
    return r;
}

void outb(unsigned short port, unsigned char byte){
    asm volatile("outb %1, %0":: "dN"(port), "a"(byte));
}
