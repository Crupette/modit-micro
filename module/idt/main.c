#include "module/interrupt.h"

#include "kernel/modloader.h"
#include "kernel/logging.h"
#include "kernel/io.h"

typedef struct idt {
    idt_entry_t entries[256];
    idt_ptr_t ptr;
} idt_t;

static idt_t _idt;

void idt_createEntry(uint8_t i, int_handler_t func, uint16_t selector, uint8_t flags){
    idt_entry_t *entry = &(_idt.entries[i]);
    entry->offset_low = (uintptr_t)func & 0xFFFF;
    entry->offset_high = ((uintptr_t)func >> 16) & 0xFFFF;
    entry->selector = selector;
    entry->flags = flags;
}

void setup_idt(){
    _idt.ptr.size = (sizeof(idt_entry_t) * 256) - 1;
    _idt.ptr.addr = (uintptr_t)&_idt.entries[0];
    idt_ptr_t *paddr = &_idt.ptr;

    extern void idt_flush(idt_ptr_t *ptr);
    idt_flush(paddr);
    log_printf(LOG_OK, "Setup IDT\n");
}

extern void idt_flush(idt_ptr_t *idt);
int idt_init(){
    memset(&_idt, 0, sizeof(_idt));
    setup_idt();

    log_printf(LOG_OK, "Loaded IDT\n");
    return 0;
}

int idt_fini(){
    asm("sti");
    log_printf(LOG_WARNING, "Unloading IDT : I hope you don't plan on having interrupts\n");
    return 0;
}

module_name(idt);

module_load(idt_init);
module_unload(idt_fini);

module_depends(gdt);
