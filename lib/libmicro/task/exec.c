#include "micro/task.h"
#include "api/syscall.h"

#include "kernel/initrd.h"

DEFN_SYSCALL3(exec, SYSCALL_EXEC, initrd_file_t *, const char*, size_t)

int micro_exec(struct FILE *file, const char *ustk, size_t usz){
    return syscall_exec(&file->desc, ustk, usz);
}
