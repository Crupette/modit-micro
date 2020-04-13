BITS 32

extern _irq_handler

irq_common:
    pusha
    push ds
    push es
    push fs
    push gs
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    push esp
    call _irq_handler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds

    popa

    add esp, 8
    iret

;Requests
%macro IRQ 1
global _irq%1
_irq%1:
    cli
    push 0
    push %1+32
    jmp irq_common
%endmacro

%assign irqno 0
%rep (256 - 32)
IRQ irqno
%assign irqno irqno+1
%endrep
