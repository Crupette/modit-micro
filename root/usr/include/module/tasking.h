/*  TASKING.H - Provides multitasking functionality.
 *              Scheduling is handled by usermode processes
 *
 *  Author: Crupette
 * */

#ifndef MODULE_TASKING_H
#define MODULE_TASKING_H 1

#include "kernel/types.h"
#include "kernel/memory.h"

typedef struct task {
    size_t tslice;
    int8_t priority;
    bool blocked;

    uintptr_t ksp;
    uintptr_t kbp;
    uintptr_t ip;

    uintptr_t kstack_top;
    page_directory_t *dir;
    uint8_t *iobm;

    void *parent_struct;
} task_t;

/*  Forks the currently running task and jumps to func
 *  func: Function to execute on new task
 *  stk:  Stack to continue execution on
 *  r:    Task object created
 * */
task_t *task_newtask(void (*func)(void), uintptr_t stk);

/*  Disables and enables tasking respectively
 * */
void tasking_disable();
void tasking_enable();

#endif
