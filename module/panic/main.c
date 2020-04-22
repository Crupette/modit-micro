#include "module/interrupt.h"

#include "kernel/modloader.h"
#include "kernel/logging.h"
#include "kernel/print.h"
#include "kernel/vgaterm.h"
#include "kernel/io.h"
#include "kernel/lock.h"

extern DECLARE_LOCK(vga_op_lock);
bool panicd = 0;

void print_context(interrupt_state_t *r){
    uint32_t om = 0xDEAD;
    uint32_t os = 0xDEAD;
    
    char *mod = get_module_nearest(r->eip, &om);
    char *sym = get_symbol_nearest(r->eip, &os);
    
    vga_printf("EIP: %X [%s:%X] (%s:%X)\n",
            r->eip, mod, om,
                    sym, os);
    vga_printf("EAX: %X   EBX: %X   ECX: %X   EDX: %X\n",
            r->eax, r->ebx, r->ecx, r->edx);
    vga_printf("ESI: %X   EDI: %X   ESP: %X   EBP: %X\n",
            r->esi, r->edi, r->esp, r->ebp);
    vga_printf("DS:  %X   ES:  %X   FS:  %X   GS:  %X\n",
            r->ds, r->es, r->fs, r->gs);
    vga_printf("CS:  %X   FLG: %X   USP: %X   SS:  %X\n",
            r->cs, r->eflags, r->usresp, r->ss);
}

typedef struct stkframe {
    struct stkframe *ebp;
    uintptr_t eip;
} stack_frame_t;

void print_stk(interrupt_state_t *r){
    stack_frame_t *stk = r->ebp;
    for(uint8_t f = 0; stk && f < 20; f++){
        uint32_t om = 0xDEAD;
        uint32_t os = 0xDEAD;

        char *mod = get_module_nearest(stk->eip, &om);
        char *sym = get_symbol_nearest(stk->eip, &os);

        vga_printf("[%X]: (%s:%s:%i)\n",
                stk->eip, mod,
                          sym, os);
        stk = stk->ebp;
    }
}

void __panic(interrupt_state_t *r, char *fmt, ...){
    UNLOCK(vga_op_lock);

    va_list ap;
    va_start(ap, fmt);

    char buf[256];
    memset(buf, 0, 256);

    if(panicd){
         vga_printf("\033[33;41m========================== \033[91mSECOND ORDER  KERNEL PANIC \033[33m==========================\033[97;40m\n");   
         asm volatile("hlt");
    }else{
        vga_printf("\033[33m================================= \033[91mKERNEL PANIC \033[33m=================================\033[97m\n");
    }
 
    k_vprintf(fmt, ap, buf);
    vga_printf("%s\n", buf);

    if(r != 0){
        vga_printf("Context:\n");
        print_context(r);
        
        if(!panicd){
            panicd = true;
            vga_printf("Stack trace:\n");
            print_stk(r);
        }
    }

    vga_printf("\nSystem halted!\n");

    while(true) asm("hlt");
}

int panic_init(){
    return 0;
}

int panic_fini(){
    return 0;
}

module_name(panic);

module_load(panic_init);
module_unload(panic_fini);
