[GLOBAL gdt_flush]
[GLOBAL tss_flush]

gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]
    jmp 0x8:.flush  ; Segments need to be reloaded
.flush:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret
    nop

tss_flush:
    mov ax, 0x2B
    ltr ax
    ret
