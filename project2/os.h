/* Last modified: MHMC Jan/30/2018 */
#ifndef _OS_H_
#define _OS_H_

#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include "process.h"
#include "common.h"

typedef enum {
    TIMING_VIOLATION = 1,
    NO_DEAD_PROCESS = 2,
    INVALID_REQ_INFO = 3,
    FAILED_START = 4,
    NO_REQUEST_INFO = 5,
    WRONG_TASK_ORDER = 6,
    INVALID_PRIORITY = 7,
    PERIODIC_MSG = 8
} ABORT_CODE;

// Aborts the RTOS and enters a "non-executing" state with an error code. That is, all tasks
// will be stopped.
void OS_Abort(ABORT_CODE error);

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

/**
 * Task Creation Functions
 *  - `f`:      a function to be created as a task instance
 *  - `arg`:    an integer argument to be assigned to this task instanace
 *  - `period`: the task's execution period in multiples of TICKs
 *  - `wcet`:   the task's worst-case execution time in TICKs, must be less than "period"
 *  - `offset`  the task's start time in TICKs
 * Returns 0 if not successful; otherwise a non-zero PID.
 * NOTE: If a /task function/ executes a return, it terminates automatically!
 */
PID Task_Create_System(taskfuncptr f, int16_t arg);
PID Task_Create_RR(taskfuncptr f, int16_t arg);
PID Task_Create_Period(taskfuncptr f, int16_t arg, TICK period, TICK wcet, TICK offset);

/**
 * When a Periodic ask calls Task_Next(), it will resume at the beginning of its next period.
 * When a RR or System task calls Task_Next(), it voluntarily gives up execution and
 * re-enters the ready state. All RR and Systems tasks are first-come-first-served.
 */
void Task_Next();

/**
 * The calling task terminates itself.
 */
void Task_Terminate(void);


/**
 * The calling task gets its initial "argument" when it was created.
 */
int16_t Task_GetArg();

/**
 * It returns the calling task's PID.
 */
PID Task_Pid();

/**
 * Send-Recv-Rply is similar to QNX-style message-passing
 * Rply() to a NULL process is a no-op.
 * See: http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_sys_arch%2Fipc.html
 *
 * Note: PERIODIC tasks are not allowed to use Msg_Send() or Msg_Recv().
 */
void Msg_Send(PID  id, MTYPE t, uint16_t* v);
PID  Msg_Recv(MASK m,           uint16_t* v);
void Msg_Rply(PID  id,          uint16_t r);

/**
 * Asychronously Send a message "v" of type "t" to "id". The task "id" must be blocked on
 * Recv() state, otherwise it is a no-op. After passing "v" to "id", the returned PID of
 * Recv() is NULL (non-existent); thus, "id" doesn't need to reply to this message.
 * Note: The message type "t" must satisfy the MASK "m" imposed by "id". If not, then it
 * is a no-op.
 *
 * Note: PERIODIC tasks (or interrupt handlers), however, may use Msg_ASend()!!!
 */
void Msg_ASend(PID id, MTYPE t, uint16_t v);

/**
 * Returns the number of milliseconds since OS_Init(). Note that this number
 * wraps around after it overflows as an uint16_teger. The arithmetic
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
uint16_t Now();  // number of milliseconds since the RTOS boots.


/**
 * Booting:
 *  The RTOS and the main application are compiled into a single executable binary, but
 *  otherwise they are totally independent.
 *  There is a single "main()" function, where the combined executable starts. This "main()"
 *  is implemented inside the RTOS, which initializes the RTOS and creates a first application
 *  System task "a_main()".
 *
 *  "a_main()" is a function defined by the application, which will create all other
 *  application tasks as necessary.
 */
#endif /* _OS_H_ */
