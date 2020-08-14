BITS 64

[EXTERN _int_handler]
[EXTERN _int_common]

%macro IRQ 1
global _irq%1
_irq%1:
    push 0
    push %1+32
    jmp _int_common
%endmacro

%assign irq 0
%rep (256 - 32)
IRQ irq
%assign irq irq+1
%endrep
