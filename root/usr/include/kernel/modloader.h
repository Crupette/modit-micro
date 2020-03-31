/*  MODLOADER.H -   Simple module loader, loading relocatable ELF objects
 *          Maps new modules to 0xD8000000
 *          Links against the kernel and module dependencies
 *
 *  Author: Crupette
 * */

#ifndef MOD_LOADER_H
#define MOD_LOADER_H 1

#include "elf.h"
#include "kernel/initrd.h"

#define MODULES_START 0xD8000000

typedef struct module_info {
    initrd_file_t *file;        //Pointer to initrd file
    char *name;         //Pointer to name in programs strtab

    struct elf32_shdr *symtab_hdr;  //Header to the symtab
    struct elf32_shdr *strtab_hdr;  //Header to the strtab

    struct elf32_shdr *sheaders;    //Beginning of headers
    struct elf32_sym *symtab;   //Symbol table (actual memory)
    char *strtab;           //String table (actual memory)

    uint8_t deplist[32];        //List of dependencies in (32 max) as module id's
    uint8_t deps;           //Number of dependencies

    int (*enter)(void);     //Entry function pointer
    int (*exit)(void);      //Exit function pointer
} module_info_t;

#define module_name(name) \
    char* _module_name = #name

#define module_load(load) \
    int (*_module_load)(void) = load

#define module_unload(unload) \
    int (*_module_unload)(void) = unload

#define module_depends(mod) \
    char *_module_depends_ ## mod __attribute__((section(".moddeps"), used)) = #mod


/*  Initializes the module loader
 *      Automatically loads all modules present on the initrd (all files ending in .ko)
 * */
void kmod_init();

/*  Loads the module from initrd file [file]
 *  file: file to load
 *  return: module id number, -1 if failed
 * */
int kmod_load(initrd_file_t *file);

#endif
