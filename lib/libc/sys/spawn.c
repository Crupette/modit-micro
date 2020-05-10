#include <sys/proc.h>
#include "micro/task.h"
#include "api/syscall.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern const char **__stitch_args(va_list ap, const char *path, const char *arg, int *argc);
extern uint8_t *__construct_ustk(const char *argv[], size_t *stksz);

extern void __send_preg(const char *path, pid_t target);

pid_t spawnl(const char *path, const char *arg, ...){
    if(path == 0) return -1;
    
    va_list ap;
    va_start(ap, arg);
    
    int argc = 0;
    const char **argv = __stitch_args(ap, path, arg, &argc);

    va_end(ap);

    int r = spawnv(path, argv);
    free(argv);
    return r;
}

pid_t spawnv(const char *path, const char *argv[]){
    FILE *file = micro_open(path, 0); //TODO: FS flags
    if(file == 0) return -1;    //TODO: EINVAL

    size_t stksz = 0;
    uint8_t *ustk = __construct_ustk(argv, &stksz);

    //TODO: Files other than initrd
    int r = micro_spawn(file, ustk, stksz);

    if(r < 0){
        //TODO: Set errno
        return r;
    }
    //Register new process with process server
    __send_preg(path, r);

    free(ustk);
    return r;
 
}
