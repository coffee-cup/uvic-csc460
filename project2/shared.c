#include <string.h>
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

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

//Comment out the following line to remove debugging code from compiled version.
#define DEBUG

typedef void (*voidfuncptr) (void);      /* pointer to void f(void) */

#define WORKSPACE     256
#define MAXPROCESS   4


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
extern void CSwitch();

/* Prototype */
void Task_Terminate(void);

/**
  * Exit_kernel() is used when OS_Start() or Task_Terminate() needs to
  * switch to a new running task.
  */
extern void Exit_Kernel();

#define Disable_Interrupt()         asm volatile ("cli"::)
#define Enable_Interrupt()          asm volatile ("sei"::)
#define JUMP(f) asm("jmp " #f::)

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

/**
  * This table contains ALL process descriptors. It doesn't matter what
  * state a task is in.
  */
static PD Process[MAXPROCESS];

/**
  * The process descriptor of the currently RUNNING task.
  */
  //??? Removed static because it was blocking external access.
  //??? Rename Cp to CurrentP because 'cp' is reserved in assembly.
volatile PD* CurrentP;

/** index to next task to run */
volatile static uint16_t NextP;

/** 1 if kernel has been started; 0 otherwise. */
volatile static uint16_t KernelActive;

/** number of tasks created so far */
volatile static uint16_t Tasks;


/**
 * When creating a new task, it is important to initialize its stack just like
 * it has called "Enter_Kernel()"; so that when we switch to it later, we
 * can just restore its execution context on its stack.
 * (See file "cswitch.S" for details.)
 */
void Kernel_Create_Task_At( PD *p, voidfuncptr f )
{
   uint8_t *sp;


   sp = (uint8_t *) &(p->workSpace[WORKSPACE-1]);

   /*----BEGIN of NEW CODE----*/
   //Initialize the workspace (i.e., stack) and PD here!

   //Clear the contents of the workspace
   memset(&(p->workSpace),0,WORKSPACE);

   //Notice that we are placing the address (16-bit) of the functions
   //onto the stack in reverse byte order (least significant first, followed
   //by most significant).  This is because the "return" assembly instructions
   //(rtn and rti) pop addresses off in BIG ENDIAN (most sig. first, least sig.
   //second), even though the AT90 is LITTLE ENDIAN machine.

   //Store terminate at the bottom of stack to protect against stack underrun.
   *(uint8_t *)sp-- = ((uint16_t)Task_Terminate) & 0xff;
   *(uint8_t *)sp-- = (((uint16_t)Task_Terminate) >> 8) & 0xff;

   //Place return address of function at bottom of stack
   *(uint8_t *)sp-- = ((uint16_t)f) & 0xff;
   *(uint8_t *)sp-- = (((uint16_t)f) >> 8) & 0xff;


   //Place stack pointer at top of stack
   sp = sp - 33;

   p->sp = sp;		/* stack pointer into the "workSpace" */

   /*----END of NEW CODE----*/



   p->state = READY;
}


/**
  *  Create a new task
  */
static void Kernel_Create_Task( voidfuncptr f )
{
   int x;

   if (Tasks == MAXPROCESS) return;  /* Too many task! */

   /* find a DEAD PD that we can use  */
   for (x = 0; x < MAXPROCESS; x++) {
       if (Process[x].state == DEAD) break;
   }

   ++Tasks;
   Kernel_Create_Task_At( &(Process[x]), f );
}

/**
  * This internal kernel function is a part of the "scheduler". It chooses the
  * next task to run, i.e., CurrentP.
  */
  //Remobed static because it was blocking external access from assembly file cswitch.S.
  //We desire to see a 'T' not a 't' in the avr-nm output from the object file.
void Dispatch()
{
     /* find the next READY task
       * Note: if there is no READY task, then this will loop forever!.
       */
   while(Process[NextP].state != READY) {
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
   int x;

   Tasks = 0;
   KernelActive = 0;
   NextP = 0;

   for (x = 0; x < MAXPROCESS; x++) {
      memset(&(Process[x]),0,sizeof(PD));
      Process[x].state = DEAD;
   }
}


/**
  * This function starts the RTOS after creating a few tasks.
  */
void OS_Start()
{
   if ( (! KernelActive) && (Tasks > 0)) {
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
void Task_Create( voidfuncptr f)
{
   Disable_Interrupt();
   Kernel_Create_Task( f );
   Enable_Interrupt();
}

/**
  * The calling task gives up its share of the processor voluntarily.
  */
void Task_Next()
{
   if (KernelActive) {
     Disable_Interrupt();
     CurrentP ->state = READY;
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
   if (KernelActive) {
      Disable_Interrupt();
      CurrentP -> state = DEAD;
        /* we will NEVER return here! */
      JUMP(Exit_Kernel);
   }
}


/*============
  * A Simple Test
  *============
  */

/**
  * A cooperative "Ping" task.
  * Added testing code for LEDs.
  */
void Ping()
{
  for(;;){
  	//Toggle B1
    PORTB ^= 0b00000010;

    _delay_ms(100);

    Task_Next();
  }
}


/**
  * A cooperative "Pong" task.
  * Added testing code for LEDs.
  */
void Pong()
{
  for(;;) {

	//Toggle B0
	PORTB ^= 0b00000001;

    _delay_ms(50);

    Task_Next();
  }
}


/**
  * This function creates two cooperative tasks, "Ping" and "Pong". Both
  * will run forever.
  */
int main()
{
    //Init io
    DDRB = 0xFF;
    PORTB = 0;

   OS_Init();
   Task_Create( Pong );
   Task_Create( Ping );
   OS_Start();
}
