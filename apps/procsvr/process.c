#include "process_tree.h"

void parse_procreg(pid_t parent, proc_reg_t *msg){
    process_t *newproc = malloc(sizeof(process_t));

    newproc->pid = msg->target;
    memcpy(newproc->name, msg->name, 128);

    process_t *found = proc_find_by_name(msg->name);
    if(found != NULL){
        newproc->name[
            strlen(found->name) >= 128 ? 127 : strlen(found->name)] = found->refcount + '0';
        found->refcount++;
    }
    proc_add(newproc, parent);
}

void parse_procreq(message_t *root, proc_req_t *msg){
    process_t *found = proc_find_by_name(msg->name);

    proc_req_ans_t *ans = (proc_req_ans_t*)msg;
    if(found == NULL){
        ans->pid = -1;
    }else{
        ans->pid = found->pid;
    }
    
    root->msgsz = sizeof(proc_req_ans_t);
    message_send(root, root->src);
}
