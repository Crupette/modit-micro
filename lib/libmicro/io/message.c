#include "micro/message.h"
#include "api/syscall.h"

DEFN_SYSCALL2(msg_req, SYSCALL_MSG_REQ, size_t, message_t*)
DEFN_SYSCALL1(msg_free, SYSCALL_MSG_FREE, message_t*)

DEFN_SYSCALL2(msg_send, SYSCALL_MSG_SEND, message_t*, uint32_t)
DEFN_SYSCALL1(msg_recv, SYSCALL_MSG_RECV, message_t*)
DEFN_SYSCALL1(msg_poll, SYSCALL_MSG_POLL, message_t*)

message_t micro_message_get(size_t s){
    message_t r = { 0 };
    syscall_msg_req(s, &r);
    return r;
}

int micro_message_free(message_t *msg){
    return syscall_msg_free(msg);
}

int micro_message_send(message_t *msg, uint32_t pid){
    return syscall_msg_send(msg, pid);
}

int micro_message_recv(message_t *msg){
    return syscall_msg_recv(msg);
}

int micro_message_poll(message_t *msg){
    return syscall_msg_poll(msg);
}
