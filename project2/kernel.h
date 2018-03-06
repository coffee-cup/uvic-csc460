#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <stdint.h>
#include "os.h"

#define LOW_BYTE(X) (((uint16_t)X) & 0xFF)
#define HIGH_BYTE(X) ((((uint16_t)X) >> 8) & 0xFF)
#define ZeroMemory(X, N) memset(&X, 0, N)

typedef void (*voidfuncptr) (void);      /* pointer to void f(void) */

/**
 *  This is the set of states that a task can be in at any given time.
 */
typedef enum process_states
    {
     DEAD = 0,
     READY,
     RUNNING
    } PROCESS_STATES;

/**
 * This is the set of kernel requests, i.e., a request code for each system call.
 */
typedef enum kernel_request_type
    {
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
typedef struct ProcessDescriptor
    {
    unsigned char *sp;   /* stack pointer into the "workSpace" */
    unsigned char workSpace[WORKSPACE];
    PROCESS_STATES state;
    voidfuncptr  code;   /* function to be executed as a task */
    KERNEL_REQUEST_TYPE request;
    } PD;

/**
 * This internal kernel function is the context switching mechanism.
 * It is done in a "funny" way in that it consists two halves: the top half
 * is called "Exit_Kernel()", and the bottom half is called "Enter_Kernel()".
 * When kernel calls this function, it starts the top half (i.e., exit). Right in
 * the middle, "Cp" is activated; as a result, Cp is running and the kernel is
 * suspended in the middle of this function. When Cp makes a system call,
 * it enters the kernel via the Enter_Kernel() software interrupt into
 * the middle of this function, where the kernel was suspended.
 * After executing the bottom half, the context of Cp is saved and the context
 * of the kernel is restore. Hence, when this function returns, kernel is active
 * again, but Cp is not running any more.
 * (See file "cswitch.S" for details.)
 */
extern void CSwitch();
extern void Exit_Kernel();    /* this is the same as CSwitch() */

void Task_Terminate(void);

/**
 * This external function could be implemented in two ways:
 *  1) as an external function call, which is called by Kernel API call stubs;
 *  2) as an inline macro which maps the call into a "software interrupt";
 *       as for the AVR processor, we could use the external interrupt feature,
 *       i.e., INT0 pin.
 *  Note: Interrupts are assumed to be disabled upon calling Enter_Kernel().
 *     This is the case if it is implemented by software interrupt. However,
 *     as an external function call, it must be done explicitly. When Enter_Kernel()
 *     returns, then interrupts will be re-enabled by Enter_Kernel().
 */
extern void Enter_Kernel();

/**
 * This table contains ALL process descriptors. It doesn't matter what
 * state a task is in.
 */
static PD Process[MAXTHREAD];

/**
 * The process descriptor of the currently RUNNING task.
 */
volatile static PD* Cp;

/**
 * Since this is a "full-served" model, the kernel is executing using its own
 * stack. We can allocate a new workspace for this kernel stack, or we can
 * use the stack of the "main()" function, i.e., the initial C runtime stack.
 * (Note: This and the following stack pointers are used primarily by the
 *   context switching code, i.e., CSwitch(), which is written in assembly
 *   language.)
 */
volatile unsigned char *KernelSp;

/**
 * This is a "shadow" copy of the stack pointer of "Cp", the currently
 * running task. During context switching, we need to save and restore
 * it into the appropriate process descriptor.
 */
volatile unsigned char *CurrentSp;

/** index to next task to run */
volatile static unsigned int NextP;

/** 1 if kernel has been started; 0 otherwise. */
volatile static unsigned int KernelActive;

/** number of tasks created so far */
volatile static unsigned int Tasks;

/**
 * When creating a new task, it is important to initialize its stack just like
 * it has called "Enter_Kernel()"; so that when we switch to it later, we
 * can just restore its execution context on its stack.
 * (See file "cswitch.S" for details.)
 */
void Kernel_Create_Task_At(PD *p, voidfuncptr f);

/**
 *  Create a new task
 */
static void Kernel_Create_Task(voidfuncptr f);

/**
 * This internal kernel function is a part of the "scheduler". It chooses the
 * next task to run, i.e., Cp.
 */
static void Dispatch();

/**
 * This internal kernel function is the "main" driving loop of this full-served
 * model architecture. Basically, on OS_Start(), the kernel repeatedly
 * requests the next user task's next system call and then invokes the
 * corresponding kernel function on its behalf.
 *
 * This is the main loop of our kernel, called by OS_Start().
 */
static void Next_Kernel_Request();

#endif
