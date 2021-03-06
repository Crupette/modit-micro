#include "kernel/modloader.h"
#include "kernel/memory.h"
#include "kernel/logging.h"
#include "kernel/io.h"

extern uintptr_t krnl_next_free_pg;

uint32_t *bitmap = 0;
uintptr_t mapsize = 0;
uintptr_t pages_free = 0;
uintptr_t pages_taken = 0;
uintptr_t pages_total = 0;

uint32_t last_free = 0;

#define BM_TEST(i)  (bitmap[i / 32] & (1 << (i % 32)))  //Tests if bit in bm is set, returns 1 if so
#define BM_SET(i)   (bitmap[i / 32] |= (1 << (i % 32))) //Sets bit in bm index i to 1
#define BM_UNSET(i) (bitmap[i / 32] &= ~(1 << (i % 32)))    //Sets bit in bm index i to 0

static int32_t bm_seek_free(void) {
    for(uint32_t i = 0; i < mapsize / 32; i++){
        if(bitmap[i] == 0xFFFFFFFF) continue;   //Section is full
        for(int j = 0; j < 32; j++){
            uint32_t bit = 1 << j;
            if(!(bitmap[i] & bit)) return i * 32 + j;
        }   
    }
    return -1;
}

static int32_t bm_get_free(void){
    if(BM_TEST(last_free) == 0){
        last_free++;
        return last_free - 1;
    }else{
        last_free = bm_seek_free();
        last_free++;
        return last_free - 1;
    }
}

static void *bm_getpg(void){
    if(pages_taken >= pages_total){
        log_printf(LOG_WARNING, "Bitmap is full!\n");
        return 0;
    }
    int free = bm_get_free();
    if(free == -1) return 0;

    BM_SET(free);
    pages_free--;
    pages_taken++;

    uintptr_t freeptr = (free * 4096);
    return (void*)freeptr;
}

static void bm_resvpg(void *ptr){
    uint32_t page = (uintptr_t)ptr / 4096;
    
    if(BM_TEST(page) == 0){
        pages_free--;
        pages_taken++;
    }
    BM_SET(page);
}

static void bm_freepg(void *ptr){
    uint32_t page = (uintptr_t)ptr / 4096;
    
    BM_UNSET(page);
    pages_free++;
    pages_taken--;
}

static void bm_addblock(void *start, uint16_t len){
    uint16_t pindex = (uintptr_t)start / 4096;
    for(uint16_t i = 0; i < len; i++){
        if(BM_TEST(pindex) == 0){
            pages_free--;
            pages_taken++;
        }
        BM_SET(pindex);
        pindex++;
    }
}

static void bm_rmblock(void *start, uint16_t len){
    uint16_t pindex = (uintptr_t)start / 4096;
    for(uint16_t i = 0; i < len; i++){
        if(BM_TEST(pindex)){
            pages_free++;
            pages_taken--;
        }
        BM_UNSET(pindex);
        pindex++;
    }
}

typedef struct multiboot_memory_map {
    uint32_t size;
    uint32_t base_low, base_high, len_low, len_high;
    uint32_t type;
}mmap_ent_t;

void _pmm_init(void){
    uintptr_t old_map_start = 0;
    uintptr_t old_map_end = 0;
    uint32_t *old_bitmap = kcore_getbm(&old_map_start, &old_map_end);

    bitmap = (uint32_t*)krnl_next_free_pg;
    //Allocates the bitmap  
    
    uintptr_t map_size = (mbinfo.mem_lower + mbinfo.mem_upper) * 1024;  //Memory size in KiB translated to bytes
    mapsize = map_size;
    uintptr_t map_pages = ((map_size / 4096) / 4096) / 8;

    for(uintptr_t i = 0; i < map_pages + 1; i++){
        virtual_allocator->allocpg((void*)krnl_next_free_pg, 0x3);
        krnl_next_free_pg += 4096;
    }
    krnl_next_free_pg += 4096;

    //Map free memory regions on bitmap
    for(uint32_t i = 0; i < mbinfo.mmap.count; i++){
        multiboot_mmap_entry_t *entry = &mbinfo.mmap.entries[i];
       
        if(entry->type != 1) continue;
        uintptr_t entry_map_index = (entry->addr / 4096) / 8;
        uintptr_t entry_map_len = (entry->len / 4096) / 8;

        if(entry_map_index > (map_size / 4096) / 8) continue;
        if(entry_map_index + entry_map_len >
                (map_size / 4096) / 8){
            entry_map_len = ((map_size / 4096) / 8) - entry_map_index;
        }
        
        memset((void*)((uintptr_t)bitmap + entry_map_index), 0, entry_map_len);
    }
    pages_total = map_size / 4096;

    memcpy(bitmap, old_bitmap, (old_map_end / 4096) / 4);

    for(uint32_t i = 0; i < pages_total; i++){
        if(BM_TEST(i)){
            pages_taken++;
        }else{
            pages_free++;
        }
    }

    physical_allocator->addblock = bm_addblock;
    physical_allocator->rmblock = bm_rmblock;
    physical_allocator->getpg = bm_getpg;
    physical_allocator->freepg = bm_freepg;
    physical_allocator->resvpg = bm_resvpg;

    log_printf(LOG_OK, "Bitmap PMM Setup: %i free, %i taken, %i total\n",
            pages_free, pages_taken, pages_total);
}

int _init(){
    _pmm_init();
    return 0;
}

int bmpmm_fini(){
    return 0;
}

module_name(pmm);

module_load(_init);
module_unload(bmpmm_fini);

module_depends(isr);
