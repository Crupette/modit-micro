#ifndef PROCESS_H
#define PROCESS_H 1

#include <stddef.h>
#include <stdint.h>

#include <sys/proc.h>
#include <sys/msg.h>

typedef struct process {
    char name[128];
    uintptr_t pid;
    uint8_t refcount;
} process_t;

void parse_procreg(pid_t parent, proc_reg_t *msg);
void parse_procreq(message_t *root, proc_req_t *msg);


#endif
