#include "micro/file.h"
#include "micro/io.h"
#include "kernel/initrd.h"

DEFN_SYSCALL2(initrd_getf, SYSCALL_INITRD_GETF, initrd_file_t *, const char*)

static size_t nextfd = 3;
static struct FILE filebuf = { 0 };

int micro_write(struct FILE *file, const char *buffer, size_t len){
    if((file->flg & FILE_FLG_PRINT)){
        return syscall_print(buffer);
    }
    return -1; //TODO: File read / write
}

struct FILE *micro_open(const char *restrict path, uint32_t flags){
    //TODO: Search for file in vfs server. This should be fallback
    int fi = syscall_initrd_getf(&(filebuf.desc), path);
    //if(fi < 0){
    //    return 0;
    //}
    filebuf.fd = nextfd++;
    filebuf.flg = flags | FILE_FLG_INITRD;

    return &filebuf;
}
