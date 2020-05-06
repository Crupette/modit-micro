/*  USER.H -    Handles usermode process permissions and execution
 *  
 *  Author: Crupette
 * */

#ifndef MODULE_USER_H
#define MODULE_USER_H 1

#include "module/tasking.h"
#include "module/datatype/list.h"
#include "api/syscall.h"
#include "api/message.h"

#include "kernel/initrd.h"

#define MSG_BUFFERMAX  128
#define MSG_MAXLEN     4096

typedef struct user_task {
    task_t *task;
    uint32_t perms;
    syscall_state_t state;

    uint32_t pid;

    message_buffer_t recvbuf;
    list_t *sendbuf;
    bool msg_ack;
} user_task_t;

/*  Spawns a new usermode task, which begins usermode execution at ELF start
 *  file:      ELF file to load
 *  ustkdata:  User stack data
 *  ustksize:  User stack size
 *  perms:     Permissions to give the new process
 *  r:         Pointer to new user task
 * */
user_task_t *user_spawn(initrd_file_t *file, void *ustkdata, uintptr_t ustksize, uint32_t perms);

/*  Forks the current context
 *  r: Successful
 * */
int user_fork();

/*  Replaces the current context's program with ELF in file
 *  file:     ELF file to load
 *  ustkdata: User stack data
 *  ustksize: User stack size
 *  perms:    Permissions given to the process
 * */
void user_exec(initrd_file_t *file, void *ustkdata, uintptr_t ustksize, uint32_t perms);

/* Blocks the task from running, making the tasking module skip it
 * id:  Pid of the task to block
 * Does nothing if PID is not found
 * */
void user_block(uint32_t id);

/*  Awakens the task, allowing the tasking module to run it
 *  id: Pid of task to awake
 *  Does nothing if PID is not found
 * */
void user_awake(uint32_t id);

/*  Tests if the task is blocked
 *  id: Pid to test
 *  r:  If the task is blocked. Returns false if couldn't find PID
 * */
bool user_isblocked(uint32_t id);

/*  Finds the user task using pid
 *  id: PID
 *  r:  User task (if found)
 * */
user_task_t *user_find(uint32_t id);

/*  Loads the given ELF file into context's memory
 *  file:   File to load
 *  return: Address of entry point. Returns 0 if invalid file / not ELF
 * */
uintptr_t modit_elf_load(initrd_file_t *file);

#endif
