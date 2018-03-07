#ifndef _COMMON_H_
#define _COMMON_H_

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

typedef enum priority_levels {
                              SYSTEM = 0,
                              PERIODIC,
                              RR
} PRIORITY_LEVELS;

/**
 *  This is the set of states that a task can be in at any given time.
 */
typedef enum process_states {
                             DEAD = 0,
                             READY,
                             RUNNING
} PROCESS_STATES;

/**
 * This is the set of kernel requests, i.e., a request code for each system call.
 */
typedef enum kernel_request_type {
                                  NONE = 0,
                                  CREATE,
                                  NEXT,
                                  TERMINATE
} KERNEL_REQUEST_TYPE;

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

#endif
