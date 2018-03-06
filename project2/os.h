/* Last modified: MHMC Jan/30/2018 */
#ifndef _OS_H_
#define _OS_H_

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#define MAXTHREAD     16
#define WORKSPACE     256   // in bytes, per THREAD
#define MSECPERTICK   10   // resolution of a system TICK in milliseconds

#ifndef NULL
#define NULL          0   /* undefined */
#endif
#define TRUE          1
#define FALSE         0

#define ANY           0xFF       // a mask for ALL message type

typedef unsigned int PID;        // always non-zero if it is valid
typedef unsigned int TICK;       // 1 TICK is defined by MSECPERTICK
typedef unsigned int BOOL;       // TRUE or FALSE
typedef unsigned char MTYPE;
typedef unsigned char MASK;

typedef void (*voidfuncptr) (void);      /* pointer to void f(void) */

// Aborts the RTOS and enters a "non-executing" state with an error code. That is, all tasks
// will be stopped.
void OS_Abort(unsigned int error);

/**
 * This function initializes the RTOS and must be called before any other
 * system calls.
 */
void OS_Init();

/**
 * This function starts the RTOS after creating a few tasks.
 */
void OS_Start();

/*
 * Scheduling Policy:
 * There are three priority levels:
 *   HIGHEST  -- System tasks,
 *   MEDIUM   -- Periodic tasks,
 *   LOWEST   -- Round-Robin (RR) tasks
 * A ready System task preempts all other lower priority running tasks. Preemption occurs
 * immediately, not until the next TICK.
 * A ready Periodic task preempts all other lower priority running tasks.
 * Periodic tasks must be scheduled conflict-free, i.e., no two periodic tasks should be
 * ready at the same time. It is the application engineer's responsibility to ensure that
 * it is the case. When a timing violation occurs, the RTOS may abort.
 * When a Periodic task is preempted, it is put on hold until all higher priority tasks
 * are no longer ready. However, when it is resumed later, a timing violation occurs if
 * another Periodic becomes ready, i.e., there is a timing conflict. The RTOS may abort.
 * System and RR tasks are first-come-first-served. They run until they terminate, block,
 * or yield. RR tasks, on the other hand, run until they expire their quantum, or are
 * pre-empted. If they are preempted, then reenter at the front of their level. If they
 * expire their quantum, then they go back to the end of their level. Currently, a quantum
 * is defined to be 1 TICK.
 */

// TODO: Replace with Priority based task creates
void Task_Create(voidfuncptr f);

PID   Task_Create_System(void (*f)(void), int arg);
PID   Task_Create_RR(    void (*f)(void), int arg);

/**
 * f a parameterless function to be created as a process instance
 * arg an integer argument to be assigned to this process instanace
 * period its execution period in multiples of TICKs
 * wcet its worst-case execution time in TICKs, must be less than "period"
 * offset its start time in TICKs
 * returns 0 if not successful; otherwise a non-zero PID.
 */
PID   Task_Create_Period(void (*f)(void), int arg, TICK period, TICK wcet, TICK offset);

// NOTE: When a task function returns, it terminates automatically!!

// When a Periodic ask calls Task_Next(), it will resume at the beginning of its next period.
// When a RR or System task calls Task_Next(), it voluntarily gives up execution and
// re-enters the ready state. All RR and Systems tasks are first-come-first-served.
//
void Task_Next(void);


// The calling task gets its initial "argument" when it was created.
int  Task_GetArg(void);


// It returns the calling task's PID.
PID  Task_Pid(void);

//
// Send-Recv-Rply is similar to QNX-style message-passing
// Rply() to a NULL process is a no-op.
// See: http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_sys_arch%2Fipc.html
//
// Note: PERIODIC tasks are not allowed to use Msg_Send() or Msg_Recv().
//
void Msg_Send( PID  id, MTYPE t, unsigned int *v );
PID  Msg_Recv( MASK m,           unsigned int *v );
void Msg_Rply( PID  id,          unsigned int r );

//
// Asychronously Send a message "v" of type "t" to "id". The task "id" must be blocked on
// Recv() state, otherwise it is a no-op. After passing "v" to "id", the returned PID of
// Recv() is NULL (non-existent); thus, "id" doesn't need to reply to this message.
// Note: The message type "t" must satisfy the MASK "m" imposed by "id". If not, then it
// is a no-op.
//
// Note: PERIODIC tasks (or interrupt handlers), however, may use Msg_ASend()!!!
//
void Msg_ASend( PID  id, MTYPE t, unsigned int v );

/**
 * Returns the number of milliseconds since OS_Init(). Note that this number
 * wraps around after it overflows as an unsigned integer. The arithmetic
 * of 2's complement will take care of this wrap-around behaviour if you use
 * this number correctly.
 * Let  T = Now() and we want to know when Now() reaches T+1000.
 * Now() is always increasing. Even if Now() wraps around, (Now() - T) always
 * >= 0. As long as the duration of interest is less than the wrap-around time,
 * then (Now() - T >= 1000) would mean we have reached T+1000.
 * However, we cannot compare Now() against T directly due to this wrap-around
 * behaviour.
 * Now() will wrap around every 65536 milliseconds. Therefore, for measurement
 * purposes, it should be used for durations less than 65 seconds.
 */
unsigned int Now();  // number of milliseconds since the RTOS boots.


/*==================================================================
 *        S T A N D A R D   I N L I N E    P R O C E D U R E S
 *==================================================================
 */

/*
 * inline assembly code to disable/enable maskable interrupts
 * (N.B. Use with caution.)
 */
#define OS_DI()    asm(" sei ")  /* disable all interrupts */
#define OS_EI()    asm(" cli ")  /* enable all interrupts */


/**
 * Booting:
 *  The RTOS and the main application are compiled into a single executable binary, but
 *  otherwise they are totally independent.
 *  There is a single "main()" function, where the combined executable starts. This "main()"
 *  is implemented inside the RTOS, which initializes the RTOS and creates a first application
 *  System task "a_main()".
 *
 *  "a_main()" is a parameterless function defined by the application, which will create all other
 *  application tasks as necessary.
 */
#endif /* _OS_H_ */
