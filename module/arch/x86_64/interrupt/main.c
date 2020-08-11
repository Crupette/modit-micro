#include "kernel/modloader.h"
#include "kernel/print.h"


int int_init(){
    k_printf("Loading interrupt structures\n");
    return 0;
}

int int_fini(){
    return 0;
}

module_name(interrupt);

module_load(int_init);
module_unload(int_fini);
