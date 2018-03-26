#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include "common.h"

typedef struct msg_type {
    uint16_t* data;        /* The data being sent in a message */
    MASK      mask;        /* The mask for the specific message being sent */
    PID       sender;      /* The sender of the message */
    PID       receiver;    /* The receiver of the message */
    struct msg_type* next;
} MSG;

typedef struct msg_queue_type {
    MSG *head;
    MSG *tail;
    uint8_t length;
} msg_queue_t;

msg_queue_t* msg_queue_init(msg_queue_t* list);

MSG* msg_peek (msg_queue_t* list);
MSG* msg_deque (msg_queue_t* list);
void msg_enqueue (msg_queue_t* list, MSG* msg);
MSG* msg_find_receiver (msg_queue_t* list, PID receiver, MASK mask);

#endif
