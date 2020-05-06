/*  PROC.H - C library wrapper for libmicro fork, exec, and spawn calls
 *  Interfaces with the process server to find and communicate with other processes
 *
 *  Author: Crupette
 * */
#ifndef SYS_PROC_H
#define SYS_PROC_H 1

#include <stddef.h>
#include <stdio.h>

#define PROCESS_NAME_MAXLEN 64

typedef uint32_t pid_t;

typedef struct proc_reg {
    uint8_t chksum[4];  //PREG
    char name[128];
    pid_t parent;
} proc_reg_t;

#define PROC_REG_CHKSUM(r) \
    (char*)r[0] == 'P' && \
    (char*)r[1] == 'R' && \
    (char*)r[2] == 'E' && \
    (char*)r[3] == 'G'

typedef struct prog_req {
    uint8_t chksum[4];
    char name[128];
} proc_req_t;

#define PROC_REQ_CHKSUM(r) \
    (char*)r[0] == 'P' && \
    (char*)r[1] == 'R' && \
    (char*)r[2] == 'E' && \
    (char*)r[3] == 'Q'

//TODO: Environment variables

/*  Duplicates the current execution environment.
 *  r:  New task's PID
 * */
pid_t fork(void);

/*  Replaces the current execution environment with the one pointed at file.
 *  arg is the first argument. If null, no arguments should be passed
 *  The last passed argument MUST be NULL.
 *  path:   Path to file to execute (absolute)
 *  r:      If the execution is successful. No return expected if so
 * */
int execl(const char *path, const char *arg, ...);

/*  Same as execl, except arguments are stored as a list of pointers, with the last being NULL
 *  path:   Path to file (absolute)
 *  argv:   Argument list for new context
 *  r:      If the execution is successful. No return expected if so
 * */
int execv(const char *path, const char *argv[]);

/*  Creates a new execution environment pointed at by path
 *  Everything else is the same as execl
 *  path:   Path to new file to spawn (absolute)
 *  r:      PID of the new spawned task. 0 if unsucessful
 * */
pid_t spawnl(const char *path, const char *arg, ...);

/*  Same as spawnl, but arguments are passed as a list of pointers, with the last being NULL
 *  path:   Path to file (absolute)
 *  argv:   Argument list for new context
 *  r:      PID of the new spawned task. 0 if unsuccessful
 * */
pid_t spawnv(const char *path, const char *argv[]);

#endif
