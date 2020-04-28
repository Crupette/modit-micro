/*  Fork / Exec
 * */
#include <sys/proc.h>
#include "micro/task.h"
#include "api/syscall.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

pid_t fork(void){
    int i = micro_fork();
    return i;
}

int execl(const char *path, const char *arg, ...){
    if(path == 0) return -1;
    
    va_list ap;
    va_start(ap, arg);

    const char **argv = malloc(16);
    size_t argi = 1;
    argv[0] = path;

    while(arg){
        argv[argi] = arg;
        arg = va_arg(ap, char*);
        argi++;
        argv = realloc(argv, sizeof(char*) * (argi + 1));
    }
    va_end(ap);

    argv[argi] = NULL;

    int r = execv(path, argv);
    free(argv);
    return r;
}

int execv(const char *path, const char *argv[]){
    FILE *file = micro_open(path, 0); //TODO: FS flags
    if(file == 0) return -1;    //TODO: EINVAL

    size_t stksz = 0;
    size_t argc = 0;
    while(argv[argc]){
        stksz += 4;
        stksz += strlen(argv[argc]);
        argc++;
    }
    stksz += 8;

    uint8_t *ustk = malloc(stksz);
    uint8_t *ustkptr = ustk + stksz;
    uintptr_t ptr_real = 0xD0000000;
    for(int i = 0; i < argc; i++){
        ustkptr -= (strlen(argv[i]) + 1);
        memcpy(ustkptr, argv[i], strlen(argv[i]));
        ustkptr[strlen(argv[i])] = 0;

        ptr_real -= strlen(argv[i]) + 1;
        argv[i] = (const char*)ptr_real;
    }

    ustkptr -= (sizeof(char*) * (argc + 1));
    memcpy(ustkptr, argv, sizeof(char*) * (argc + 1));

    //Add argc
    ustkptr -= sizeof(int);
    *((uintptr_t*)ustkptr) = argc;
    
    printf("Calling exec\n");

    //TODO: Files other than initrd
    int r = micro_exec(file, ustk, stksz);
    free(ustk);
    return r;
}
