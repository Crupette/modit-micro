/*	MEMORY.H -	Provides a rudimentary PMM and VMM for the base kernel to use
 *			Needed to give modules a concrete address and reserve memory for future use
 *
 *	Author:	Crupette
 * */

#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "kernel/multiboot.h"

//================= PMM ============================

#define VIRT_BASE 0xE0000000
#define BM_SIZE 1024
#define PAGE_SIZE 4096

/*	Initializes both physical and virtual memory managers for the kernel
 *	PMM can access 16MB of physical memory
 *	VMM can allocate anywhere, replacing the bootloader's 4MB pages
 *
 *	mbinfo: Pointer to multiboot info in virtual memory to retrieve mmap
 * */
void kmem_init(multiboot_info_t *mbinfo);

/*	Adds a block of memory to be indexable by the kernel.
 *
 *	start: Beginning byte of memory wished to be reserved
 *	n:     Number of pages to reserve
 * */
void kpmm_addblock(void *start, uint16_t n);

/*	Interfaces to allocate and free a physical page (4096 aligned).
 *	getpg returns pointer to first byte (in physical memory) of allocated page
 *	freepg frees the physical memory from use
 * */
void *kpmm_getpg();
void kpmm_freepg(void *ptr);

/*	Reserves a page to be blocked from use
 * */
void kpmm_resvpg(void *ptr);

//================= VMM ============================

typedef struct page {
	union {
		uint32_t entry;
		struct {
			uint8_t present		: 1; //Present
			uint8_t rw		: 1; //Writeable
			uint8_t usr		: 1; //User page
			uint8_t wt		: 1; //Write-through
			uint8_t cache		: 1; //Cache disabled
			uint8_t accessed	: 1; //Accessed
			uint8_t dirty		: 1; //Zero / Dirty
			uint8_t sg		: 1; //Size / Global
			uint8_t reserved	: 3; //Reserved
			uint32_t addr		: 20;
		}__attribute__((packed));
	};
} page_t;

typedef struct page_table {
	page_t pages[1024];
} page_table_t;

typedef struct page_directory {
	page_t tablesPhys[1024];
	page_table_t *tables[1024];

	uintptr_t phys;
} page_directory_t;

/*	Maps the address phys to virt using current pdir
 *
 *	phys:	Physical address to map
 *	virt:	Virtual address to map to
 *	flags:	Flags to apply for the page
 * */
void kvmm_mappg(void *phys, void *virt, uint32_t flags);

/*	alloc: Maps a free block of physical memory to virtual address [req]
 *	free:  Releases a block of physical memory from binding [req]
 *
 *	req:	Requested virtual address
 * */
void *kvmm_allocpg(void *req);
void kvmm_freepg(void *req);

#endif
