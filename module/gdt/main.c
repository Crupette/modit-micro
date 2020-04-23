#include "module/gdt.h"

#include "kernel/modloader.h"
#include "kernel/logging.h"
#include "kernel/io.h"

enum {
    GDT_NULL_SEG,
    GDT_CODE_SEG,
    GDT_DATA_SEG,
    GDT_USR_CODE_SEG,
    GDT_USR_DATA_SEG,
    GDT_TSS_SEG,        //TODO: Add TSS
    GDT_NUM_ENTRIES
};

//SHN_COMMON is a pain, just allocate your data as part of BSS
gdt_entry_t _gdts[GDT_NUM_ENTRIES] = {
    0
};
gdt_descriptor_t _gdtd = {
    .addr = 0, .size = 0
};

tss_entry_t tss_entry = { 0 };

void add_gdt(uint8_t i, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags){
    gdt_entry_t *entry = &_gdts[i];
    
    entry->base_low = base & 0xFFFF;
    entry->base_mid = (base >> 16) & 0xFF;
    entry->base_high = (base >> 24) & 0xFF;

    entry->limit_low = limit & 0xFFFF;
    entry->limit_high = (limit >> 16) & 0xF;

    entry->access = access;
    entry->flags = (limit >> 16) & 0xF;
    entry->flags |= (flags & 0xF0);
}

void add_tss(uint8_t i){
    uint32_t base = (uint32_t)&tss_entry;
    uint32_t limit = base + sizeof(tss_entry_t);

    add_gdt(i, base, limit, 0xE9, 0x00);

    memset(&tss_entry, 0, sizeof(tss_entry));

    tss_entry.base.cs = 0x0b;
    tss_entry.base.ss = tss_entry.base.ds = 
        tss_entry.base.es = tss_entry.base.fs = 
        tss_entry.base.gs = 0x13;

    tss_entry.base.ss0 = 0x10;
    tss_entry.base.esp0 = 0;
    tss_entry.base.iopb_off = sizeof(struct tss_entry_base);
    memset(&tss_entry.iobmap, 0xFF, 8192);
}

void update_iobm(uint8_t *bm){
    memcpy(&(tss_entry.iobmap.bm), bm, 8192);
}

void update_kstack(uint32_t esp){
    tss_entry.base.esp0 = esp;
}

int gdt_init(){
    _gdtd.size = (sizeof(gdt_entry_t) * GDT_NUM_ENTRIES) - 1;
    _gdtd.addr = (uint32_t)&(_gdts[0]);

    memset(_gdts, 0, sizeof(gdt_entry_t) * GDT_NUM_ENTRIES);

    //All segments are mapped 4GB identity
    add_gdt(GDT_CODE_SEG,     0, 0xFFFFFFFF, 0x9A, 0xCF);
    add_gdt(GDT_DATA_SEG,     0, 0xFFFFFFFF, 0x92, 0xCF);
    add_gdt(GDT_USR_CODE_SEG, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    add_gdt(GDT_USR_DATA_SEG, 0, 0xFFFFFFFF, 0xF2, 0xCF);   

    add_tss(GDT_TSS_SEG);

    extern void gdt_flush(uint32_t ptr);
    gdt_flush((uintptr_t)&_gdtd);

    extern void tss_flush();
    tss_flush();

    log_printf(LOG_OK, "Setup GDT\n");

    return 0;
}

int gdt_fini(){
    return 0;
}

module_name(gdt);

module_load(gdt_init);
module_unload(gdt_fini);
