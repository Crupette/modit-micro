#include "module/interrupt.h"
#include "module/heap.h"

#include "kernel/modloader.h"
#include "kernel/memory.h"
#include "kernel/print.h"
#include "kernel/io.h"

static bin_header_t *bin_tail;
static bin_header_t *bin_head;
static bin_header_t *bin_hiaddr;

static bin_header_t *empty_head;

extern uintptr_t krnl_next_free_pg;

static bin_header_t *find_free_bin(){
    if(empty_head != 0){
        bin_header_t *bin = empty_head;
        empty_head = bin->next;
        return bin;
    }
    bin_hiaddr = bin_hiaddr + 1;
    virtual_allocator->allocpg(bin_hiaddr, 0x3);
    return bin_hiaddr;
}

static bin_header_t *add_bin(bin_header_t *parent, void *addr, size_t size){
    if(parent == 0){    //Can't have a floating bin
        vga_printf("[ERR]: Attempted to create heap bin with no parent\n");
        return 0;
    }
    if((uintptr_t)addr < BIN_START){    //Bin addr cannot be out of bounds
        vga_printf("[ERR]: Bin allocator requested bin outside of heap range (%i)\n", addr);
        return 0;
    }
    bin_header_t *newbin = find_free_bin();

    if(parent->next != 0){
        parent->next->prev = newbin;
        newbin->next = parent->next;
    }else{  //Newbin is now the tail of the linked list
        bin_tail = newbin;
    }

    parent->next = newbin;
    newbin->prev = parent;

    newbin->addr = addr;
    newbin->size = size;

    virtual_allocator->allocpgs(addr, size, 0x3);
    return newbin;
}

static void remove_bin(bin_header_t *bin){
    if(bin->next != 0) bin->next->prev = bin->prev;
    if(bin->prev != 0) bin->prev->next = bin->next;

    if(bin_head == bin) bin_head = bin->next;
    if(bin_tail == bin) bin_tail = bin->prev;

    bin->next = NULL;
    bin->prev = NULL;

    int i = 0;
    if(empty_head != 0){
        bin_header_t *empty_next = empty_head;
        while(empty_next->next){
            empty_next = empty_next->next;
            i++;
        }
        empty_next->next = bin;
        bin->prev = empty_next;
    }else{
        empty_head = bin;
    }
}

static bin_header_t *append_bin(size_t size){
    void *nextaddr = (void*)((uintptr_t)bin_tail->addr + bin_tail->size);
    return add_bin(bin_tail, nextaddr, size);
}

static bin_header_t *merge_bin(bin_header_t *mergee){
    bool merged = true;
    while(merged){
        merged = false;
        if(mergee->next != 0)
        if(mergee->taken != 1 && mergee->next->taken != 1){
            mergee->size += mergee->next->size;
            remove_bin(mergee->next);
            merged = true;
        }
        
        if(mergee->prev != 0)
        if(mergee->taken != 1 && mergee->prev->taken != 1){
            mergee->size += mergee->prev->size;
            mergee->addr = mergee->prev->addr;

            remove_bin(mergee->prev);
            merged = true;
        }
    }

    return mergee;
}

static void split_bin(bin_header_t *bin, size_t s){
    if(bin->size == s) return;
    
    size_t sdif = bin->size - s;
    void *naddr = (void*)((uintptr_t)bin->addr + s);

    add_bin(bin, naddr, sdif);
    bin->size = s;
}

static bin_header_t *find_best_fit(size_t s){
    bin_header_t *best = 0;
    for(bin_header_t *bin = bin_head; bin; bin = bin->next){
        if(bin->taken != 0) continue;
        if(best == 0){
            best = bin;
            continue;
        }
        size_t sdif = bin->size - s;
        if(sdif >= 0 && best->size - s > sdif){
            best = bin;
        }
    }
    return best;
}

void *kalloc(size_t s){
    if(s == 0) return 0;
    size_t rsize = ((s + 15) / 16) * 16;

    bin_header_t *bestbin = find_best_fit(rsize);
    if(bestbin == 0){
        bestbin = append_bin(rsize);
    }

    split_bin(bestbin, rsize);

    bestbin->taken = 1;
    return bestbin->addr;
}

void kfree(void *p){
    for(bin_header_t *bin = bin_head; bin->next; bin = bin->next){
        if(bin->addr == p){
            bin->taken = 0;
            merge_bin(bin);
            return;
        }
    }
}

int _init(){
    //Setup the first heap bin
    empty_head = 0;
    bin_head = (bin_header_t*)(krnl_next_free_pg);
    bin_tail = bin_head;
    bin_hiaddr = bin_head;

    virtual_allocator->allocpg(bin_head, 0x3);
    virtual_allocator->allocpg((void*)BIN_START, 0x3);

    memset(bin_head, 0, sizeof(bin_header_t));
    bin_head->addr = (void*)BIN_START;
    bin_head->size = 0x1000;

    return 0;
}

int _fini(){
    return 0;
}

module_name(heap);

module_load(_init);
module_unload(_fini);

module_depends(paging);
