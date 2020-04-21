[GLOBAL isr_77_syscall]
[EXTERN syscall_handler]

isr_77_syscall:
    cli

    push gs
    push fs
    push es
    push ds

    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    pop eax
       
    cld

    push ebp
    push edi
    push esi
    push edx
    push ecx
    push ebx
    push eax
   
    push esp
    call syscall_handler
    add esp, 8

    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    pop ebp

    pop ds
    pop es
    pop fs
    pop gs

    iret
