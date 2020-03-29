BITS 32

[GLOBAL idt_flush]
idt_flush:
	mov eax, [esp + 4]
	lidt [eax]
	ret

extern _interrupt_handler

interrupt_common:
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
	call _interrupt_handler
	add esp, 4

	pop gs
	pop fs
	pop es
	pop ds

	popa

	add esp, 8
	iret

%macro ISR_CODE 2
global %1
%1:
	cli
	push %2
	jmp interrupt_common
%endmacro

%macro ISR_NOCODE 2
global %1
%1:
	cli
	push 0
	push %2
	jmp interrupt_common
%endmacro

ISR_NOCODE	_isr0, 0	;Divide by 0
ISR_NOCODE	_isr1, 1	;Debug
ISR_NOCODE	_isr2, 2	;NMI
ISR_NOCODE	_isr3, 3	;Breakpoint
ISR_NOCODE	_isr4, 4	;Overflow
ISR_NOCODE	_isr5, 5	;OOB
ISR_NOCODE	_isr6, 6	;Invalid Opcode
ISR_NOCODE	_isr7, 7	;Device not available
ISR_CODE	_isr8, 8	;Double fault
ISR_NOCODE	_isr9, 9	;Coprocessor Segment Overrun (Depricated)
ISR_CODE	_isr10, 10	;Invalid TSS
ISR_CODE	_isr11, 11	;Segment not present
ISR_CODE	_isr12, 12	;Stack-segment fault
ISR_CODE	_isr13, 13	;GPF
ISR_CODE	_isr14, 14	;Page fault
ISR_NOCODE	_isr15, 15	;Reserved
ISR_NOCODE	_isr16, 16	;x87 Floating-point exception
ISR_CODE	_isr17,	17	;Alignment check
ISR_NOCODE	_isr18, 18	;Machine check
ISR_NOCODE	_isr19, 19	;SIMD Floating-point exception
ISR_NOCODE	_isr20, 20	;Virtualization exception

;Reserved codes
ISR_NOCODE	_isr21, 21
ISR_NOCODE	_isr22, 22
ISR_NOCODE	_isr23, 23
ISR_NOCODE	_isr24, 24
ISR_NOCODE	_isr25, 25
ISR_NOCODE	_isr26, 26
ISR_NOCODE	_isr27, 27
ISR_NOCODE	_isr28, 28
ISR_NOCODE	_isr29, 29
ISR_NOCODE	_isr30, 30
ISR_NOCODE	_isr31, 31

;Requests
%macro IRQ 1
global _irq%1
_irq%1:
	cli
	push 0
	push %1+32
	jmp interrupt_common
%endmacro

%assign irqno 0
%rep (256 - 32)
IRQ irqno
%assign irqno irqno+1
%endrep
