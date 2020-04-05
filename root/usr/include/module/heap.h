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
            bool taken       : 1;
            uint32_t size   : 31;
        };
    };

    struct bin_header *prev;
    struct bin_header *next;
} bin_header_t;

void *kalloc_a(size_t s, uint32_t a);

void *kalloc(size_t s);
void kfree(void *p);

#endif
