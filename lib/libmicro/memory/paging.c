#include "micro/paging.h"
#include "api/syscall.h"

DEFN_SYSCALL1(resvpg, SYSCALL_RESVPG, void*)    //Phys
DEFN_SYSCALL3(mappg, SYSCALL_MAPPG, void*, void*, uint32_t)    //phys, virt, flgs
DEFN_SYSCALL3(allocpgs, SYSCALL_ALLOCPGS, void*, size_t, uint32_t)  //virt, cnt, flgs
DEFN_SYSCALL2(freepgs, SYSCALL_FREEPGS, void*, size_t) //virt, cnt

int micro_resvpg(void *phys){
    return syscall_resvpg(phys);
}

int micro_mappg(void *phys, void *virt, uint32_t flgs){
    return syscall_mappg(phys, virt, flgs);
}

int micro_allocpgs(void *virt, size_t cnt, uint32_t flgs){
    return syscall_allocpgs(virt, cnt, flgs);
}

int micro_freepgs(void *virt, size_t cnt){
    return syscall_freepgs(virt, cnt);
}
