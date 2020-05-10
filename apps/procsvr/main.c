#include <stdio.h>
#include <sys/proc.h>

#include <sys/msg.h>

#include "process_tree.h"

void handle_message(message_t *msg){
    if(PROC_REG_CHKSUM(msg->dat)){
        proc_reg_t *preg = (proc_reg_t*)msg->dat;
        parse_procreg(msg->src, preg); 
        message_release(msg);
    }
    if(PROC_REQ_CHKSUM(msg->dat)){
        proc_req_t *preq = (proc_req_t*)msg->dat;
        parse_procreq(msg, preq);
    }
}

int main(int argc, char **argv){
    proc_init();
    while(1){
        message_t *msg = message_recv(); 
        handle_message(msg);
    }

    while(1) {}
    return 0;
}
