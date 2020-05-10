#include <sys/proc.h>
#include <sys/msg.h>

#include <string.h>

pid_t find_process(const char *name){
    message_t *msg = message_prepare(sizeof(proc_req_t));
    if(msg == 0){
        return -1;
    }
    proc_req_t *req = (proc_req_t*)msg->dat;
    memset(req, 0, sizeof(*req));
    memcpy(req, "PREQ", 4);
    memcpy(req->name, name, strlen(name));
    
    message_send(msg, 1);

    while(1){
        msg = message_recv();
        if(PROC_REQ_CHKSUM(msg->dat)) break;
    }

    proc_req_ans_t *ans = (proc_req_ans_t*)msg->dat;
    pid_t r = ans->pid;

    message_release(msg);
    return r;
}
