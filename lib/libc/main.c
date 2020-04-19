/*  The standard C library assumes the existance of a VFS, files, device drivers, etc
 *  so the lib_micro library will have to provide hooks for these operations
 *  this library will assume that a file server, process registry, device drivers exist
 * */

extern void _init();
extern void _fini();

void pre_main(int (*main)(int, char**), int argc, char **argv, char **env){
    main(argc, argv);
}
