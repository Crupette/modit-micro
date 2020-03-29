/*	MODLOADER.H -	Simple module loader, loading relocatable ELF objects
 *			Maps new modules to 0xD8000000
 *			Links against the kernel and module dependencies
 *
 *	Author: Crupette
 * */

#ifndef MOD_LOADER_H
#define MOD_LOADER_H 1

#include "elf.h"
#include "kernel/initrd.h"

#define MODULES_START 0xD8000000

typedef struct module_info {
	initrd_file_t *file;
	char *name;

	struct elf32_shdr *symtab_hdr;
	struct elf32_shdr *strtab_hdr;

	struct elf32_shdr *sheaders;
	struct elf32_sym *symtab;
	char *strtab;

	uint8_t deplist[32];
	uint8_t deps;

	int (*enter)(void);
	int (*exit)(void);
} module_info_t;

#define module_name(name) \
	char* _module_name = #name

#define module_load(load) \
	void* _module_load = load

#define module_unload(unload) \
	void* _module_unload = unload

#define module_depends(mod) \
	char *_module_depends_ ## mod __attribute__((section(".moddeps"), used)) = #mod


/*	Initializes the module loader
 *		Automatically loads all modules present on the initrd (all files ending in .ko)
 * */
void kmod_init(multiboot_info_t *mbinfo);

/*	Loads the module from initrd file [file]
 *	file: file to load
 *	return: module id number
 * */
unsigned int kmod_load(initrd_file_t *file);

#endif
