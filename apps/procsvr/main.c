#include <stdio.h>
#include <sys/proc.h>

#include <sys/msg.h>

void handle_message(message_t *msg){
    if(PROC_REG_CHKSUM(msg->dat)){
        proc_reg_t *preg = (proc_reg_t*)msg->dat;
        printf("Process %s(%i) sent a valid procreg message\n",
                preg->name, msg->src);
    }
}

int main(int argc, char **argv){   
    while(1){
        message_t *msg = message_recv();

        printf("[PROCSVR]: Got msg %p (%i bytes) from %i\n",
                msg, msg->msgsz, msg->src);
        
        handle_message(msg);
        message_release(msg);
    }

    while(1) {}
    return 0;
}
