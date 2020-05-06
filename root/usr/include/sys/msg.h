#ifndef SYS_MSG_H
#define SYS_MSG_H 1

#include "sys/proc.h"
#include "api/message.h"

message_t *message_prepare(size_t s);
int message_release(message_t *msg);

int message_send(message_t *msg, pid_t pid);
message_t *message_recv();

#endif
