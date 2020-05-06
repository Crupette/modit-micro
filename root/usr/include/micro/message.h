#ifndef MESSAGE_H
#define MESSAGE_H

#include "api/message.h"

#include <stdint.h>
#include <stddef.h>

message_t micro_message_get(size_t s);
int micro_message_free(message_t *msg);

int micro_message_send(message_t *msg, uint32_t pid);
int micro_message_recv(message_t *msg);
int micro_message_poll(message_t *msg);

#endif
