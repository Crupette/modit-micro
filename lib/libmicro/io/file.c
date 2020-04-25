#include "micro/file.h"
#include "micro/io.h"

int micro_write(struct FILE *file, const char *buffer, size_t len){
    if(file->fd == FD_PRINT){
        return syscall_print(buffer);
    }
    return -1; //TODO: File read / write
}
