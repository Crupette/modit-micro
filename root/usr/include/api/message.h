/*  MESSAGE.H - Datatypes for data passing
 *
 *  Author: Crupette
 * */

#ifndef API_MESSAGE_H
#define API_MESSAGE_H 1

#include <stdint.h>

#define MSG_MAXCOUNT 128

typedef struct message {
    uint32_t dest;
    uint32_t src;
    uint32_t msgsz;
    uint8_t *msg;
} message_t;

typedef struct message_buffer {
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    message_t buffer[MSG_MAXCOUNT];
} message_buffer_t;

#endif
