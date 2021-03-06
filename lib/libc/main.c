/*  The standard C library assumes the existance of a VFS, files, device drivers, etc
 *  so the lib_micro library will have to provide hooks for these operations
 *  this library will assume that a file server, process registry, device drivers exist
 * */

#include <stddef.h>
#include <stdbool.h>

extern void _init();
extern void _fini();

extern void __heap_setup(bool pg);
extern void __stdio_init(void);

void pre_main(int (*main)(int, char**), int argc, char **argv, char **env){
    __heap_setup(true);
    __stdio_init();
    main(argc, argv);
}
