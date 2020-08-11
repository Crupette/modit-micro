[GLOBAL load_gdt]
load_gdt:
    lgdt [rdi]
    mov ax, 0x28
    ltr ax
    ret
