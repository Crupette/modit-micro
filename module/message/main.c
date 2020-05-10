#include "module/heap.h"
#include "module/user.h"
#include "module/tasking.h"
#include "api/message.h"

#include "kernel/modloader.h"
#include "kernel/logging.h"
#include "kernel/io.h"

int message_push(message_buffer_t *buf, message_t *msg){
    buf->buffer[buf->tail] = msg;
    buf->tail++;

    if(buf->tail >= MSG_MAXCOUNT) buf->tail = 0;
    if(buf->count < MSG_MAXCOUNT) buf->count++;
    return 0;
}

message_t *message_pop(message_buffer_t *buf){
    if(buf->count == 0) return 0;

    message_t *r = buf->buffer[buf->head];
    buf->head++;

    if(buf->head < 0) buf->head = MSG_MAXCOUNT - 1;

    buf->count--;

    return r;
}

message_t *message_peep(message_buffer_t *buf){
    if(buf->count == 0) return 0;
    return buf->buffer[buf->head];
}

extern void tasking_tick(void);
extern list_node_t *current_task;

int message_async_send(message_t *msg){
    user_task_t *stsk = user_find(msg->src);
    user_task_t *dtsk = user_find(msg->dst);
    if(dtsk == 0) return -1;    //TODO: ENOTFOUND
    if(stsk->msg_ack) return -2; //TODO: EREQACK

    tasking_disable();

    if(dtsk->recvbuf.count >= MSG_MAXCOUNT){
        list_push(dtsk->sendbuf, msg);
        user_block(msg->src);

        tasking_enable();
        tasking_tick();
    }
    message_push(&dtsk->recvbuf, msg);
    if(user_isblocked(msg->dst)) user_awake(msg->dst);

    tasking_enable();
    return 0;
}

message_t *message_async_recv(){
    tasking_disable();

    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;

    if(utsk->recvbuf.count == 0){
        user_block(utsk->pid);

        tasking_enable();
        tasking_tick(); 
    }

    message_t *msg = message_pop(&utsk->recvbuf);
    tasking_enable();

    return msg;
}

message_t *message_async_recv_poll(){
    tasking_disable();

    task_t *ctsk = current_task->data;
    user_task_t *utsk = ctsk->parent_struct;

    message_t *msg = message_pop(&utsk->recvbuf);
    tasking_enable();

    return msg;
}


int message_init(){
    return 0;
}

int message_fini(){
    return 0;
}

module_name(message);

module_load(message_init);
module_unload(message_fini);

module_depends(heap);
module_depends(dthelper);
module_depends(tasking);
module_depends(user);
