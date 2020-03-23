ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

phys = 0x100000;
offset = 0xE0000000;
virt = offset + phys;

SECTIONS
{
	. = virt;
	_begin = .;
	
	.text : AT(ADDR(.text) - offset) 
	{
		*(.text)
		*(.rodata)
		. = ALIGN(0x1000);
	}	

	.data : AT(ADDR(.data) - offset)
	{
		*(.data)
		. = ALIGN(0x1000);
	}

	.bss : AT(ADDR(.bss) - offset)
	{
		*(COMMON)
		*(.bss)
		. = ALIGN(0x1000);
	}

	_end = .;
}
