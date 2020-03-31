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

multiboot_info_t *mbinfo = 0;

/*  Initializes the kernel, then passes control to kernel modules
 * */
void kernel_main(multiboot_info_t *mbi, int magic){
    mbi = (multiboot_info_t*)(((uint32_t)mbi) + VIRT_BASE);
    mbinfo = mbi;

    vgaterm_init();

    if(magic != MULTIBOOT_BOOTLOADER_MAGIC){
        vgaterm_putstr("[ERR]: Multiboot info not passed\n");
        return;
    }

    kmem_init();
    initrd_init();
    kmod_init();

    //This should not return:
    //  Modules will be loaded, which will include a process manager and idle process
    //  This idle process will take-over the kernel context
}
