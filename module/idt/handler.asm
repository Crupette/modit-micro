BITS 32

[GLOBAL idt_flush]
idt_flush:
    mov eax, [esp + 4]
    lidt [eax]
    ret
