#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <stdint.h>

#define WORKSPACE     256   // in bytes, per THREAD

typedef void (*voidfuncptr) (void);      /* pointer to void f(void) */

typedef enum priority_levels {
    SYSTEM = 0,
    PERIODIC,
    RR
} PRIORITY_LEVELS;

/**
 *  This is the set of states that a task can be in at any given time.
 */
typedef enum process_states {
     DEAD = 0,
     READY,
     RUNNING
} PROCESS_STATES;

/**
 * This is the set of kernel requests, i.e., a request code for each system call.
 */
typedef enum kernel_request_type {
     NONE = 0,
     CREATE,
     NEXT,
     TERMINATE
} KERNEL_REQUEST_TYPE;

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
