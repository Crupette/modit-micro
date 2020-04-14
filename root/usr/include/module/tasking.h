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
    bool new;

    uintptr_t *ksp;
    uintptr_t kbp;
    page_directory_t *dir;
} task_t;

#endif
