/*  MESSAGE.H - Datatypes for data passing
 *
 *  Author: Crupette
 * */

#ifndef API_MESSAGE_H
#define API_MESSAGE_H 1

#include <stdint.h>

#define MSG_MAXCOUNT 128
#define MSG_MAXSZ 0xFF000

typedef struct message {
    uint32_t dst;
    uint32_t src;
    uint32_t msgsz;

    uint8_t *dat;
    uint8_t *kdat;
} message_t;

typedef struct message_buffer {
    int16_t head;
    int16_t tail;
    uint16_t count;
    message_t *buffer[MSG_MAXCOUNT];
} message_buffer_t;

#ifdef KERNEL 

int message_push(message_buffer_t *buf, message_t *msg);
message_t *message_pop(message_buffer_t *buf);
message_t *message_peep(message_buffer_t *buf);

int message_async_send(message_t *msg);
message_t *message_async_recv();
message_t *message_async_recv_poll();

#endif

#endif
