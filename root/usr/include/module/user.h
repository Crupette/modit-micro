/*  USER.H -    Handles usermode process permissions and execution
 *  
 *  Author: Crupette
 * */

#ifndef MODULE_USER_H
#define MODULE_USER_H 1

#include "module/tasking.h"

#include "kernel/initrd.h"

typedef struct user_task {
    task_t *task;
    uint32_t perms;

    uint32_t pid;
} user_task_t;

#define USER_PERM_IO 0x1

/*  Spawns a new usermode process, which begins usermode execution at ELF start
 *  entry:     Usermode entry
 *  ustkdata:  User stack data
 *  ustksize:  User stack size
 *  perms:     Permissions to give the new process
 *  r:         Pointer to new user task
 * */
user_task_t *user_spawn(initrd_file_t *file, void *ustkdata, uintptr_t ustksize, uint32_t perms);

/*  Loads the given ELF file into context's memory
 *  file:   File to load
 *  return: Address of entry point
 * */
uintptr_t modit_elf_load(initrd_file_t *file);

#endif
