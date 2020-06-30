#include "kernel/modloader.h"
#include "kernel/memory.h"
#include "kernel/logging.h"
#include "kernel/io.h"

int signal_init(){
    return 0;
}

int signal_fini(){
    return 0;
}

module_name(signal);

module_load(signal_init);
module_unload(signal_fini);

module_depends(tasking);
