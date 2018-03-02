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

#define bit(b) ((uint16_t)(1 << ((uint16_t)b)))
#define LOW_BYTE(X) (((uint16_t)X) & 0xFF)
#define HIGH_BYTE(X) ((((uint16_t)X) >> 8) & 0xFF)
#define ZeroMemory(X, N) memset(&X, 0, N)
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
extern void CSwitch(void);

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
//??? Removed static because it was blocking external access.
volatile PD *CurrentP;

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

/*================
  * RTOS  API  and Stubs
  *================
  */

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
        memset(&(Process[x]), 0, sizeof(PD));
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

        /* here we go...  */
        KernelActive = 1;
        JUMP(Exit_Kernel);
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
        CSwitch();
        /* resume here when this task is rescheduled again later */
        Enable_Interrupt();
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
        CurrentP->state = DEAD;
        /* we will NEVER return here! */
        JUMP(Exit_Kernel);
    }
}

void setupTimer()
{
    //Clear timer config.
    TCCR4A = 0;
    TCCR4B = 0;
    //Set to CTC (mode 4)
    TCCR4B |= (1 << WGM42);

    //Set prescaller to 256
    TCCR4B |= (1 << CS42);

    //Set TOP value (0.01 seconds)
    OCR4A = 625;

    //Enable interupt A for timer 3.
    TIMSK4 |= (1 << OCIE4A);

    //Set timer to 0 (optional here).
    TCNT4 = 0;

    Enable_Interrupt();
}

ISR(TIMER4_COMPA_vect)
{
    PORTB |= bit(6);
    if ((num_ticks++ % 100) == 0)
    {
        PORTB ^= bit(7);
    }
    PORTB &= ~bit(6);
}

/**
  * A cooperative "Ping" task.
  * Added testing code for LEDs.
  */
void Ping() TASK
({
    PORTD ^= bit(0);
    //LED on
    PORTB |= bit(0);
    _delay_ms(300);

    PORTD ^= bit(0);
})

/**
 * A cooperative "Pong" task.
* Added testing code for LEDs.
*/
void Pong() TASK
({
    PORTD ^= bit(0);
    //LED off
    PORTB &= ~bit(1);
    _delay_ms(300);

    PORTD ^= bit(0);
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
    PORTB = bit(2);
    PORTC = 0x00;
    PORTD = 0x00;

    _delay_ms(3000);

    PORTB = 0x00;
    num_ticks = 0;

    setupTimer();

    OS_Init();
    Task_Create(Pong);
    Task_Create(Ping);
    OS_Start();
}
