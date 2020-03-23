MBALIGN	equ	1 << 0
MEMINFO	equ	1 << 1

FLAGS	equ	MBALIGN | MEMINFO
MAGIC	equ	0x1BADB002
CHKSUM	equ	-(MAGIC + FLAGS)

VIRT_BASE equ	0xE0000000
PD_INDEX equ	(VIRT_BASE >> 22)

section .bss
stack_bottom:
	resb 16384
stack_top:

section .data
align 4096
global boot_page_dir
boot_page_dir:
	dd 0x00000083
	times(PD_INDEX - 1) dd 0
	dd 0x00000083
	times(1024 - PD_INDEX - 1) dd 0

section .text
	dd MAGIC
	dd FLAGS
	dd CHKSUM
extern kernel_main
global _start
_start:
	mov ecx, (boot_page_dir - VIRT_BASE)
	mov cr3, ecx

	mov ecx, cr4
	or ecx, 0x10
	mov cr4, ecx

	mov ecx, cr0
	or ecx, 0x80000000
	mov cr0, ecx

	lea ecx, [.higher_half]
	jmp ecx
.higher_half:
	;mov dword [boot_page_dir], 0
	;invlpg[0]

	mov esp, stack_top
	
	push eax
	push ebx

	call kernel_main
.hlt:
	hlt
	jmp .hlt

[GLOBAL _krnl_enablepg]
_krnl_enablepg:
	cli

	mov eax, dword [esp + 4]

	lea ebx, [.lower - VIRT_BASE]
	jmp ebx
.lower:
	mov ebx, cr0
	xor ebx, 0x80000000
	mov cr0, ebx

	mov cr3, eax
	mov ebx, cr4
	xor ebx, 0x10
	mov cr4, ebx

	mov ebx, cr0
	or ebx, 0x80000000
	mov cr0, ebx

	lea ecx, [.final]
	jmp .final
.final:
	ret

[GLOBAL outb]
[GLOBAL inb]

outb:
	mov al, [esp + 8]
	mov dx, [esp + 4]
	out dx, al
	ret

inb:
	mov dx, [esp + 4]
	in al, dx
	ret
