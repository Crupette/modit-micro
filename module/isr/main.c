#include "module/interrupt.h"

#include "kernel/modloader.h"
#include "kernel/logging.h"
#include "kernel/io.h"

static isr_handler_t _handlers[32] = { 0 };

extern void __panic(interrupt_state_t *r, char *fmt, ...);

void isr_addHandler(uint8_t i, isr_handler_t handler){
    if(i >= 32) return;
    _handlers[i] = handler;
}

void _unhandled_exception(interrupt_state_t *s){
    switch(s->num){
        case 0:
            __panic(s, "Unhandled Divide by Zero Error @ %p\n", s->eip);
        break;
        case 2:
            __panic(s, "Unhandled NMI\n");
        break;
        case 4:
            __panic(s, "Unhandled Overflow exception @ %p\n", s->eip);
        break;
        case 5:
            __panic(s, "Unhandled Bound Range Exceeded exception @ %p\n", s->eip);
        break;
        case 6:
            __panic(s, "Unhandled Invalid Opcode exception @ %p\n", s->eip);
        break;
        case 7:
            __panic(s, "Unhandled DNA @ %p\n", s->eip);
        break;
        case 8:
            __panic(s, "Unhandled Double Fault exception");
        break;
        case 10:
            __panic(s, "Unhandled Invalid TSS exception (%i)\n", s->err);
        break;
        case 11:
            __panic(s, "Unhandled Segment Not Present exception (%i)\n", s->err);
        break;
        case 12:
            __panic(s, "Unhandled Stack Segment Fault (%i)\n", s->err);
        break;
        case 13:
            __panic(s, "Unhandled General Protection Fault (%i)\n", s->err);
        break;
        case 14:
            __panic(s, "Unhandled Page Fault @ %p (err %i)\n", s->eip, s->err);
        break;
        default:
            __panic(s, "Unhandled Unknown exception @ %p (err %i)\n", s->eip, s->err);
        break;
    }
}

void _isr_handler(interrupt_state_t *state){
    //Handler must exist to be called
    if(_handlers[state->num] != 0){
        _handlers[state->num](state);
    }else{
        _unhandled_exception(state);
        asm("hlt");
    }
}

int isr_init(void){
    idt_createEntry(0, _isr0, 0x08, 0x8E);
    idt_createEntry(1, _isr1, 0x08, 0x8E);
    idt_createEntry(2, _isr2, 0x08, 0x8E);
    idt_createEntry(3, _isr3, 0x08, 0x8E);
    idt_createEntry(4, _isr4, 0x08, 0x8E);
    idt_createEntry(5, _isr5, 0x08, 0x8E);
    idt_createEntry(6, _isr6, 0x08, 0x8E);
    idt_createEntry(7, _isr7, 0x08, 0x8E);
    idt_createEntry(8, _isr8, 0x08, 0x8E);
    idt_createEntry(9, _isr9, 0x08, 0x8E);
    idt_createEntry(10, _isr10, 0x08, 0x8E);
    idt_createEntry(11, _isr11, 0x08, 0x8E);
    idt_createEntry(12, _isr12, 0x08, 0x8E);
    idt_createEntry(13, _isr13, 0x08, 0x8E);
    idt_createEntry(14, _isr14, 0x08, 0x8E);
    idt_createEntry(15, _isr15, 0x08, 0x8E);
    idt_createEntry(16, _isr16, 0x08, 0x8E);
    idt_createEntry(17, _isr17, 0x08, 0x8E);
    idt_createEntry(18, _isr18, 0x08, 0x8E);
    idt_createEntry(19, _isr19, 0x08, 0x8E);
    idt_createEntry(20, _isr20, 0x08, 0x8E);
    idt_createEntry(21, _isr21, 0x08, 0x8E);
    idt_createEntry(22, _isr22, 0x08, 0x8E);
    idt_createEntry(23, _isr23, 0x08, 0x8E);
    idt_createEntry(24, _isr24, 0x08, 0x8E);
    idt_createEntry(25, _isr25, 0x08, 0x8E);
    idt_createEntry(26, _isr26, 0x08, 0x8E);
    idt_createEntry(27, _isr27, 0x08, 0x8E);
    idt_createEntry(28, _isr28, 0x08, 0x8E);
    idt_createEntry(29, _isr29, 0x08, 0x8E);
    idt_createEntry(30, _isr30, 0x08, 0x8E);
    idt_createEntry(31, _isr31, 0x08, 0x8E);

    log_printf(LOG_OK, "Setup ISR Routines\n");
    return 0;
}

int isr_fini(){
    log_printf(LOG_WARNING, "Removing ISR hooks\n");
    return 0;
}

module_name(isr);

module_load(isr_init);
module_unload(isr_fini);

module_depends(idt);
module_depends(panic);
