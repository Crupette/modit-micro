#include "module/gdt.h"

#include "kernel/modloader.h"
#include "kernel/print.h"
#include "kernel/io.h"

enum {
	GDT_NULL_SEG,
	GDT_CODE_SEG,
	GDT_DATA_SEG,
	GDT_USR_CODE_SEG,
	GDT_USR_DATA_SEG,
	GDT_TSS_SEG,
	GDT_NUM_ENTRIES
};

gdt_entry_t _gdts[GDT_NUM_ENTRIES];
gdt_descriptor_t _gdtd;

gdt_entry_t build_gdt(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags){
	gdt_entry_t entry;
	
	entry.base_low = base & 0xFFFF;
	entry.base_mid = (base >> 16) & 0xFF;
	entry.base_high = (base >> 24) & 0xFF;

	entry.limit_low = limit & 0xFFFF;
	entry.limit_high = (limit >> 16) & 0xF;

	entry.access = access;
	entry.flags |= (flags & 0xF0);

	return entry;
}

void add_gdt(unsigned int target, gdt_entry_t src){
	if(target >= GDT_NUM_ENTRIES) return;
	_gdts[target] = src;
}

int _init(){
	_gdtd.size = (sizeof(gdt_entry_t) * GDT_NUM_ENTRIES) - 1;
	_gdtd.addr = (uint32_t)&(_gdts[0]);

	memset(_gdts, 0, sizeof(gdt_entry_t) * GDT_NUM_ENTRIES);

	add_gdt(GDT_CODE_SEG, build_gdt(0, 0xFFFFFFFF, 0x9A, 0xCF));
	add_gdt(GDT_DATA_SEG, build_gdt(0, 0xFFFFFFFF, 0x92, 0xCF));
	add_gdt(GDT_USR_CODE_SEG, build_gdt(0, 0xFFFFFFFF, 0xFA, 0xCF));
	add_gdt(GDT_USR_DATA_SEG, build_gdt(0, 0xFFFFFFFF, 0xF2, 0xCF));

	extern void gdt_flush(uint32_t ptr);
	gdt_flush((uintptr_t)&_gdtd);

	vga_printf("[OK]: Setup GDT\n");

	return 0;
}

int _fini(){
	return 0;
}

module_name(gdt);

module_load(_init);
module_unload(_fini);
