#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "common.h"
#include "process.h"
#include "kernel.h"
#include "os.h"

/*
 * inline assembly code to disable/enable maskable interrupts
 * (N.B. Use with caution.)
 */
#define OS_DI()    asm volatile("cli"::)     /* disable all interrupts */
#define OS_EI()    asm volatile("sei"::)     /* enable all interrupts */
#define OS_JUMP(f) asm volatile("jmp " #f::) /* direct jump to assembly label */

#define VALID_ID(id) (id >= 0 && id < MAXTHREAD)

typedef void (*request_handler_func) (void);

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
static PD IdleProcess;

task_queue_t system_tasks;
task_queue_t periodic_tasks;
task_queue_t rr_tasks;

/**
 * This array represents an outgoing mailbox.
 * If Process[i] is in the SEND_BLOCK state then Messages[i] will be the message
 * it is trying to send.
 */
static MSG Messages[MAXTHREAD];

/**
 * This message queue stores the order of messages that a process is trying to send.
 * If a process is trying to send a message, the message will be in this queue.
 * This queue is needed in addition to the Messages array so that the order of outgoing messages
 * is first come first serve. When a process receives a message, it will receive the first message sent to it.
 */
static msg_queue_t msg_queue;

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

/** state of system clock overflow */
volatile static OVERFLOW_STATE clock_state;

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
extern void create(void);

ISR(TIMER4_COMPA_vect) {
    if (KernelActive) {

        // Timer pin
        BIT_FLIP(PORTB, 6);

        if (request_info != NULL) {
            OS_Abort(INVALID_REQ_INFO);
        }

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

    p->next = NULL;
    p->req_params = NULL;
}

void Kernel_Task_Create() {
    if (Tasks >= MAXTHREAD) {
        /* Too many tasks! */
        /* Do not OS Abort because this error should be recoverable according to spec */
        LOG("WARN: Too many task created\n");
        return;
    }

    if (request_info->code == NULL) {
        OS_Abort(NULL_TASK_FUNCTION);
        return;
    }

    /* find a DEAD PD that we can use  */
    int x;
    for (x = 0; x < MAXTHREAD; x++) {
        if (Process[x].state == DEAD) {
            ZeroMemory(Process[x], sizeof(PD));
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

            Process[x].ticks_remaining = 1;

            enqueue(&rr_tasks, &Process[x]);

        } else if (Process[x].priority == PERIODIC) {

            if (request_info->period > 0 && request_info->wcet < request_info->period) {
                Process[x].period = request_info->period;
                Process[x].wcet = request_info->wcet;
                Process[x].ttns = sys_clock + request_info->offset;
                Process[x].ticks_remaining = Process[x].wcet;
                Process[x].clockState = clock_state;

                insert(&periodic_tasks, &Process[x]);
            } else {
                OS_Abort(INVALID_REQ_INFO);
            }

        } else {
            OS_Abort(INVALID_REQ_INFO);
            return;
        }

        request_info->out_pid = Process[x].process_id = x;
        Tasks += 1;
    } else {
        /* Couldn't find a dead task */
        OS_Abort(NO_DEAD_PROCESS);
        return;
    }
}

/**
 * Finds and returns the first READY task in `queue`.
 * If no task is found, returns NULL and queue order is unchanged
 * If a task is found, `queue` will be rotated so that the READY task is
 * at the front.
 */
PD* Queue_Rotate_Ready(task_queue_t* queue) {
    // Get candidate task and first task in queue
    PD* iter_task;
    PD* first;
    first = iter_task = peek(queue);


    // do-while so that the first loop doesn't break since first == new_p
    do {
        // Specify that the task has to be ready
        if (iter_task->state != READY) {
            // Move non-ready tasks to the end of the queue
            // If this is a periodic queue, use insert
            if (queue->type == PERIODIC) {
                insert(queue, deque(queue));
            } else {
                enqueue(queue, deque(queue));
            }

            // Get the new head task
            iter_task = peek(queue);
        } else {
            // task state is ready!
            break;
        }
    } // Loop until the first task comes up again
    while (iter_task != first);

    if (iter_task->state != READY) {
        // Didn't find anything that was ready
        iter_task = NULL;
    }

    return iter_task;
}

/**
 * This internal kernel function is a part of the "scheduler". It chooses the
 * next task to run, i.e., Cp.
 */
static void Dispatch() {
    /* Move the current task to the end of it's queue */
    /* We use the invatiant that the running task is at the front of it's queue */
    switch (Cp->priority) {
        case SYSTEM:
            if (system_tasks.length > 1) {
                enqueue(&system_tasks, deque(&system_tasks));
            }
            break;

        case PERIODIC:
            if (periodic_tasks.length > 1) {
                insert(&periodic_tasks, deque(&periodic_tasks));
            }
            break;

        case RR:
            if (rr_tasks.length > 1) {
                enqueue(&rr_tasks, deque(&rr_tasks));
            }
            break;

        default:
            // Could have Cp == IdleProcess, in which case the priority isn't a normal value
            // Only abort if the CP isn't the idle process
            if (Cp != &IdleProcess) {
                OS_Abort(INVALID_PRIORITY);
                return;
            }
        break;
    }


    /* Only change the current task if it's not running */
    if (Cp->state != RUNNING ) {
        PD* new_p = NULL;

        /* Check the system tasks */
        if (system_tasks.length > 0) {
            new_p = Queue_Rotate_Ready(&system_tasks);
        }

        /* Haven't found a new task */
        if (new_p == NULL) {

            /* Check for periodic tasks which are ready to run, assuming sorted order
               based on increasing time to next start, only need to check first task */
            if (periodic_tasks.length > 0
                && sys_clock >= peek(&periodic_tasks)->ttns
                && clock_state == peek(&periodic_tasks)->clockState) {
                new_p = Queue_Rotate_Ready(&periodic_tasks);

                if (new_p != NULL && sys_clock > new_p->ttns && clock_state == new_p->clockState) {
                    // A periodic task must be run on its period
                    OS_Abort(TIMING_VIOLATION);
                    return;
                }
            }

            /* No periodic tasks should be started, check round robin tasks now */
            else if (rr_tasks.length > 0) {
                /* Check all the round robin tasks */
                new_p = Queue_Rotate_Ready(&rr_tasks);
            }
        }

        /* Nothing is ready to run! Use our lower-than-low priority task */
        if (new_p == NULL) {
            new_p = &IdleProcess;
        }

        /* Finally set the new task */
        Cp = new_p;
    }

    CurrentSp = Cp->sp;
    Cp->state = RUNNING;
}

void Kernel_Request_Create() {
    Kernel_Task_Create();

    /* Dispatch because created task might be higher priority then the currently running task */
    Dispatch();
}

void Kernel_Request_Next() {
    TICK prev_ttns;
    switch (Cp->priority) {
        case SYSTEM:
            // System task yielded, nothing to do
        break;

        case PERIODIC:
            // The task yieleded, make it ready for next time
            prev_ttns = Cp->ttns;
    		Cp->ttns += Cp->period;
	        Cp->ticks_remaining = Cp->wcet;

            // Overflow
            if (Cp->ttns < prev_ttns) {
                Cp->clockState = Cp->clockState == EVEN ? ODD : EVEN;
            }
        break;

        case RR:
            // Round robin task yielded, give it another shot at execution
            Cp->ticks_remaining = 1;
        break;

        default:
            // Always abort, Idle Process doesn't yield
            OS_Abort(INVALID_PRIORITY);
            return;
            break;
    }

    // If the task wasn't blocked, make it ready
    if (Cp->state != SEND_BLOCK && Cp->state != REPLY_BLOCK && Cp->state != RECV_BLOCK) {
        Cp->state = READY;
    }

    // Dispatch to another task
    Dispatch();
}

void Kernel_Request_Abort() {
    /* Disable system clock by setting prescaler to 0 */
    MASK_CLR(TCCR4B, 0b111);

    /* Blink the built-in LED in accordance with the error code */
    BIT_SET(DDRB, 7); /* Set PB7 as output */
    BIT_CLR(PORTB, 7);
    uint8_t ctr;

    for(;;) {
        LOG("OS Abort. Error code: %d\n", request_info->abort_code);
        for (ctr = 0; ctr < request_info->abort_code; ctr += 1) {
            BIT_SET(PORTB, 7);
            _delay_ms(200);
            BIT_CLR(PORTB, 7);
            _delay_ms(200);
        }
        _delay_ms(1000);
    }
}

void Kernel_Request_Terminate() {
    /* deallocate all resources used by this task */
    /* Assume it will be at the front of it's queue? */
    PD* killed_task = NULL;
    switch (Cp->priority) {
        case SYSTEM:
            killed_task = deque(&system_tasks);
            break;
        case PERIODIC:
            killed_task = deque(&periodic_tasks);
            break;
        case RR:
            killed_task = deque(&rr_tasks);
            break;
        default:
            OS_Abort(INVALID_PRIORITY);
            return;
            break;
    }

    if (killed_task != Cp) {
        OS_Abort(WRONG_TASK_ORDER);
        return;
    }

    // Remove any messages being sent to this process
    MSG *msg = msg_find_receiver(&msg_queue, Cp->process_id, ANY);
    while (msg != NULL) {
        msg = msg_find_receiver(&msg_queue, Cp->process_id, ANY);
    }

    Cp->state = DEAD;
    Tasks -= 1;
    Dispatch();
}

void Kernel_Request_MsgSend() {
    // Periodic tasks cannot send
    if (Cp->priority == PERIODIC) {
        OS_Abort(PERIODIC_MSG);
        return;
    }

    // Check if process id is valid
    if (!VALID_ID(request_info->msg_to)) {
        /* Do not OS Abort because sending a message to an invalid process is recoverable */
        return;
    }

    PD *p_recv = &Process[request_info->msg_to];
    MTYPE recv_mask = p_recv->req_params->msg_mask;

    // If sending to non-existent process, noop
    if (p_recv->state == DEAD) {
        return;
    }

    // Check if info.msg_to is waiting for a message of same type
    if (p_recv->state == RECV_BLOCK && MASK_TEST_ANY(recv_mask, request_info->msg_mask)) {
        // If yes, change state of waiting process to ready and sender to reply block
        p_recv->state = READY;
        Cp->state = REPLY_BLOCK;

        // Add the message data and pid of sender to the receiving processes request info
        p_recv->req_params->msg_ptr_data = request_info->msg_ptr_data;
        p_recv->req_params->out_pid = Cp->process_id;
    } else {
        // If not, sender process goes to send block state
        MSG *msg = &Messages[Cp->process_id];

        // Save message
        msg->data = request_info->msg_ptr_data;
        msg->mask = request_info->msg_mask;
        msg->receiver = request_info->msg_to;
        msg->sender = Cp->process_id;

        // Add message to message queue
        msg_enqueue(&msg_queue, msg);

        Cp->state = SEND_BLOCK;
    }

    Dispatch();
}

void Kernel_Request_MsgRecv() {
    // Periodic tasks cannot receive
    if (Cp->priority == PERIODIC) {
        OS_Abort(PERIODIC_MSG);
        return;
    }

    // Get the first message that was sent to this process
    MSG *msg = msg_find_receiver(&msg_queue, Cp->process_id, request_info->msg_mask);

    // Check if there is a message waiting for this process
    if (msg != NULL) {
        // If yes, change state to ready and set msg data on Cp's request info
        request_info->msg_ptr_data = msg->data;
        request_info->out_pid = msg->sender;

        Cp->state = READY;

        // Sender process now waiting for reply
        PD *sender = &Process[msg->sender];
        sender->state = REPLY_BLOCK;

        // Remove data from Messages
        msg->data = NULL;
        msg->receiver = -1;
        msg->sender = -1;
    } else {
        // If not, set process to receive block state
        Cp->state = RECV_BLOCK;
    }

    Dispatch();
}

void Kernel_Request_MsgRply() {
    // Periodic tasks cannot reply
    if (Cp->priority == PERIODIC) {
        OS_Abort(PERIODIC_MSG);
        return;
    }

    // Check if process id is valid
    if (!VALID_ID(request_info->msg_to)) {
        /* Do not OS Abort because replying to an invalid process is recoverable */
        return;
    }

    PD *p_recv = &Process[request_info->msg_to];

    // If replying to non-existent process, noop
    if (p_recv->state == DEAD) {
        return;
    }

    // Check if process replying to is in reply block state
    if (p_recv->state == REPLY_BLOCK) {
        p_recv->state = READY;

        p_recv->req_params->msg_data = request_info->msg_data;
    } else {
        // If not, noop
    }

    Cp->state = READY;

    Dispatch();
}

void Kernel_Request_MsgASend() {
    // Check if process id is valid
    if (!VALID_ID(request_info->msg_to)) {
        OS_Abort(INVALID_REQ_INFO);
        return;
    }

    PD *p_recv = &Process[request_info->msg_to];
    MTYPE recv_mask = p_recv->req_params->msg_mask;

    // If sending to non-existent process, noop
    if (p_recv->state == DEAD) {
        return;
    }

    // Check if info.msg_to is waiting for a message of same type
    if (p_recv->state == RECV_BLOCK && MASK_TEST_ANY(recv_mask, request_info->msg_mask)) {
        // If yes, change state of waiting process to ready and sender to reply block
        p_recv->state = READY;

        // Add the message data and pid of sender to the receiving processes request info
        p_recv->req_params->msg_data = request_info->msg_data;

        // Since the sender passes data by value, we need to set the data ptr to NULL
        // so the receiver knows to look at the msg_data instead
        p_recv->req_params->msg_ptr_data = NULL;
        p_recv->req_params->out_pid = Cp->process_id;

        // Dispatch because awaiting process might be higher priority
        Dispatch();
    } else {
        // If not, noop
    }
}

void Kernel_Request_Timer() {
    switch (Cp->priority) {
        case SYSTEM:
            // Tick ended during system task

        break;

        case PERIODIC:
            // Tick ended during periodic task
    		Cp->ticks_remaining -= 1;
            if (Cp->ticks_remaining <= 0) {
                // Task ran over it's worst case execution time
                OS_Abort(TIMING_VIOLATION);
                return;
            }
        break;

        case RR:
            // Tick ended during round robin task
            Cp->ticks_remaining -= 1;
            if (Cp->ticks_remaining <= 0) {
                // Let it go again
                Cp->ticks_remaining = 1;
            }
        break;

        default:
            // Timer can interrupt the idle process
            if (Cp != &IdleProcess) {
                OS_Abort(INVALID_PRIORITY);
                return;
            }
        break;
    }

    // Clock ticked, increment the value
    TICK prev_clock = sys_clock;
    sys_clock += 1;

    // Detect clock overflow
    if (sys_clock < prev_clock) {
        clock_state = clock_state == EVEN ? ODD : EVEN;
    }

    // You were running before the tick, so you're ready now
    Cp->state = READY;

    // Do not dispatch for periodic
    if (Cp->priority != PERIODIC) {
        Dispatch();
    }
}

void Kernel_Request_GetArg() {
    request_info->arg = Cp->arg;
}

void Kernel_Request_GetPid() {
    request_info->out_pid = Cp->process_id;
}

void Kernel_Request_GetNow() {
    request_info->out_now = sys_clock;
}

void Kernel_Request_None() {
    ;
}


static void Next_Kernel_Request() {
    request_handler_func request_handlers[NUM_KERNEL_REQUEST_TYPES] = {
        // Must be in order of KERNEL_REQUEST_TYPE
        Kernel_Request_None,
        Kernel_Request_Timer,
        Kernel_Request_Create,
        Kernel_Request_Next,
        Kernel_Request_GetArg,
        Kernel_Request_GetPid,
        Kernel_Request_GetNow,
        Kernel_Request_MsgSend,
        Kernel_Request_MsgRecv,
        Kernel_Request_MsgRply,
        Kernel_Request_MsgASend,
        Kernel_Request_Terminate,
        Kernel_Request_Abort
    };

    Dispatch();  /* select a new task to run */

    for(;;) {
        if (request_info) {
            // Clear the caller's request type
            request_info->request = NONE;
            // Clear our reference to request_info
            request_info = NULL;
        } else {
            if (KernelActive) {
                OS_Abort(NO_REQUEST_INFO);
                return;
            }
        }

        /* activate this newly selected task */
        CurrentSp = Cp->sp;
        KernelActive = 1;
        Exit_Kernel();    /* or CSwitch() */

        /* if this task makes a kernel request, it will return to here! */
        /* request_info should be valid again! */
        if (!request_info) {
            OS_Abort(NO_REQUEST_INFO);
            return;
        }

        /* save the Cp's stack pointer */
        Cp->sp = CurrentSp;

        /* Switch current process state from RUNNING to READY */
        Cp->state = READY;


        /* Run the approrpriate handler */
        if (request_info->request >= NONE && request_info->request < NUM_KERNEL_REQUEST_TYPES) {
            /* It's up to the handler to decide if it should dispatch */
            request_handlers[request_info->request]();
        } else {
            OS_Abort(INVALID_REQ_INFO);
            return;
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

void Kernel_idle() {
    for(;;) {
        // Kernel idle pin
        BIT_FLIP(PORTD, 0);
    }
}

void Kernel_Init() {
    int x;

    Tasks = 0;
    KernelActive = 0;
    NextP = 0;
    sys_clock = 0;
    clock_state = EVEN;

    Kernel_Init_Clock();

    // The onboard LED is reserved for OS_Abort
    BIT_SET(DDRB, 7);
    BIT_CLR(PORTB, 7);

    // Clear the memory for the IdleProcess
    ZeroMemory(IdleProcess, sizeof(PD));
    Kernel_Task_Create_At(&IdleProcess, Kernel_idle);

    // This process is not of normal priority
    // No outsiders can see or modify this task
    // So we should be okay to set this to a non PRIORITY_LEVEL enum
    IdleProcess.priority = -1;

    // Reminder: Clear the memory for the task on creation.
    for (x = 0; x < MAXTHREAD; x++) {
        ZeroMemory(Process[x], sizeof(PD));
        Process[x].state = DEAD;
        Process[x].clockState = clock_state;

        ZeroMemory(Messages[x], sizeof(MSG));
        Messages[x].data = NULL;
        Messages[x].receiver = -1;
        Messages[x].sender = -1;
        Messages[x].mask = 0x00;
    }

    queue_init(&system_tasks, SYSTEM);
    queue_init(&rr_tasks, RR);
    queue_init(&periodic_tasks, PERIODIC);

    msg_queue_init(&msg_queue);

    // Add the setup system task
    KERNEL_REQUEST_PARAMS info = {
        .request = CREATE,
        .priority = SYSTEM,
        .code = create,
        .arg = 0
    };

    request_info = &info;
    Kernel_Task_Create();
    request_info = NULL;
}

void Kernel_Start() {
    if ( (! KernelActive) && (Tasks > 0)) {
        OS_DI();
        /* we may have to initialize the interrupt vector for Enter_Kernel() here. */

        /* here we go...  */
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

        // Save pointer to current processes request info so we can
        // return data to process after it has been cswitched out
        Cp->req_params = info;
        Enter_Kernel();
    }
}

int Kernel_Begin(void) {
    // Clock pin
    BIT_SET(DDRB, 6);
    BIT_SET(PORTB, 6);

    // Debug pins
    BIT_SET(DDRD, 0);
    BIT_SET(DDRD, 1);
    BIT_CLR(PORTD, 0);
    BIT_CLR(PORTD, 1);

    Kernel_Init();
    /* Can't add tasks here since Kernel_Request doesn't return until KernelActive is truthy */
    Kernel_Start();

    /* Control should never reach this point */
    OS_Abort(FAILED_START);

    return -1;
}
