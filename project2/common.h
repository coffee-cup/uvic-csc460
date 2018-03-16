#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "uart.h"

/*==================================================================
 *        S T A N D A R D   I N L I N E    P R O C E D U R E S
 *==================================================================
 */

#define BIT_SET(PORT, PIN)  PORT |= 1 << PIN              // Sets  the single ith bit of PORT to HIGH where i == PIN
#define BIT_CLR(PORT, PIN)  PORT &= ~(1 << PIN)           // Sets  the single ith bit of PORT to LOW  where i == PIN
#define BIT_FLIP(PORT, PIN) PORT ^= 1 << PIN              // Flips the single ith bit of PORT         where i == PIN
#define BIT_TEST(PORT, PIN) ((PORT & (1 << PIN)) != 0)    // Returns TRUE if  ith bit of PORT is set to HIGH

#define MASK_SET(PORT, MASK) PORT |= MASK                 // Sets  ith bits of PORT to HIGH when the ith bit in MASK is 1
#define MASK_CLR(PORT, MASK) PORT &= ~(MASK)              // Sets  ith bits of PORT to LOW  when the ith bit in MASK is 1
#define MASK_FLIP(PORT, MASK) PORT ^= MASK                // Flips ith bits of PORT         when the ith bit in MASK is 1
#define MASK_TEST_ALL(PORT, MASK) ((PORT & MASK) == MASK) // Returns TRUE if ALL bits in MASK are also set in PORT
#define MASK_TEST_ANY(PORT, MASK) ((PORT & MASK) != 0)    // Returns TRUE if ANY bits in MASK are also set in PORT

#define LOW_BYTE(X)   (((uint16_t)X)       & 0xFF)        // Returns the 8 LSB bits of X
#define HIGH_BYTE(X) ((((uint16_t)X) >> 8) & 0xFF)        // Returns the 8 MSB bits of X
#define ZeroMemory(X, N) memset(&(X), 0, N)               // Sets N bytes of memory to 0 starting at X

#define DEBUG 1

// Baud rate for log messages
#define LOGBAUD (38400)

// Print a string over uart 0
#define LOG(...)                         \
    {                                    \
        if (DEBUG) {                     \
            UART_Init0(LOGBAUD);         \
            UART_print(__VA_ARGS__);     \
        }                                \
    }

/**
 * Macro to simulate a decorator for automatically calling Task_Next()
 */
#define TASK(body)           \
    {                        \
        for (;; Task_Next()) \
        {                    \
            body             \
        }                    \
    }

/*==================================================================
 *        S T A N D A R D    T Y P E    D E F I N I T I O N S
 *==================================================================
 */
#define TRUE         true                /* stdbool types */
#define FALSE        false               /* stdbool is a very light-weight header */
#define ANY          0xFF                /* A mask for ALL message types */

#define MAXTHREAD    16                  /* Maximum supported threads */
#define WORKSPACE    256                 /* in bytes, per THREAD */
#define MSECPERTICK  10                  /* resolution of a system TICK in milliseconds */

typedef uint16_t     PID;                /* Process ID: always non-zero if it is valid */
typedef uint16_t     TICK;               /* Tick type: Length of 1 TICK is defined by MSECPERTICK */
typedef bool         BOOL;               /* Boolean type: C99 introduced _Bool */
typedef uint8_t      MTYPE;              /* Message type: used to classify messages */
typedef uint8_t      MASK;               /* Message Mask type: used to filter messages by type */

typedef void (*taskfuncptr) (void);      /* pointer to void f(void) */

/**
 * This is the set of possible task priority levels
 */
typedef enum priority_level {
    SYSTEM = 0,
    PERIODIC,
    RR,
    NUM_PRIORITY_LEVELS /* Must be last */
} PRIORITY_LEVEL;

/**
 *  This is the set of states that a task can be in at any given time.
 */
typedef enum process_state {
    DEAD = 0,
    READY,
    RUNNING,
    SEND_BLOCK,
    REPLY_BLOCK,
    RECV_BLOCK,
    NUM_PROCESS_STATES /* Must be last */
} PROCESS_STATE;

/**
 * This is the set of kernel requests, i.e., a request code for each system call.
 */
typedef enum kernel_request_type {
    NONE = 0, /* Must be first */
    TIMER,
    CREATE,
    NEXT,
    GET_ARG,
    GET_PID,
    GET_NOW,
    MSG_SEND,
    MSG_RECV,
    MSG_RPLY,
    MSG_ASEND,
    TERMINATE,
    NUM_KERNEL_REQUEST_TYPES /* Must be last */
} KERNEL_REQUEST_TYPE;

/**
 * This struct is used to indirectly pass information within a kernel request
 */
typedef struct kernel_request_params_type {
    PID                       out_pid;              /* Set by the kernel, used to return PID */
    TICK                      out_now;              /* Set by the kernel, used to return elapsed ticks */
    taskfuncptr               code;                 /* function to be executed as a task  */
    int16_t                   arg;                  /* parameter to be passed to the task */
    KERNEL_REQUEST_TYPE       request;
    PRIORITY_LEVEL            priority;
    TICK                      period;               /* The period of a PERIODIC task */
    TICK                      wcet;                 /* The worst case execution time of a PERIODIC task */
    TICK                      offset;               /* The initial delay to start for a PERIODIC task */
    uint16_t                  *msg_ptr_data;
    uint16_t                  msg_data;
    MTYPE                     msg_mask;
    PID                       msg_to;
} KERNEL_REQUEST_PARAMS;

#endif
