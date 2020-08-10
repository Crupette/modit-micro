[GLOBAL load_gdt]
load_gdt:
    lgdt [rdi]
    mov ax, 40
    ltr ax
    ret
