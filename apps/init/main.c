#include <stdio.h>
#include <sys/proc.h>
#include <sys/msg.h>

int main(int argc, char **argv){   
    spawnl("procsvr", NULL);

    message_t *regmsg = message_prepare(sizeof(proc_reg_t));
    if(regmsg == 0){
        printf("Failed to allocate msg buffer for registry\n");
        return -1;  //Failed to allocate message buffer
    }
    proc_reg_t *regstr = (proc_reg_t*)regmsg->dat;
    memset(regstr, 0, sizeof(proc_reg_t));
    memcpy(regstr, "PREG", 4);
    memcpy(regstr->name, "init", 5);
    regstr->parent = 0;

    message_send(regmsg, 1);

    while(1) {}
    return 0;
}
