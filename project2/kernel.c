#include <stdint.h>
#include <avr/interrupt.h>
#include "kernel.h"
#include "os.h"

/**
 * The process descriptor of the currently RUNNING task.
 */
volatile static PD* Cp;
volatile static KERNEL_REQUEST_PARAMS* request_info;
/**
 * This table contains ALL process descriptors. It doesn't matter what
 * state a task is in. This table also serves as storage for the elements of
 * the task_queues. A task queue keeps track of the priority it manages, and won't enqueue
 * tasks that have the wrong priority. Doing it this way saves memory by allowing all three
 * queues to use the same storage, with the assurance that they won't overwrite eachother.
 */
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

/** number of ticks elapsed since boot */
volatile static TICK sys_clock;

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

/* User level 'main' function */
extern void setup(void);


ISR(TIMER4_COMPA_vect)
{
    if (KernelActive) {
        sys_clock += 1;
        KERNEL_REQUEST_PARAMS info = {
            .request = TIMER
        };

        request_info = &info;

        // This timer interrupts user mode programs
        // Need to make sure to switch modes
        Enter_Kernel();
    }
}

void Kernel_Task_Create_At(PD *p, taskfuncptr f) {
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
    p->state = READY;
}

void Kernel_Task_Create() {
    if (Tasks >= MAXTHREAD) {
        /* Too many tasks! */
        return;
    }

    /* find a DEAD PD that we can use  */
    int x;
    for (x = 0; x < MAXTHREAD; x++) {
        if (Process[x].state == DEAD) {
            break;
        }
    }

    /* Create the new task at dead process x.
     * Should have one since Tasks < MAXTHREAD */
    if (x < MAXTHREAD) {
        Kernel_Task_Create_At( &(Process[x]), request_info->code );

        Process[x].priority = request_info->priority;
        Process[x].arg = request_info->arg;

        if (Process[x].priority == SYSTEM) {

            enqueue(&system_tasks, &Process[x]);

        } else if (Process[x].priority == RR) {

            enqueue(&rr_tasks, &Process[x]);

        } else if (Process[x].priority == PERIODIC) {

            insert(&periodic_tasks, &Process[x]);

        } else {
            OS_Abort(INVALID_REQ_INFO);
        }

        request_info->out_pid = Process[x].process_id = x;
        Tasks += 1;
    } else {
        /* Couldn't find a dead task */
        OS_Abort(NO_DEAD_PROCESS);
    }
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
        request_info->request = NONE; /* clear its request */

        /* activate this newly selected task */
        CurrentSp = Cp->sp;
        Exit_Kernel();    /* or CSwitch() */

        /* if this task makes a kernel request, it will return to here! */

        /* save the Cp's stack pointer */
        Cp->sp = CurrentSp;

        switch(request_info->request){
            case CREATE:
                Kernel_Task_Create();
                break;

            case NEXT:
                Cp->state = READY;
                Dispatch();
                break;

            case TERMINATE:
                /* deallocate all resources used by this task */
                Cp->state = DEAD;
                Dispatch();
                break;

            case TIMER:
                sys_clock += 1;
                Cp->state = READY;
                Dispatch();
                break;

            case GET_ARG:
                request_info->arg = Cp->arg;
                break;

            case GET_PID:
                request_info->out_pid = Cp->process_id;
                break;

            case GET_NOW:
                request_info->out_now = sys_clock;
                break;

            case NONE:
            default:
                /* Houston! we have a problem here! */
                break;
        }
    }
}

void Kernel_Init_Clock() {

    // Clear timer config.
    TCCR4A = 0;
    TCCR4B = 0;

    // Set to CTC (mode 4)
    BIT_SET(TCCR4B, WGM42);

    // Set prescaller to 256
    BIT_SET(TCCR4B, CS42);

    // Set TOP value (0.01 seconds)
    // TODO: Adjust this based on MSECPERTICK definition
    OCR4A = 625;

    // Enable interupt A for timer 4.
    BIT_SET(TIMSK4, OCIE4A);

    // Set timer to 0 (optional here).
    TCNT4 = 0;
}

void Kernel_Init() {
    int x;

    Tasks = 0;
    KernelActive = 0;
    NextP = 0;
    sys_clock = 0;

    Kernel_Init_Clock();

    //Reminder: Clear the memory for the task on creation.
    for (x = 0; x < MAXTHREAD; x++) {
        ZeroMemory(Process[x], sizeof(PD));
        Process[x].state = DEAD;
    }

    queue_init(&system_tasks, SYSTEM);
    queue_init(&rr_tasks, RR);
    queue_init(&periodic_tasks, PERIODIC);

    // Add the setup system task
    KERNEL_REQUEST_PARAMS info = {
        .request = CREATE,
        .priority = SYSTEM,
        .code = setup,
        .arg = 0
    };

    request_info = &info;
    Kernel_Task_Create();
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
void Kernel_Request(KERNEL_REQUEST_PARAMS* info) {
    if (KernelActive) {
        OS_DI();

        // No race condition here since interrupts are disabled
        request_info = info;
        Enter_Kernel();
    }
}
