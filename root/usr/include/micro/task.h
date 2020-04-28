/*  TASK.H - Protocol for task spawning, IPC, etc
 *
 *  Author: Crupette
 * */
#ifndef MICRO_TASK_H
#define MICRO_TASK_H 1

#include <stdint.h>
#include <stddef.h>

#include "micro/file.h"

int micro_fork(void);
int micro_exec(struct FILE *file, const char *ustk, size_t usz);
int micro_spawn(struct FILE *file, const char *ustk, size_t usz);

#endif
