#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <stdint.h>
#include "common.h"

/**
 * Each task is represented by a process descriptor, which contains all
 * relevant information about this task. For convenience, we also store
 * the task's stack, i.e., its workspace, in here.
 */
typedef struct ProcessDescriptor {
    volatile uint8_t*       sp;                   /* stack pointer into the "workSpace" */
    uint8_t                 workSpace[WORKSPACE];
    volatile PROCESS_STATES state;
    voidfuncptr             code;                 /* function to be executed as a task */
    KERNEL_REQUEST_TYPE     request;
} PD;

typedef struct task_queue_type {
    PD* head;
    PD* tail;
    uint8_t length;
} task_queue_t;

void queue_init(task_queue_t* list);

PD*  peek   (task_queue_t* list);
PD*  deque  (task_queue_t* list);
void enqueue(task_queue_t* list, PD* task);

#endif
