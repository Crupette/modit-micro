/*	MODLOADER.H -	Simple module loader, loading relocatable ELF objects
 *			Maps new modules to 0xD0000000
 *			Links against the kernel and module dependencies
 *
 *	Author: Crupette
 * */

#ifndef MOD_LOADER_H
#define MOD_LOADER_H 1

#include "elf.h"
#include "kernel/initrd.h"

#define module_name(name) \
	char* _module_name = #name

#define module_load(load) \
	void* _module_load = load

#define module_unload(unload) \
	void* _module_unload = unload

#define module_depends(mod) \
	char _module_depends_ ## mod [] __attribute__((section("moddeps"), used)) = #mod


/*	Initializes the module loader
 *		Automatically loads all modules present on the initrd (all files ending in .ko)
 * */
void kmod_init(multiboot_info_t *mbinfo);

/*	Loads the module from initrd file [file]
 *	file: file to load
 * */
void kmod_load(initrd_file_t *file);

#endif
