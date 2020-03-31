/*  MEMORY.H -  Provides a rudimentary PMM and VMM for the base kernel to use
 *          Needed to give modules a concrete address and reserve memory for future use
 *
 *  Author: Crupette
 * */

#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "kernel/multiboot.h"

//================= PMM ============================

#define VIRT_BASE 0xE0000000
#define BM_SIZE 64
#define PAGE_SIZE 4096


//This structure is needed so that the kernel can get core components replaced by modules
typedef struct pmm_allocator {
    /*  Adds a block of memory to be indexable by the kernel.
     *  1:  Beginning byte of memory to be indexed
     *  2:  Number of pages to index
     * */
    void (*addblock)(void*, uint16_t);

    /*  Removes a block of memory from indexing
     *  1:  Beginning byte of memory to be invalidated
     *  2:  Number of pages to invalidate
     * */
    void (*rmblock)(void*, uint16_t);

    /*  Interface to allocate a physical page (4096 aligned) block of memory.
     *  Returns the next free page
     * */
    void *(*getpg)();

    /*  Deallocates a physical page of memory
     *  1:  Page address to free
     * */
    void (*freepg)(void*);

    /*  Reserves a page to be not used
     *  1:  Page address to reserve
     * */
    void (*resvpg)(void*);
} pmm_allocator_t;

extern pmm_allocator_t *physical_allocator;
/*  Initializes both physical and virtual memory managers for the kernel
 *  PMM can access 16MB of physical memory
 *  VMM can allocate anywhere, replacing the bootloader's 4MB pages
 * */
void kmem_init();

/*  Internal function used by modules to get existing physical memory map
 *  start:  Pointer to variable to store the previous start of the map
 *  end:    Pointer to variable to store previous end of the map
 *  return: Pointer to bm
 * */
uint32_t *kcore_getbm(uintptr_t *start, uintptr_t *end);

//================= VMM ============================

typedef struct page {
    union {
        uint32_t entry;
        struct {
            uint8_t present     : 1; //Present
            uint8_t rw      : 1; //Writeable
            uint8_t usr     : 1; //User page
            uint8_t wt      : 1; //Write-through
            uint8_t cache       : 1; //Cache disabled
            uint8_t accessed    : 1; //Accessed
            uint8_t dirty       : 1; //Zero / Dirty
            uint8_t sg      : 1; //Size / Global
            uint8_t reserved    : 3; //Reserved
            uint32_t addr       : 20;
        }__attribute__((packed));
    };
} page_t;

typedef struct vmm_allocator {
    /*  Maps the address phys to virt using current pdir
     *  1:  Physical address to map
     *  2:  Virtual address to map to
     *  3:  Flags to apply
     * */
    void (*mappg)(void*, void*, uint32_t);

    /*  Allocates a free block of physical memory to virtual address[1]
     *  1:  Virtual address to map
     *  2:  User flag
     *  3:  R/W flag
     *  ret:    Requested virtual address if successful
     * */
    void *(*allocpg)(void*, bool, bool);

    /*  Allocates free blocks of physical memory to virtual address [1]
     *  1:  Virtual address to map
     *  2:  Size of memory in bytes
     *  3:  User flag
     *  4:  R/W flag
     *  ret:    Requested virtual address if successful
     * */
    void *(*allocpgs)(void*, uint32_t, bool, bool);

    /*  Removes a block of physical memory from the page mapping
     *  1:  Page to free
     * */
    void (*freepg)(void*);

    /*  Removes several blocks of memory from the page mapping
     *  1:  Beginning page address
     *  2:  Size of region in bytes
     * */
    void (*freepgs)(void*, uint32_t);
} vmm_allocator_t;

extern vmm_allocator_t *virtual_allocator;

typedef struct page_table {
    page_t pages[1024];
} page_table_t;

typedef struct page_directory {
    page_t tablesPhys[1024];
    page_table_t *tables[1024];

    uintptr_t phys;
} page_directory_t;

#endif
