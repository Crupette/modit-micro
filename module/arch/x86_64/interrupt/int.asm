BITS 64

[GLOBAL idt_flush]
idt_flush:
    lidt [rdi]
    ret

[EXTERN _int_handler]

%macro pushall 0
    push rax
    push rcx
    push rdx
    push rbx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    sub rsp, 8
    stmxcsr [rsp]
%endmacro

%macro popall 0
    ldmxcsr [rsp]
    add rsp, 8

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rbx
    pop rdx
    pop rcx
    pop rax
%endmacro

_int_common:
    pushall
.handle:
    cld
    lea rdi, [rsp + 8]
    call _int_handler

.return:
    popall
    add rsp, 16
    iretq

%macro IRQ 1
global _irq%1
_irq%1:
    cli
    push 0
    push %1+32
    jmp _int_common
%endmacro

%macro ISR_CODE 1
global _isr%1
_isr%1:
    cli
    push %1
    jmp _int_common
%endmacro

%macro ISR_NOCODE 1
global _isr%1
_isr%1:
    cli
    push 0
    push %1
    jmp _int_common
%endmacro

ISR_NOCODE  0
ISR_NOCODE  1
ISR_NOCODE  2
ISR_NOCODE  3
ISR_NOCODE  4
ISR_NOCODE  5
ISR_NOCODE  6
ISR_NOCODE  7
ISR_CODE    8
ISR_NOCODE  9
ISR_CODE    10
ISR_CODE    11
ISR_CODE    12
ISR_CODE    13
ISR_CODE    14
ISR_NOCODE  15
ISR_NOCODE  16
ISR_CODE    17
ISR_NOCODE  18
ISR_NOCODE  19
ISR_NOCODE  20
ISR_NOCODE  21
ISR_NOCODE  22
ISR_NOCODE  23
ISR_NOCODE  24
ISR_NOCODE  25
ISR_NOCODE  26
ISR_NOCODE  27
ISR_NOCODE  28
ISR_NOCODE  29
ISR_NOCODE  30
ISR_NOCODE  31

%assign irq 0
%rep (256 - 32)
IRQ irq
%assign irq irq+1
%endrep
