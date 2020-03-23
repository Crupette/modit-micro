#include "kernel/modloader.h"
#include "kernel/print.h"

unsigned int status = 0;

int _init(){
	extern uintptr_t krnl_next_free_pg;
	vga_printf("Hello, kernelland! (%i) (%x)\n", status, krnl_next_free_pg);
	status += 5;
	return 0;
}

int _fini(){
	vga_printf("Goodbye cruel world! (%i)\n", status);
	return 0;
}

module_name(testmod);

module_load(_init);
module_unload(_fini);
