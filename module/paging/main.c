#include "module/interrupt.h"

#include "kernel/modloader.h"
#include "kernel/memory.h"
#include "kernel/print.h"
#include "kernel/io.h"

void invlpg(void *addr){
    asm volatile("invlpg (%0)":: "r"(addr) : "memory");
}

void invldir(){
    asm volatile("movl %%cr3, %0":: "r"(
                virtual_allocator->currentDirectory->phys));
}

page_t remappg(void* phys, void *virt, uint32_t flags){
    uint32_t dindex = (uint32_t)virt >> 22;
    uint32_t tindex = ((uint32_t)virt >> 12) & 0x3FF;

    page_directory_t *dir = virtual_allocator->currentDirectory;
    page_table_t *tbl = dir->tables[dindex];

    if(dir->tablesPhys[dindex].present == 0){
        uintptr_t *tbl_phys = physical_allocator->getpg();

        dir->tablesPhys[dindex].entry = (uintptr_t)tbl_phys | flags;
        dir->tables[dindex] = (page_table_t*)((dindex * 0x1000) + 0xFFC00000);
        tbl = dir->tables[dindex];

        memset(tbl, 0, sizeof(tbl));
    }
    page_t ret = tbl->pages[tindex];
    tbl->pages[tindex].entry = ((uintptr_t)phys) | (flags & 0xFFF);
    invlpg(virt);
    return ret;
}

void mappg(void *phys, void *virt, uint32_t flags){
    uint32_t dindex = (uint32_t)virt >> 22;
    uint32_t tindex = ((uint32_t)virt >> 12) & 0x3FF;

    page_directory_t *dir = virtual_allocator->currentDirectory;
    page_table_t *tbl = dir->tables[dindex];   

    if(dir->tablesPhys[dindex].present == 0){
        remappg(phys, virt, flags);
        return;
    }
    if(tbl->pages[tindex].entry == 0){
        remappg(phys, virt, flags);
        return;
    }
}

void *allocpg(void *req, uint32_t flags){
    void *block = physical_allocator->getpg();
    if(block == 0){
        vga_printf("[ERR]: Physical allocator failed to retrieve a free page\n");
        return 0;
    }

    page_t prev = virtual_allocator->remappg(block, req, flags);
    if(prev.entry != 0){
        void *oblock = (void*)((uintptr_t)prev.addr * 0x1000);
        physical_allocator->freepg(oblock);
    }
    return req;
}

void *allocpgs(void *req, uint32_t size, uint32_t flags){
    uintptr_t pgsize = (size + 0xFFF) / 0x1000;

    void *oreq = req;
    for(uint32_t i = 0; i < pgsize; i++){
        void *ret = virtual_allocator->allocpg(req, flags);
        if(ret != req){
            vga_printf("[ERR]: Chunk allocation failed!\n");
            return 0;
        }
        req = (void*)((uintptr_t)req + 0x1000);
    }
    return oreq;
}

void freepg(void *req){
    page_t ret = virtual_allocator->remappg(0, req, 0);
    if(ret.entry == 0){
        void *blk = (void*)((uintptr_t)ret.addr * 0x1000);
        physical_allocator->freepg(blk);
    }
}

void freepgs(void *req, uint32_t size){
    uintptr_t pgsize = (size + 0xFFF) / 0x1000;

    for(uint32_t i = 0; i < pgsize; i++){
        virtual_allocator->freepg(req);
        req = (void*)((uintptr_t)req + 0x1000);
    }
}

page_directory_t *swpdir(page_directory_t *dir){
    page_directory_t *orig = virtual_allocator->currentDirectory;
    virtual_allocator->currentDirectory = dir;
    virtual_allocator->invldir();
    return orig;
}

void pfHandler(interrupt_state_t *r){
    uint32_t faddr;
    asm volatile("mov %%cr2, %0": "=r"(faddr));

    vga_printf("Page faulr : %i @ 0x%x\n", r->err, faddr);
    asm volatile("hlt");
}

int _init(){
    virtual_allocator->invlpg = invlpg;
    virtual_allocator->invldir = invldir;
    virtual_allocator->remappg = remappg;
    virtual_allocator->mappg = mappg;
    
    virtual_allocator->allocpg =  allocpg;
    virtual_allocator->allocpgs = allocpgs;
 
    virtual_allocator->freepg =  freepg;
    virtual_allocator->freepgs = freepgs;

    virtual_allocator->swpdir = swpdir;
    
    idt_addHandler(14, pfHandler); 

    vga_printf("[OK]: Setup second-stage pager\n");
    return 0;
}

int _fini(){
    return 0;
}

module_name(pager);

module_load(_init);
module_unload(_fini);

module_depends(interrupt);
