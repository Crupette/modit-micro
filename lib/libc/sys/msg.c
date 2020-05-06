#include "sys/msg.h"
#include <stdlib.h>
#include <string.h>

#include "micro/message.h"

message_t *message_prepare(size_t s){
    message_t prepbuf = micro_message_get(s);
    prepbuf.msgsz = s;

    if(prepbuf.kdat == 0) return 0; //TODO: Set errno to -EINVAL
    message_t *r = malloc(sizeof(message_t));
    memcpy(r, &prepbuf, sizeof(message_t));
    return r;
}

int message_release(message_t *msg){
    if(msg == 0) return -1; //TODO: -EINVAL
    int r = micro_message_free(msg);
    
    free(msg);
    return r;
}

int message_send(message_t *msg, pid_t pid){
    if(msg == 0) return -1; //TODO: -EINVAL

    return micro_message_send(msg, pid);
}

message_t *message_recv(){
    message_t *msg = malloc(sizeof(message_t));
    memset(msg, 0, sizeof(message_t));

    if(micro_message_recv(msg) < 0){
        free(msg);
        return 0;
    }
    return msg;
}
