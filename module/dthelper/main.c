#include "module/datatype/list.h"

#include "kernel/modloader.h"
#include "kernel/logging.h"
#include "kernel/io.h"

int dthlpr_init(){
    log_printf(LOG_OK, "Setup definitions for datatypes\n");
    return 0;
}

int dthlpr_fini(){
    return 0;
}

module_name(dthelper);

module_load(dthlpr_init);
module_unload(dthlpr_fini);

module_depends(heap);
module_depends(logger);
