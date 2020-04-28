//TODO: Send signals to all services to find process requirements. For now, assume we are alone

#include "micro/system.h"
#include "micro/io.h"

#include "api/syscall.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

DEFN_SYSCALL0(getperms, SYSCALL_GETPERMS)

uint32_t micro_getperms(void){
    return syscall_getperms();
}

void micro_init(){
    uint32_t perms = micro_getperms();
    if((perms & USER_PERM_MEM) == 0){
        syscall_print("System does not have enough permissions to start\n");
        return;
    }
}

void micro_fini(){

}
