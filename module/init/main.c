#include "module/user.h"
#include "module/heap.h"
#include "module/interrupt.h"

#include "kernel/string.h"
#include "kernel/modloader.h"
#include "kernel/memory.h"
#include "kernel/logging.h"
#include "kernel/initrd.h"

char init_stk[16] = { 0 };
extern void __panic(interrupt_state_t *r, char *fmt, ...);

int init_init(){
    uint32_t filecount = 0;
    initrd_file_t *files = initrd_get_files(&filecount);

    for(uint32_t i = 0; i < filecount; i++){
        initrd_file_t *file = &(files[i]);
        if(strcmp(file->name, "init") == 0){
            user_spawn(file, init_stk, 16, 0xFFFFFFFF);
            return 0;
        }
    }
    __panic(0, "Failed to load init!\n");
    return -1;
}

int init_fini(){
    return 0;
}

module_name(init);

module_load(init_init);
module_unload(init_fini);

module_depends(syscall);
module_depends(user);
module_depends(message);
module_depends(signal);
module_depends(panic);
