#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel/vgaterm.h"
#include "kernel/multiboot.h"
#include "kernel/memory.h"
#include "kernel/initrd.h"
#include "kernel/modloader.h"
#include "kernel/print.h"

extern uint32_t _begin;
extern uint32_t _end;

/*	Initializes the kernel, then passes control to kernel modules
 * */
void kernel_main(multiboot_info_t *mbinfo, int magic){
	mbinfo = (multiboot_info_t*)(((uint32_t)mbinfo) + VIRT_BASE);
	vgaterm_init();

	if(magic != MULTIBOOT_BOOTLOADER_MAGIC){
		vgaterm_putstr("[ERR]: Multiboot info not passed\n");
		return;
	}

	kmem_init(mbinfo);
	initrd_init(mbinfo);
	kmod_init(mbinfo);
}
