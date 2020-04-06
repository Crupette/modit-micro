/*  HEAP.H -    A simple bin allocator for dynamic memory allocation
 *              bin headers are stored in a linked list starting at krnl_next_free_pg
 *              actual memory is stored at 0xE1000000 and up
 *  
 *  Author: Crupette
 * */

#ifndef MODULE_HEAP_H
#define MODULE_HEAP_H 1

#define BIN_START   0xE1000000

#include "kernel/types.h"
#include <stddef.h>
#include <stdbool.h>

typedef struct bin_header {
    void *addr;
    union {
        uint32_t flags;
        struct {
            bool taken       : 1;   //The block is occupied / unfreed
            uint32_t size   : 31;   //Size of the block
        };
    };

    struct bin_header *prev;    //Previous entry in the linked list
    struct bin_header *next;    //Next entry in the linked list
} bin_header_t;

/*  Allocates a memory aligned block of memory with alignment a and size s
 *  s:  Size of memory to allocate
 *  a:  Alignment of memory
 *  r:  Pointer to allocated memory
 * */
void *kalloc_a(size_t s, uint32_t a);

/*  Allocates a block of memory with size s:
 *  s:  Size of memory to allocate
 *  r:  Pointer to memory allocated
 * */
void *kalloc(size_t s);

/*  Frees a block of memory at address p
 *  p:  Pointer to memory to be freed
 * */
void kfree(void *p);

#endif
