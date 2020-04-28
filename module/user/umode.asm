[GLOBAL usr_switch]
[GLOBAL usr_ret]

;Args:
;   1: Pointer to run
;   2: Stack pointer
usr_switch:
    mov ebp, esp
    
    cli

    ;Use user-mode stack segments
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ;Setup flags and store stack pointer
    push 0x23
    mov eax, [ebp + 8]
    push eax
    pushf

    ;Re-enable interrupts
    pop eax
    or eax, 0x200
    push eax

    ;Store execution
    push 0x1B
    mov eax, [ebp + 4]
    push eax

    iret

usr_ret:

    add esp, 4
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
