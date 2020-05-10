#include <sys/proc.h>
#include <sys/msg.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void __send_preg(const char *path, pid_t target){
    message_t *msg = message_prepare(sizeof(proc_reg_t));
    if(msg == 0){
        //TODO: Set errno
        printf("Failed to register new process (%s)\n", path);
        return;
    }

    proc_reg_t *regstr = (proc_reg_t*)msg->dat;
    memset(regstr, 0, sizeof(proc_reg_t));
    memcpy(regstr, "PREG", 4);
    memcpy(regstr->name, path, strlen(path));
    regstr->target = target;

    message_send(msg, 1);
}
