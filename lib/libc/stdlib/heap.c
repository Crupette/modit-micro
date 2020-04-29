#include "micro/paging.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BIN_TAKEN(flg) (flg & 1)
#define BIN_SIZE(flg) (flg & 0xFFFFFFFE)

typedef struct heap_bin {
    struct heap_bin *prev;
    struct heap_bin *next;

    uintptr_t addr;
    uint32_t flags;
} heap_bin_t;

static heap_bin_t *bin_tail = 0;
static heap_bin_t *bin_head = 0;
static heap_bin_t *bin_next = 0;

static heap_bin_t *ehead = 0;
bool pg = false;

static heap_bin_t *find_free_bin(){
    if(ehead != 0){
        heap_bin_t *bin = ehead;
        ehead = bin->next;
        return bin;
    }
    bin_next++;
    if(pg){
        micro_allocpgs(bin_next, sizeof(heap_bin_t), 0x7);
    }else{
        //TODO: SBRK
    }
    memset(bin_next, 0, sizeof(heap_bin_t));
    return bin_next;
}

static heap_bin_t *add_bin(heap_bin_t *parent, uintptr_t addr, size_t size){
    if(parent == 0) return 0;
    if(size == 0) return 0;

    heap_bin_t *newbin = find_free_bin();
    memset(newbin, 0, sizeof(*newbin));

    newbin->next = parent->next;
    newbin->prev = parent;

    if(newbin->next) newbin->next->prev = newbin;
    parent->next = newbin;

    if(parent == bin_tail) bin_tail = newbin;

    newbin->flags = size;
    newbin->addr = addr;

    if(pg){
        micro_allocpgs((void*)addr, size, 0x7);
    }else{
        //TODO: SBRK
    }
    return newbin;
}

static void remove_bin(heap_bin_t *bin){
    if(bin->next != 0) bin->next->prev = bin->prev;
    if(bin->prev != 0) bin->prev->next = bin->next;

    if(bin_head == bin_tail && bin_head == bin) return;

    if(bin_head == bin) bin_head = bin->next;
    if(bin_tail == bin) bin_tail = bin->prev;

    if(ehead != 0){
        heap_bin_t *enext = ehead;
        while(enext->next){
            enext = enext->next;
        }
        enext->next = bin;
        bin->prev = enext;
    }else{
        ehead = bin;
    }
}

static heap_bin_t *append_bin(size_t size){
    uintptr_t nextaddr = (uintptr_t)bin_tail->addr + BIN_SIZE(bin_tail->flags);
    return add_bin(bin_tail, nextaddr, size);
}

static heap_bin_t *merge_bin(heap_bin_t *mergee){
    bool merged = true;
    while(merged){
        merged = false;
        if(mergee->next != 0)
        if(BIN_TAKEN(mergee->flags) != 1 && BIN_TAKEN(mergee->next->flags) != 1){
            mergee->flags += BIN_SIZE(mergee->next->flags);
            if(mergee->next == bin_tail) bin_tail = mergee;
            if(mergee == bin_head) bin_head = mergee->next;
            remove_bin(mergee->next);
            merged = true;
        }
        if(mergee->prev != 0)
        if(BIN_TAKEN(mergee->flags) != 1 && BIN_TAKEN(mergee->prev->flags) != 1){
            mergee->flags += BIN_SIZE(mergee->prev->flags);
            if(mergee == bin_tail) bin_tail = mergee->prev;
            if(mergee->prev == bin_head) bin_head = mergee;
            remove_bin(mergee->prev);
            merged = true;
        }
    }
    return mergee;
}

static void split_bin(heap_bin_t *bin, size_t s){
    if(BIN_SIZE(bin->flags) == s) return;

    size_t sdif = BIN_SIZE(bin->flags) - s;
    uintptr_t naddr = (uintptr_t)bin->addr + s;

    add_bin(bin, naddr, sdif);
    bin->flags = s;
}

static heap_bin_t *find_best_fit(size_t s){
    heap_bin_t *best = 0;
    for(heap_bin_t *bin = bin_head; bin; bin = bin->next){
        if((bin->flags & 1) != 0) continue;
        if(BIN_SIZE(bin->flags) < s) continue;
        if(best == 0){
            best = bin;
            continue;
        }
        int32_t sdif = BIN_SIZE(bin->flags) - s;
        if(BIN_SIZE(best->flags) - s > sdif){
            best = bin;
        }
    }
    return best;
}

void *malloc(size_t size){
    if(size == 0) return 0;
    size = ((size + 1) / 2) * 2;

    heap_bin_t *best = find_best_fit(size);
    if(best == 0){
        if(bin_tail->flags & 1){
            best = append_bin(size);
        }else{
            best = bin_tail;
            best->flags = size;
            if(pg){
                micro_allocpgs((void*)best->addr, size + 1, 0x7);
            }else{
                //TODO: SBRK
            }
        }
    }
    split_bin(best, size);

    best->flags |= 1;
    return (void*)best->addr;
}

void *realloc(void *ptr, size_t size){
    if(size == 0 && ptr != 0){
        free(ptr);
        return 0;
    }
    size = ((size + 1) / 2) * 2;

    heap_bin_t *bin;
    for(heap_bin_t *bin = bin_head; bin && bin->addr != (uintptr_t)ptr; bin = bin->next) {}
    if(bin == 0) return 0;

    if(size <= BIN_SIZE(bin->flags)) return ptr;
    void *new = malloc(size);
    memcpy(new, ptr, BIN_SIZE(bin->flags));

    free(ptr);

    return new;
}

void free(void *mem){
    for(heap_bin_t *bin = bin_head; bin; bin = bin->next){
        if(bin->addr == (uintptr_t)mem){
            bin->flags = BIN_SIZE(bin->flags);
            bin = merge_bin(bin);
            return;
        }
    }
}

#define PG_BIN_START 0x80000000
#define PG_HEAP_START 0x81000000

void __heap_setup(bool paging){
    ehead = 0;

    if(paging){
        bin_head = bin_tail = bin_next = (heap_bin_t*)(PG_BIN_START);
        micro_allocpgs(bin_head, sizeof(heap_bin_t), 0x7);
        micro_allocpgs((void*)PG_HEAP_START, 0x1000, 0x7);

        memset(bin_head, 0, sizeof(heap_bin_t));
        bin_head->addr = PG_HEAP_START;
        bin_head->flags = 0x1000;
    }else{
       //TODO: SBRK 
    }
    pg = paging;
}
