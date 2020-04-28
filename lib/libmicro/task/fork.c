#include "micro/task.h"
#include "api/syscall.h"

DEFN_SYSCALL0(fork, SYSCALL_FORK)

int micro_fork(void){
    return syscall_fork();
}
