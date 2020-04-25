/*  TASK.H - Protocol for task spawning, IPC, etc
 *
 *  Author: Crupette
 * */
#ifndef MICRO_TASK_H
#define MICRO_TASK_H 1

#include <stdint.h>
#include <stddef.h>

int micro_fork(void);
int micro_exec(char *stk, size_t size);
int micro_spawn(char *stk, size_t size);

#endif
