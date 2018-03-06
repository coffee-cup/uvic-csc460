#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h> // memset

/**
 * \file shared.c
 * \brief A Skeleton Implementation of an RTOS
 *
 * \mainpage A Skeleton Implementation of a "Self-Served" RTOS Model
 * This is an example of how to implement context-switching based on a
 * self-served model. That is, the RTOS is implemented by a collection of
 * user-callable functions. The kernel executes its functions using the calling
 * task's stack.
 *
 * \author Dr. Mantis Cheng
 * \date 2 October 2006
 *
 * ChangeLog: Modified by Alexander M. Hoole, October 2006.
 *			  -Rectified errors and enabled context switching.
 *			  -LED Testing code added for development (remove later).
 *
 * \section Implementation Note
 * This example uses the ATMEL AT90USB1287 instruction set as an example
 * for implementing the context switching mechanism.
 * This code is ready to be loaded onto an AT90USBKey.  Once loaded the
 * RTOS scheduling code will alternate lighting of the GREEN LED light on
 * LED D2 and D5 whenever the correspoing PING and PONG tasks are running.
 * (See the file "cswitch.S" for details.)
 */

#define WORKSPACE 256
#define MAXPROCESS 4

#define Disable_Interrupt() asm volatile("cli" ::)
#define Enable_Interrupt() asm volatile("sei" ::)
#define JUMP(f) asm("jmp " #f::)

#define BIT_SET(PORT, PIN) PORT |= 1 << PIN
#define BIT_CLR(PORT, PIN) PORT &= ~(1 << PIN)
#define BIT_FLIP(PORT, PIN) PORT ^= 1 << PIN

#define LOW_BYTE(X) (((uint16_t)X) & 0xFF)
#define HIGH_BYTE(X) ((((uint16_t)X) >> 8) & 0xFF)
#define ZeroMemory(X, N) memset(&(X), 0, N)
#define TASK(body)           \
    {                        \
        for (;; Task_Next()) \
        {                    \
            body             \
        }                    \
    }

/**
  *  This is the set of states that a task can be in at any given time.
  */
typedef enum process_states {
    DEAD = 0,
    READY,
    RUNNING
} PROCESS_STATES;

typedef enum process_requests {
    NONE = 0,
    YEILD,
    QUANTUM,
    TERMINATE
} PROCESS_REQUESTS;

/**
  * Each task is represented by a process descriptor, which contains all
  * relevant information about this task. For convenience, we also store
  * the task's stack, i.e., its workspace, in here.
  * To simplify our "CSwitch()" assembly code, which needs to access the
  * "sp" variable during context switching, "sp" MUST BE the first entry
  * in the ProcessDescriptor.
  * (See file "cswitch.S" for details.)
  */
typedef struct ProcessDescriptor
{
    uint8_t *sp;
    uint8_t workSpace[WORKSPACE];
    PROCESS_STATES state;
    PROCESS_REQUESTS request;
} PD;

typedef void (*fp_vv)(void); /* pointer to void f(void) */

/*===========
  * RTOS Internal
  *===========
  */

/**
  * This internal kernel function is the context switching mechanism.
  * Fundamentally, the CSwitch() function saves the current task CurrentP's
  * context, selects a new running task, and then restores the new CurrentP's
  * context.
  * (See file "switch.S" for details.)
  */
extern void Enter_Kernel(void);

/* Prototype */
void Task_Terminate(void);

/**
  * Exit_kernel() is used when OS_Start() or Task_Terminate() needs to
  * switch to a new running task.
  */
extern void Exit_Kernel(void);

/**
  * This table contains ALL process descriptors. It doesn't matter what
  * state a task is in.
  */
static PD Process[MAXPROCESS];

/**
  * The process descriptor of the currently RUNNING task.
  */
volatile PD* CurrentP;

/**
  * The process descriptor of the kernel.
  */
volatile uint8_t* KernelSP;

/** index to next task to run */
volatile static uint16_t NextP;

/** 1 if kernel has been started; 0 otherwise. */
volatile static uint16_t KernelActive;

/** number of tasks created so far */
volatile static uint16_t Tasks;

/** number of 10ms ticks occurred so far */
volatile static uint32_t num_ticks;

/**
 * When creating a new task, it is important to initialize its stack just like
 * it has called "Enter_Kernel()"; so that when we switch to it later, we
 * can just restore its execution context on its stack.
 * (See file "cswitch.S" for details.)
 */
void Kernel_Create_Task_At(PD *p, fp_vv f)
{
    uint8_t *sp = &(p->workSpace[WORKSPACE - 1]);

    //Initialize the workspace (i.e., stack) and PD here!

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

    p->sp = sp; /* stack pointer into the "workSpace" */
    p->state = READY;
    p->request = NONE;
}

/**
  *  Create a new task
  */
static void Kernel_Create_Task(fp_vv f)
{
    int16_t x;

    if (Tasks == MAXPROCESS)
        return; /* Too many task! */

    /* find a DEAD PD that we can use  */
    for (x = 0; x < MAXPROCESS; x++)
    {
        if (Process[x].state == DEAD)
            break;
    }

    ++Tasks;
    Kernel_Create_Task_At(&(Process[x]), f);
}

/**
  * This internal kernel function is a part of the "scheduler". It chooses the
  * next task to run, i.e., CurrentP.
  */
//Removed static because it was blocking external access from assembly file cswitch.S.
//We desire to see a 'T' not a 't' in the avr-nm output from the object file.
void Dispatch()
{
    /* find the next READY task
       * Note: if there is no READY task, then this will loop forever!.
       */
    while (Process[NextP].state != READY)
    {
        NextP = (NextP + 1) % MAXPROCESS;
    }

    /* we have a new CurrentP */
    CurrentP = &(Process[NextP]);
    CurrentP->state = RUNNING;

    //Moved to bottom (this was in the wrong place).
    NextP = (NextP + 1) % MAXPROCESS;
}

void Kernel_Event_nop(void) {
    Dispatch();
};

void Kernel_Event_yeild(void) {
    Dispatch();
};

void Kernel_Event_quantum(void) {
    Dispatch();
};

void Kernel_Event_terminate(void) {
    CurrentP->state = DEAD;
    Tasks -= 1;
    Dispatch();
};


fp_vv func_table[3][4] = {
/*              |     NONE         |      YEILD          |       QUANTUM         |      TERMINATE */
/* DEAD    */ {  Kernel_Event_nop,   Kernel_Event_yeild,   Kernel_Event_quantum,   Kernel_Event_terminate },
/* READY   */ {  Kernel_Event_nop,   Kernel_Event_yeild,   Kernel_Event_quantum,   Kernel_Event_terminate },
/* RUNNING */ {  Kernel_Event_nop,   Kernel_Event_yeild,   Kernel_Event_quantum,   Kernel_Event_terminate }
};

void Kernel_Event_Loop() {

    Dispatch();

    for (;;) {
        /* Clear the task's request */
        CurrentP->request = NONE;

        BIT_CLR(PORTD, 0);
        Exit_Kernel();
        /* When the task makes a system request execution will return here */

        func_table[CurrentP->state][CurrentP->request]();
    }
}

/*================
  * RTOS  API  and Stubs
  *================
  */

void OS_Configure_Timer() {
    //Clear timer config.
    TCCR4A = 0;
    TCCR4B = 0;
    //Set to CTC (mode 4)
    BIT_SET(TCCR4B, WGM42);

    //Set prescaller to 256
    BIT_SET(TCCR4B, CS42);

    //Set TOP value (0.01 seconds)
    OCR4A = 625;

    //Enable interupt A for timer 3.
    BIT_SET(TIMSK4, OCIE4A);

    //Set timer to 0 (optional here).
    TCNT4 = 0;
}


/**
  * This function initializes the RTOS and must be called before any other
  * system calls.
  */
void OS_Init()
{
    int16_t x;

    Tasks = 0;
    KernelActive = 0;
    NextP = 0;

    for (x = 0; x < MAXPROCESS; x++)
    {
        ZeroMemory(Process[x], sizeof(PD));
        Process[x].state = DEAD;
    }
}

/**
  * This function starts the RTOS after creating a few tasks.
  */
void OS_Start()
{
    if ((!KernelActive) && (Tasks > 0))
    {
        Disable_Interrupt();
        OS_Configure_Timer();
        /* here we go...  */
        KernelActive = 1;
        BIT_FLIP(PORTB, 0);
        BIT_FLIP(PORTB, 0);
        BIT_FLIP(PORTB, 0);
        BIT_FLIP(PORTB, 0);
        BIT_FLIP(PORTB, 0);
        BIT_FLIP(PORTB, 0);
        for (;;) {
            Kernel_Event_Loop();
        }
    }
}

/**
  * For this example, we only support cooperatively multitasking, i.e.,
  * each task gives up its share of the processor voluntarily by calling
  * Task_Next().
  */
void Task_Create(fp_vv f)
{
    Disable_Interrupt();
    Kernel_Create_Task(f);
    Enable_Interrupt();
}

/**
  * The calling task gives up its share of the processor voluntarily.
  */
void Task_Next()
{
    if (KernelActive)
    {
        Disable_Interrupt();
        CurrentP->state = READY;
        CurrentP->request = YEILD;

        BIT_SET(PORTD, 0);
        Enter_Kernel();
        /* resume here when this task is rescheduled again later */
        /* Interrupts will be re-enabled automatically */
    }
}

/**
  * The calling task terminates itself.
  */
void Task_Terminate()
{
    if (KernelActive)
    {
        Disable_Interrupt();
        CurrentP->request = TERMINATE;
        /* we will NEVER return here! */
        BIT_SET(PORTD, 0);
        Enter_Kernel();
    }
}

ISR(TIMER4_COMPA_vect)
{
    BIT_SET(PORTB, 6);
    if ((++num_ticks % 100) == 0){

        BIT_FLIP(PORTB, 7);
        CurrentP->request = QUANTUM;

        BIT_SET(PORTD, 0);
        Enter_Kernel();
    }
    BIT_CLR(PORTB, 6);
}

/**
  * A cooperative "Ping" task.
  * Added testing code for LEDs.
  */
void Ping() TASK
({
    //LED on
    BIT_FLIP(PORTB, 0);
    _delay_ms(100);
})

/**
 * A cooperative "Pong" task.
* Added testing code for LEDs.
*/
void Pong() TASK
({
    //LED off
    BIT_FLIP(PORTB, 1);
    _delay_ms(100);
})

/**
 * This function creates two cooperative tasks, "Ping" and "Pong". Both
* will run forever.
*/
int main(void)
{
    // io init
    DDRA = 0xFF;
    DDRB = 0xFF;
    DDRC = 0xFF;
    DDRD = 0xFF;

    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    PORTD = 0x00;

    BIT_SET(PORTB, 2);
    _delay_ms(37);
    BIT_CLR(PORTB, 2);

    num_ticks = 0;

    // Starting in 'kernel' mode
    BIT_FLIP(PORTD, 0);

    OS_Init();
    Kernel_Create_Task(Pong);
    Kernel_Create_Task(Ping);
    OS_Start();
}
