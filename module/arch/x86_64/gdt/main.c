#include "micro/mod/interrupt.h"
#include "kernel/modloader.h"
#include "kernel/print.h"
#include "kernel/io.h"

int gdt_init(){
    return 0;
}

int gdt_fini(){
    return 0;
}

module_name(gdt);

module_load(gdt_init);
module_unload(gdt_fini);
