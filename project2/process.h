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
    volatile uint8_t*         sp;                   /* stack pointer into the "workSpace" */
    uint8_t                   workSpace[WORKSPACE];
    volatile PROCESS_STATE    state;
    taskfuncptr               code;                 /* function to be executed as a task  */
    int16_t                   arg;                  /* parameter to be passed to the task */
    PID                       process_id;
    PRIORITY_LEVEL            priority;
    TICK                      period;               /* The period of a PERIODIC task */
    TICK                      wcet;                 /* The worst case execution time of a PERIODIC task */
    TICK                      ttns;                 /* The time to next start for a PERIODIC task */
    TICK                      ticks_remaining;      /* Until a PERIODIC or RR task is forced to yield */
    struct ProcessDescriptor* next;
    volatile KERNEL_REQUEST_PARAMS *req_params;
} PD;

typedef struct task_queue_type {
    PD* head;
    PD* tail;
    uint8_t length;
    PRIORITY_LEVEL type;
} task_queue_t;

task_queue_t* queue_init(task_queue_t* list, PRIORITY_LEVEL type);

PD*  peek   (task_queue_t* list);
PD*  deque  (task_queue_t* list);
void enqueue(task_queue_t* list, PD* task);
void insert (task_queue_t* list, PD* task);

#endif
