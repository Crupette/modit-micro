#include "kernel/memory.h"
#include "kernel/io.h"
#include "kernel/print.h"

extern multiboot_info_t *mbinfo;

static uint32_t bitmap[BM_SIZE];

static uint32_t pages_taken;
static uint32_t pages_free;
static uint32_t last_free;

static uintptr_t map_start;
static uintptr_t map_end;

//Physical descriptors for default memory allocators
pmm_allocator_t k_physical_allocator = { 0 };
vmm_allocator_t k_virtual_allocator = { 0 };

//Replaceable pointers to memory allocators
//  Replace these in modules to change the kernel's allocation scheme
pmm_allocator_t *physical_allocator = 0;
vmm_allocator_t *virtual_allocator = 0;

#define BM_TEST(i)  (bitmap[i / 32] & (1 << (i % 32)))  //Returns if bit in bm is set
#define BM_SET(i)   (bitmap[i / 32] |= (1 << (i % 32))) //Sets bit in bm index i to 1
#define BM_UNSET(i) (bitmap[i / 32] &= ~(1 << (i % 32)))    //Sets bit in bm index i to 0

/*  Gets a free value in the bm starting at 0
 * */
static uint32_t bm_seek_free(){
    for(size_t i = 0; i < BM_SIZE; i++){
        //This section of bitmap is full
        if(bitmap[i] == 0xFFFFFFFF) continue;
        for(size_t j = 0; j < 32; j++){
            //Bitmap has 32-bit granularity
            uint32_t bit = 1 << j;
            if(!(bitmap[i] & bit)) return i * 32 + j;
        }
    }
    return BM_SIZE;
}

/*  Gets a free value faster than seek_free assuming last_free is valid
 * */
static uint32_t bm_get_free(){
    if(BM_TEST(last_free)){
        last_free = bm_seek_free();
    }
    if(last_free >= BM_SIZE){
        vga_printf("[ERR]: Kernel pmm ran out of memory!\n");
    }
    return last_free++;
}

typedef struct multiboot_memory_map {
    uint32_t size;
    uint32_t base_low, base_high, len_low, len_high;
    uint32_t type;
}mmap_ent_t;

static void kpmm_addblock(void *start, uint16_t n){
    uintptr_t absstart;

    //Start of block cannot come before map_start, 
    //so must be adjusted
    if((uintptr_t)start < map_start){
        uintptr_t diff = map_start - (uintptr_t)start;
        start = (void*)((uintptr_t)start + diff);
        n -= diff / PAGE_SIZE;
    }
    //Start of block cannot come after map_end, 
    //so it's invalidated
    if((uintptr_t)start > map_end) return;
    absstart = ((uintptr_t)start - map_start) / PAGE_SIZE;
    
    if((uintptr_t)start + ((uintptr_t)n * PAGE_SIZE) > map_end){    //Size of block cannot exceed (map_end - start), 
   //so must be adjusted
        n = BM_SIZE - absstart;
    }

    for(int i = absstart; i < n * 16; i++){
        BM_UNSET(i);
        pages_free++;
        pages_taken--;
    }
}

static void kpmm_rmblock(void *start, uint16_t n){
    uintptr_t absstart;

    if((uintptr_t)start < map_start){ //Start of block cannot come before map_start, so must be adjusted
        uintptr_t diff = map_start - (uintptr_t)start;
        start = (void*)((uintptr_t)start + diff);
        n -= diff / PAGE_SIZE;
    }

    //Start of block cannot come after map_end, so it's invalidated
    if((uintptr_t)start > map_end) return;  

    absstart = ((uintptr_t)start - map_start) / PAGE_SIZE;

    //Size of block cannot exceed (map_end - start), so must be adjusted
    if((uintptr_t)start + ((uintptr_t)n * PAGE_SIZE) > map_end){
        n = BM_SIZE - absstart;
    }

    for(int i = absstart; i < n * 16; i++){
        BM_SET(i);
        pages_free--;
        pages_taken++;
    }
}

static void *kpmm_getpg(){
    uintptr_t free = bm_get_free();
    if(free >= BM_SIZE)
        return 0;   //Error is already reported, so no work needed
    
    BM_SET(free);
    pages_free--;
    pages_taken++;

    return (void*)((free * PAGE_SIZE) + map_start);
}

static void kpmm_freepg(void *ptr){
    if(ptr == 0) return;
    uintptr_t freepg = ((uintptr_t)ptr - map_start) / PAGE_SIZE;
    BM_UNSET(freepg);

    pages_free++;
    pages_taken--;
}

static void kpmm_resvpg(void *ptr){
    if(ptr == 0) return;
    uintptr_t resvpg = ((uintptr_t)ptr - map_start) / PAGE_SIZE;
    BM_SET(resvpg);

    pages_free--;
    pages_taken++;
}

uint32_t *kcore_getbm(uintptr_t *start, uintptr_t *end){
    *start = map_start;
    *end = map_end;

    return bitmap;
}

page_directory_t *kernel_dir = 0;
page_table_t *kernel_tbl = 0;

uintptr_t krnl_next_free_pg = 0xE0400000;


static void kvmm_mappg(void *phys, void *virt, uint32_t flags){
    uint32_t dindex = ((uint32_t)virt) >> 22;
    uint32_t tindex = ((uint32_t)virt) >> 12 & 0x03FF;

    //Kernel directory is recursively mapped, so info can be found at 0xFF000000
    page_directory_t *dir = kernel_dir;
    page_table_t *table = dir->tables[dindex];

    if(dir->tablesPhys[dindex].present == 0){
        //Table is not present, so it needs to be initialized
        uintptr_t *tablePhys = kpmm_getpg();

        dir->tablesPhys[dindex].entry = (uintptr_t)tablePhys | 3;
        dir->tables[dindex] = (page_table_t*)((dindex * 0x1000) + 0xFFC00000);
        table = dir->tables[dindex];

        memset(table, 0, sizeof(*table));
    }

    if(table->pages[tindex].entry != 0) return;
    table->pages[tindex].entry = ((uintptr_t)phys) | (flags & 0xFFF);
    asm volatile("invlpg (%0)":: "r"(virt) : "memory"); //Pages need to be invalidated to refresh the tlb
}

static void *kvmm_allocpg(void *req, bool usr, bool rw){
    void *block = physical_allocator->getpg();
    if(block == 0) return 0;

    kvmm_mappg(block, req, 0x1 | (usr << 2) | (rw << 1));
    return req;
}

static void kvmm_freepg(void *req){
    kvmm_mappg(0, req, 0);
}

extern void _krnl_enablepg(uintptr_t dir);
static void vmm_init(){

    k_virtual_allocator.mappg = kvmm_mappg;
    k_virtual_allocator.allocpg = kvmm_allocpg;
    k_virtual_allocator.freepg = kvmm_freepg;

    virtual_allocator = &k_virtual_allocator;
    
    //Kernel dir needs to be allocated dynamically (thanks GCC)
    //It needs > 2 pages, so 4092 bytes are wasted
    kernel_dir = (page_directory_t*)((uintptr_t)physical_allocator->getpg() + VIRT_BASE);
    physical_allocator->getpg();
    physical_allocator->getpg();
    kernel_tbl = (page_table_t*)((uintptr_t)physical_allocator->getpg() + VIRT_BASE);

    memset(kernel_dir, 0, sizeof(kernel_dir));
    memset(kernel_tbl, 0, sizeof(kernel_tbl));
    
    //Identity maps the last page directory
    kernel_dir->tables[1023] = (page_table_t*)kernel_dir;
    kernel_dir->tablesPhys[1023].entry = (((uintptr_t)kernel_dir) - VIRT_BASE) | 0x03;

    kernel_dir->phys = ((uintptr_t)kernel_dir) - VIRT_BASE;

    //Maps 0x0 -> 0xE0000000
    uint32_t tindex = VIRT_BASE >> 22;
    kernel_dir->tables[tindex] = kernel_tbl;
    kernel_dir->tables[0] = kernel_tbl;

    kernel_dir->tablesPhys[tindex].entry = (((uintptr_t)kernel_tbl) - VIRT_BASE) | 0x03;
    kernel_dir->tablesPhys[0].entry = (((uintptr_t)kernel_tbl) - VIRT_BASE) | 0x03;

    for(size_t i = 0; i < 0x400; i++){
        kernel_tbl->pages[i].entry = (i * 0x1000) | 0x03;
    }

    _krnl_enablepg(kernel_dir->phys);
    vga_printf("[OK]: VMM Initialized\n");
}

static void pmm_init(){
    pages_taken = BM_SIZE * 32;
    pages_free = 0;
    last_free = 0;
    map_start = 0;
    map_end = 0;

    memset(bitmap, 0xFF, BM_SIZE / 4);

    k_physical_allocator.addblock = kpmm_addblock;
    k_physical_allocator.rmblock = kpmm_rmblock;
    k_physical_allocator.getpg = kpmm_getpg;
    k_physical_allocator.freepg = kpmm_freepg;
    k_physical_allocator.resvpg = kpmm_resvpg;

    physical_allocator = &k_physical_allocator;

    size_t memsz = mbinfo->mem_upper;
    vga_printf("System has %iMB of usable ram\n", memsz / 1024);

    map_start = 0;
    map_end = BM_SIZE * 4096 * 32;
    //GRUB does not know we are higher half
    mmap_ent_t *entry = (mmap_ent_t*)(mbinfo->mmap_addr + VIRT_BASE); 
    while((uintptr_t)entry < mbinfo->mmap_addr + mbinfo->mmap_length + VIRT_BASE){
        if(entry->type == 1){
            uintptr_t entry_map_index = (entry->base_low / 4096) / 8;
            uintptr_t entry_map_len = (entry->len_low / 4096) / 8;
    
            if(entry_map_index > BM_SIZE * 4) continue;
            if(entry_map_index + entry_map_len > BM_SIZE * 4){
                entry_map_len = (BM_SIZE * 4) - entry_map_index;
            }
                
            memset((void*)((uintptr_t)bitmap + entry_map_index), 0, entry_map_len);
        }
        entry = (mmap_ent_t*)((uint32_t)entry + entry->size + sizeof(entry->size));
    }

    for(int i = 0; i < BM_SIZE * 32; i++){
        if(BM_TEST(i) == 0){
            pages_free++;
            pages_taken--;
        }
    }

    extern uintptr_t _begin;
    extern uintptr_t _end;

    //Kernel pages need to be kept safe
    for(uintptr_t i = (uintptr_t)(&_begin); i < (uintptr_t)(&_end) + 0x20000; i += 4096){
        physical_allocator->resvpg((void*)(i - VIRT_BASE));
    }
    (void)physical_allocator->getpg();

    vga_printf("[OK]: PMM Initialized with %i pages free, %i reserved\n", pages_free, pages_taken);
}

void kmem_init(){
    pmm_init();
    vmm_init();
}
