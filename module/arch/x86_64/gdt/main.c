#include "micro/mod/gdt.h"
#include "kernel/modloader.h"
#include "kernel/print.h"

enum {
    GDT_NULL_SEG = 0,
    GDT_CODE_SEG,
    GDT_DATA_SEG,
    GDT_USR_CODE_SEG,
    GDT_USR_DATA_SEG,
    GDT_TSS_LOW,
    GDT_TSS_HIGH,
    GDT_NUM_ENTRIES
};

static gdt_desc_t _gdtd = { .addr = 0, .size = 0 };
static tss_entry_t _tss = { 0 };
struct {
    gdt_entry_t null;
    gdt_entry_t kernel_code;
    gdt_entry_t kernel_data;
    gdt_entry_t user_code;
    gdt_entry_t user_data;
    gdt_entry_t tss_low;
    gdt_entry_t tss_high;
} gdt_table = {
    { 0, 0, 0, 0x00, 0x00, 0 },
    { 0, 0, 0, 0x9A, 0xA0, 0 },
    { 0, 0, 0, 0x92, 0xA0, 0 },
    { 0, 0, 0, 0xFA, 0xA0, 0 },
    { 0, 0, 0, 0xF2, 0xA0, 0 },
    { 0, 0, 0, 0x89, 0xA0, 0 },
    { 0, 0, 0, 0x00, 0x00, 0 }
};

void update_iobm(uint8_t *bm){

}

void update_kstk(uintptr_t sp){

}

extern void load_gdt(gdt_desc_t *gdtd);
int gdt_init(){
    _gdtd.size = sizeof(gdt_table) - 1;
    _gdtd.addr = (uintptr_t)&gdt_table;

    uintptr_t tss_base = ((uintptr_t)&_tss);
    gdt_table.tss_low.base_low = tss_base & 0xFFFF;
    gdt_table.tss_low.base_mid = (tss_base >> 16) & 0xFF;
    gdt_table.tss_low.base_high = (tss_base >> 24) & 0xFF;
    gdt_table.tss_low.limit_low = sizeof(_tss);
    gdt_table.tss_high.limit_low = (tss_base >> 32) & 0xFFFF;

    load_gdt(&_gdtd);
    return 0;
}

int gdt_fini(){
    return 0;
}

module_name(gdt);

module_load(gdt_init);
module_unload(gdt_fini);
