#include "micro/task.h"
#include "api/syscall.h"

#include "kernel/initrd.h"

DEFN_SYSCALL3(spawn, SYSCALL_SPAWN, initrd_file_t *, const char*, size_t)

int micro_spawn(struct FILE *file, const char *ustk, size_t usz){
    return syscall_spawn(&file->desc, ustk, usz);
}
