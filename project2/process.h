#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "common.h"
#include <stdint.h>

/**
 * Each task is represented by a process descriptor, which contains all
 * relevant information about this task. For convenience, we also store
 * the task's stack, i.e., its workspace, in here.
 */
typedef struct ProcessDescriptor {
    unsigned char *sp;   /* stack pointer into the "workSpace" */
    unsigned char workSpace[WORKSPACE];
    PROCESS_STATES state;
    voidfuncptr  code;   /* function to be executed as a task */
    KERNEL_REQUEST_TYPE request;
} PD;

typedef struct task_queue_type {
    PD *head;
    PD *tail;
    uint8_t length;
} task_queue_t;

task_queue_t *queue_init();
void enqueue(task_queue_t *list, PD * task);
PD *deque(task_queue_t *list);
PD *peek(task_queue_t *list);

#endif
