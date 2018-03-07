#include <stdint.h>
#include "kernel.h"
#include "os.h"

/**
 * The process descriptor of the currently RUNNING task.
 */
volatile static PD* Cp;

/**
 * This table contains ALL process descriptors. It doesn't matter what
 * state a task is in.
 */
// TODO: Replace with Priority based queues
static PD Process[MAXTHREAD];

task_queue_t system_tasks;
task_queue_t periodic_tasks;
task_queue_t rr_tasks;

/**
 * Since this is a "full-served" model, the kernel is executing using its own
 * stack. We can allocate a new workspace for this kernel stack, or we can
 * use the stack of the "main()" function, i.e., the initial C runtime stack.
 * (Note: This and the following stack pointers are used primarily by the
 *   context switching code, i.e., CSwitch(), which is written in assembly
 *   language.)
 */
volatile uint8_t* KernelSp;

/**
 * This is a "shadow" copy of the stack pointer of "Cp", the currently
 * running task. During context switching, we need to save and restore
 * it into the appropriate process descriptor.
 */
volatile uint8_t* CurrentSp;

/** index to next task to run */
volatile static uint16_t NextP;

/** 1 if kernel has been started; 0 otherwise. */
volatile static uint16_t KernelActive;

/** number of tasks created so far */
volatile static uint16_t Tasks;

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
#define CSwitch() Exit_Kernel()  /* Legacy CSwitch stub now uses Exit_Kernel */
extern void Exit_Kernel();

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

void Task_Terminate(void);

void Kernel_Task_Create_At(PD *p, voidfuncptr f) {
    uint8_t *sp = &(p->workSpace[WORKSPACE - 1]);

    //Clear the contents of the workspace
    ZeroMemory(p->workSpace, WORKSPACE);

    //Notice that we are placing the address (16-bit) of the functions
    //onto the stack in reverse byte order (least significant first, followed
    //by most significant).  This is because the "return" assembly instructions
    //(rtn and rti) pop addresses off in BIG ENDIAN (most sig. first, least sig.
    //second), even though the AT90 is LITTLE ENDIAN machine.

    //Store terminate at the bottom of stack to protect against stack underrun.
    *sp-- = LOW_BYTE(Task_Terminate);
    *sp-- = HIGH_BYTE(Task_Terminate);
    *sp-- = LOW_BYTE(0);

    //Place return address of function at bottom of stack
    *sp-- = LOW_BYTE(f);
    *sp-- = HIGH_BYTE(f);
    *sp-- = LOW_BYTE(0);
//Place stack pointer at top of stack
    sp = sp - 34;

    p->sp = sp;      /* stack pointer into the "workSpace" */
    p->code = f;     /* function to be executed as a task */
    p->request = NONE;
    p->state = READY;
}

void Kernel_Task_Create(voidfuncptr f) {
    int x;

    if (Tasks == MAXTHREAD) return;  /* Too many task! */

    /* find a DEAD PD that we can use  */
    for (x = 0; x < MAXTHREAD; x++) {
        if (Process[x].state == DEAD) break;
    }

    ++Tasks;
    Kernel_Task_Create_At( &(Process[x]), f );
}

/**
 * This internal kernel function is a part of the "scheduler". It chooses the
 * next task to run, i.e., Cp.
 */
static void Dispatch() {
    /* find the next READY task
     * Note: if there is no READY task, then this will loop forever!.
     */
    while(Process[NextP].state != READY) {
        NextP = (NextP + 1) % MAXTHREAD;
    }

    Cp = &(Process[NextP]);
    CurrentSp = Cp->sp;
    Cp->state = RUNNING;

    NextP = (NextP + 1) % MAXTHREAD;
}

static void Next_Kernel_Request() {
    Dispatch();  /* select a new task to run */

    while(1) {
        Cp->request = NONE; /* clear its request */

        /* activate this newly selected task */
        CurrentSp = Cp->sp;
        Exit_Kernel();    /* or CSwitch() */

        /* if this task makes a system call, it will return to here! */

        /* save the Cp's stack pointer */
        Cp->sp = CurrentSp;

        switch(Cp->request){
        case CREATE:
            Task_Create( Cp->code );
            break;
        case NEXT:
        case NONE:
            /* NONE could be caused by a timer interrupt */
            Cp->state = READY;
            Dispatch();
            break;
        case TERMINATE:
            /* deallocate all resources used by this task */
            Cp->state = DEAD;
            Dispatch();
            break;
        default:
            /* Houston! we have a problem here! */
            break;
        }
    }
}

/**
 * The calling task terminates itself.
 */
void Task_Terminate()
{
    if (KernelActive) {
        OS_DI();
        Cp->request = TERMINATE;
        Enter_Kernel();
        /* never returns here! */
    }
}

void Kernel_Init() {
    int x;

    Tasks = 0;
    KernelActive = 0;
    NextP = 0;

    //Reminder: Clear the memory for the task on creation.
    for (x = 0; x < MAXTHREAD; x++) {
        ZeroMemory(Process[x], sizeof(PD));
        Process[x].state = DEAD;
    }
}

void Kernel_Start() {
    if ( (! KernelActive) && (Tasks > 0)) {
        OS_DI();
        /* we may have to initialize the interrupt vector for Enter_Kernel() here. */

        /* here we go...  */
        KernelActive = 1;
        Next_Kernel_Request();
        /* NEVER RETURNS!!! */
    }
}

// THIS IS RUN IN USER MODE
void Kernel_Request(KERNEL_REQUEST_TYPE type) {
    if (KernelActive) {
        OS_DI();
        Cp->request = type;
        Enter_Kernel();
    }
}
