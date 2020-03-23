/*	INITRD.H -	Defines the root kernel's handling of the TAR-based initrd.
 *
 *	Author: Crupette
 * */

#ifndef INITRD_H
#define INITRD_H 1

#include <stdint.h>
#include "kernel/multiboot.h"

typedef struct tar_header {
	char name[100];
	uint8_t mode[8];	//File type
	uint8_t uid[8];		//User id
	uint8_t gid[8];		//Group id
	uint8_t size[12];	//Size in octal
	uint8_t mtime[12];	//Time mounted
	uint8_t chksum[8];	//Checksum
	uint8_t type;		//File or SYMLINK?
} tar_header_t;

typedef struct initrd_file {
	char *name;
	uint8_t *header_start;
	uint8_t *data_start;
	uint32_t size;
} initrd_file_t;

/*	Loads initrd headers into kernel memory, to be used by the kernel-side module loader
 *	The first module seen through mbinfo is assumed to be the initrd.
 *	If no files can be retrieved by parsing the initrd, loading fails
 *	If no modules are loaded, loading fails.
 *	Requires initialization of pmm and vmm before use
 * */
void initrd_init(multiboot_info_t *mbinfo);

/*	Returns the list of initrd files, and sets the passed value to number of files
 * */
initrd_file_t *initrd_get_files(uint32_t *n);

#endif
