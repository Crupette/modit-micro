#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

const char **__stitch_args(va_list ap, const char *path, const char *arg, int *argc){
    const char **argv = malloc(16);
    size_t argi = 1;
    argv[0] = path;

    while(arg){
        argv[argi] = arg;
        arg = va_arg(ap, char*);
        argi++;
        argv = realloc(argv, sizeof(char*) * (argi + 1));
    }

    argv[argi] = NULL;

    *argc = argi;
    return argv;
}

uint8_t *__construct_ustk(const char *argv[], size_t *stksz){
    int argc = 0;
    for(argc = 0; argv[argc]; argc++){
        *stksz += 4;
        *stksz += strlen(argv[argc]);
    }
    *stksz += 8;

    uint8_t *ustk = malloc(*stksz);
    uint8_t *ustkptr = ustk + *stksz;
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

    return ustk;
}
