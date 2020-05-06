#include "module/heap.h"
#include "module/interrupt.h"

#include "kernel/modloader.h"
#include "kernel/memory.h"
#include "kernel/logging.h"
#include "kernel/io.h"
#include "kernel/lock.h"

DECLARE_LOCK(page_lock) = 0;
extern void __panic(interrupt_state_t *r, char *fmt, ...);

void invlpg(void *addr){
    asm volatile("invlpg (%0)":: "r"(addr) : "memory");
}

void invldir(){
    asm volatile("movl %0, %%cr3":: "r"(
                virtual_allocator->currentDirectory->phys));
}

page_t remappg(void* phys, void *virt, uint32_t flags){
    uint16_t dindex = (uint32_t)virt >> 22;
    uint16_t tindex = ((uint32_t)virt >> 12) & 0x3FF;

    LOCK(page_lock);

    page_directory_t *dir = virtual_allocator->currentDirectory;
    page_table_t *tbl = dir->tables[dindex];

    if(dir->tablesPhys[dindex].present == 0){
        uintptr_t *tbl_phys = physical_allocator->getpg();

        dir->tablesPhys[dindex].entry = (uintptr_t)tbl_phys | flags;
        
        dir->tables[dindex] = (page_table_t*)((dindex * 0x1000) + 0xFFC00000);
        tbl = dir->tables[dindex];
        
        virtual_allocator->invldir();

        memset(tbl, 0, sizeof(*tbl));
    }
    page_t ret = tbl->pages[tindex];
    tbl->pages[tindex].entry = ((uintptr_t)phys) | (flags & 0xFFF);
    invlpg(virt);

    UNLOCK(page_lock);
    return ret;
}

page_t mappg(void *phys, void *virt, uint32_t flags){
    uint16_t dindex = (uint32_t)virt >> 22;
    uint16_t tindex = ((uint32_t)virt >> 12) & 0x3FF;

    page_directory_t *dir = virtual_allocator->currentDirectory;
    page_table_t *tbl = dir->tables[dindex];   

    if(dir->tablesPhys[dindex].present == 0){
        return remappg(phys, virt, flags);
    }
    if(tbl->pages[tindex].entry == 0){
        return remappg(phys, virt, flags);
    }
    return tbl->pages[tindex];
}

void *allocpg(void *req, uint32_t flags){
    void *block = physical_allocator->getpg();
    if(block == 0){
        log_printf(LOG_WARNING, "Physical allocator failed to retrieve a free page\n");
        return 0;
    }

    page_t prev = virtual_allocator->mappg(block, req, flags);
    if(prev.entry != 0){
        physical_allocator->freepg(block);
    }
    return req;
}

void *allocpgs(void *req, uint32_t size, uint32_t flags){
    uintptr_t pgsize = (size + 0xFFF) / 0x1000;

    void *oreq = req;
    for(uint32_t i = 0; i < pgsize; i++){
        void *ret = virtual_allocator->allocpg(req, flags);
        if(ret != req){
            log_printf(LOG_WARNING, "Chunk allocation failed!\n");
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
    LOCK(page_lock);
    page_directory_t *orig = virtual_allocator->currentDirectory;
    virtual_allocator->currentDirectory = dir;
    virtual_allocator->invldir();
    UNLOCK(page_lock);
    return orig;
}

void clonetbl(page_directory_t *parent, page_table_t *tbl, uint16_t index){
    uint32_t paddr = parent->tablesPhys[index].entry & 0xFFFFF000;

    page_table_t *newtbl = kalloc_a(0x1000, 0x1000);
    void *pgbuffer = kalloc_a(0x1000, 0x1000);

    //Newtbl is a temporary allocation, which is remapped to physical address of new dir
    page_t oldmaptbl = virtual_allocator->remappg((void*)paddr, newtbl, 0x3);

    uint32_t cloneaddr = (uint32_t)index << 22;
    for(uint32_t i = 0; i < 1024; i++){
        if(tbl->pages[i].entry == 0) {
            cloneaddr += 0x1000;
            continue;
        }
        page_t oldmappg = virtual_allocator->remappg(
                physical_allocator->getpg(), pgbuffer,
                tbl->pages[i].entry & 0xFFF);

        memcpy((void*)pgbuffer, (void*)cloneaddr, 0x1000);

        oldmappg = virtual_allocator->remappg((void*)(oldmappg.entry & 0xFFFFF000),
                pgbuffer, oldmappg.entry & 0xFFF);

        newtbl->pages[i] = oldmappg;
        cloneaddr += 0x1000;
    }

    //Restore table address to original mapping
    virtual_allocator->remappg(
            (void*)(oldmaptbl.entry & 0xFFFFF000), newtbl, oldmaptbl.entry & 0xFFF);

    kfree(newtbl);
    kfree(pgbuffer);
}

page_directory_t *clonedir(page_directory_t *dir){
    page_directory_t *new = kalloc_a(sizeof(page_directory_t), 0x1000);

    memset(new, 0, sizeof(page_directory_t));

    //Map kernel page table entries
    for(unsigned int i = 864; i < 1023; i++){
        new->tables[i] = dir->tables[i];
        new->tablesPhys[i] = dir->tablesPhys[i];
    }

    //Copy rest of tables
    for(unsigned int i = 0; i < 864; i++){
        if(dir->tables[i] == 0) continue;
        new->tablesPhys[i].entry = (uintptr_t)physical_allocator->getpg() 
            | (dir->tablesPhys[i].entry & 0xFFF);
        new->tables[i] = (page_table_t*)((i * 0x1000) + 0xFFC00000);
        clonetbl(new, dir->tables[i], i);
    }

    new->tablesPhys[1023].entry = virtual_allocator->getphys((uintptr_t)new) | 0x3;
    new->tables[1023] = (page_table_t*)new;
    new->phys = virtual_allocator->getphys((uintptr_t)new);
    return new;
}

char *fault_possibles[] = {
    "non-present",  //r->err & 0
    "present",
    "read from",    //((r->err & 2) >> 1) + 2
    "wrote to",
    "kernel",       //((r->err & 4) >> 2) + 2
    "user"
};

void pfHandler(interrupt_state_t *r){
    uint32_t faddr;
    asm volatile("mov %%cr2, %0": "=r"(faddr));

    __panic(r, "Page fault : %s %s %s page @ %x\n",
            fault_possibles[((r->err & 0x4) >> 2) + 4],
            fault_possibles[((r->err & 0x2) >> 1) + 2],
            fault_possibles[((r->err & 0x1) >> 0) + 0],
            faddr);
}

int paging_init(){

    virtual_allocator->invlpg = invlpg;
    virtual_allocator->invldir = invldir;
    virtual_allocator->remappg = remappg;
    virtual_allocator->mappg = mappg;
    
    virtual_allocator->allocpg =  allocpg;
    virtual_allocator->allocpgs = allocpgs;
 
    virtual_allocator->freepg =  freepg;
    virtual_allocator->freepgs = freepgs;

    virtual_allocator->swpdir = swpdir;
    virtual_allocator->clonetbl = clonetbl;
    virtual_allocator->clonedir = clonedir;
    
    isr_addHandler(14, pfHandler); 

    virtual_allocator->currentDirectory->tables[0] = 0;
    virtual_allocator->currentDirectory->tablesPhys[0].entry = 0;

    log_printf(LOG_OK, "Setup second-stage pager\n");
    return 0;
}

int paging_fini(){
    return 0;
}

module_name(pager);

module_load(paging_init);
module_unload(paging_fini);

module_depends(panic);
module_depends(idt);
module_depends(isr);
module_depends(heap);
