# Design

In this section we cover the specific design choices made before and during implementation of our RTOS.

## Hardware choices

This year, each group was provided access to several Arduino Mega 2560 development boards. The Arduino Mega 2560 gets its name from the central processing unit present on the board; the AMTEL ATmega2560. Specific documentation for the ATmega2560 can be [found here](http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2549-8-bit-AVR-Microcontroller-ATmega640-1280-1281-2560-2561_datasheet.pdf) but a brief description follows.

[mega2560]: https://i.imgur.com/O6ApT45.jpg "Arduino Mega 2560"
![Arduino Mega 2560][mega2560]

At the board level, the Arduino Mega 2560 provides 54 digital configurable input / output pins, 15 of which can be configured to output pulse width modulated signals. Furthermore, there are 16 analog input pins. The Arduino Mega 2560 has one non-volatile secondary storage device which can hold up to 256 kB of data (although 8 kB are commonly reserved for the bootloader). Additionally, the Arduino Mega 2560 has 8 kB of SRAM - this device however is volatile as it requires power to retain information.

At the microprocessor level, the ATmega2560 operates on 8-bit registers at 16 MHz using AVR. Although, undocumented AVR is commonly accepted to stand for **A**lf-Egil and **V**egard's **R**ISC processor. The AVR architecture is a modified Harvard architecture which is part of the reduced instruction set computer (RISC) microprocessor family. The ATmega2560 has 32 8-bit general purpose registers, four Universal Synchronous/Asynchronous Receiver-Transmitters, and support for many peripherals including several 8 and 16 bit counter-timers, four 8-bit PWM channels and an additional twelve programmable (2 to 16-bit) resolution PWM channels.

## Kernel Design

The kernel is critical part of our RTOS. As such, it received the most attention during implementation. It is also the owner of the `c` program's `main()` function. As discussed previously, a real-time operating system can be implemented using a time-sharing or event-driven scheme, or a hybrid of the two. Our operating system is a hybrid system. Tasks can generate 'events' by making system calls, meanwhile a counter-timer generates interrupts at a pre-specified frequency - we typically use 100 Hz or every 10 ms. This has the advantage of being more flexible when concerning usability. Tasks can voluntarily yield their share of the hardware, and if they do not, tasks of equal priority will still be given equal shares. The following diagram shows, at a high level, how the operating system starts its execution when the program begins. Some details are omitted for brevity.

[OSEntry]: https://i.imgur.com/5BxPZzZ.png "Entry point execution diagram"
![RTOS entry point execution diagram][OSEntry]

With reference to the entry point diagram above, the `main()` function calls `Kernel_Init()` which configures the system clock, clears then initialized pre-allocated required memory, then creates a high priority task for the application code to run its initialization code. It is assumed that all applications have a `setup()` function. This function is in place of the `a_main()` function used in other sample operating systems. When the `setup()` function returns, the operating system is ready to enter it's main execution loop. At this point control is passed all the back to the `main()` function. It then calls the `Kernel_Start()` function, which enables the counter-timer and dispatches a new task. The counter-timer will interrupt the currently executing task every 10 ms, during this interruption the kernel will perform a reschedule and possibly context switch to a new task.

In our project we tried to isolate the kernel functions from user programs as much as possible. This can be done by reducing the number of functions that are declared in the kernel's header file, and instead routing calls to the kernel (i.e.: syscalls) though a dedicated function. Below is the entirety of our kernel.h file, note that the only way of 'outside' communication with the kernel is though the `Kernel_Request()` function. A task is never allowed access to it's `ProcessDescriptor` (described below).

```c
#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "common.h"

void Kernel_Request(KERNEL_REQUEST_PARAMS* info);

#endif
```

## Task Design

A the core of a task is simply a function and some isolated stack space to execute the function. We use the notion of a process descriptor to keep track of a task. It contains a pointer to the function to execute, a pointer the task's stack along with the current top of the task's stack, additionally some extra properties are stored in the process descriptor to facilitate other functionality in our operating system. These include:

* The current running state of the task, which can be any of the following options:
    ```c
    typedef enum process_state {
        DEAD,
        READY,
        RUNNING,
        SEND_BLOCK,
        REPLY_BLOCK,
        RECV_BLOCK
    } PROCESS_STATE;
    ```
* The task's priority level, which can be any of the following options:
    ```c
    typedef enum priority_level {
        SYSTEM = 0,
        PERIODIC,
        RR
    } PRIORITY_LEVEL;
    ```
* Some timing variables, each of which is only used for `PERIODIC` level tasks:
    * Period
    * Worst Case Execution Time
    * Execution time remaining
    * Time until next start

The entire process descriptor is defined as follows:

```c
typedef struct ProcessDescriptor {
    volatile uint8_t* sp;                   /* stack pointer into the "workSpace" */
    uint8_t           workSpace[WORKSPACE]; /* stack allocation */
    volatile PROCESS_STATE state;           /* state of the process */
    taskfuncptr       code;                 /* function to be executed as a task  */
    int16_t           arg;                  /* parameter to be passed to the task */
    PID               process_id;           /* the process number */
    PRIORITY_LEVEL    priority;             /* priority of the task */
    TICK              period;               /* period of a PERIODIC task */
    TICK              wcet;                 /* worst case exec time of a PERIODIC task */
    TICK              ttns;                 /* time to next start for a PERIODIC task */
    TICK              ticks_remaining;      /* Until a PERIODIC task is forced to yield */
    struct ProcessDescriptor* next;         /* Used for in-place task queue */
    volatile KERNEL_REQUEST_PARAMS *req_params; /* cached kernel request */
} PD;
```

Tasks can call any of the functions defined in our modified [`os.h`](https://github.com/coffee-cup/uvic-csc460/blob/cc0288c4173f32dfe451c822ecb1c3c61e0af104/project2/os.h) file. These are:

* `Task_Create_System` - Creates a new system level task
* `Task_Create_Period` - Creates a new periodic level task
* `Task_Create_RR`     - Creates a new Round Robin level task
* `Task_Next`          - Immediately forfeits current share of hardware
* `Task_Terminate`     - Immediately terminates
* `Task_GetArg`        - Retrieves the integer argument used during creation
* `Task_Pid`           - Retrieves the task's process id
* `Msg_Send`           - Sends a message to another process
* `Msg_Recv`           - Receives a process from another process
* `Msg_Rply`           - Replies to a message from another process
* `Msg_ASend`          - Asynchronously sends a message to another process
* `Now`                - Retrieves the current system time

There are three priority levels of tasks available to the application. One further priority level is for kernel use only, and is of lower priority than any other task type. This priority level is used for an idle task for cases when no other tasks are available to run.

### System tasks

A system task is the highest priority task. They are scheduled on a first come, first serve basis, and run until they terminate, yield, or block. A system task cannot be preempted by any other running task. System tasks can call any of the message passing functions. Tasks of this type are stored in their own task queue. To create a task of this type, the application can call the function below where it must provide a function pointer to execute as the task and an argument which is retrievable by the task.
```c
PID Task_Create_System(taskfuncptr f, int16_t arg);
```

### Periodic tasks

A periodic task is the medium priority task. They are scheduled in accordance with their period and offset within at most 10 ms (1 tick jitter). A periodic task is immediately preempted at the next tick or 'rescheduling' syscall by any `READY` system task. Periodic tasks are never preempted by round-robin tasks. Periodic tasks run until they terminate, yield, or exceed their specified worst-case execution time, whichever first. Note that a periodic task **cannot** become blocked, this is because calls to any of the *synchronous* message passing functions devolves to a no-op if the caller is a periodic function. A periodic task cannot be preempted by any other running task. Tasks of this type are stored in their own task queue. To create a task of this type, the application can call the function below where it must provide a function pointer to execute as the task, an argument which is retrievable by the task, and measures for it's period, worst-case execution time, and starting offset respectively.
```c
PID Task_Create_Period(taskfuncptr f, int16_t arg, TICK period, TICK wcet, TICK offset);
```

### Round Robin tasks

A round robin task is the lowest priority task. They are scheduled on a first come, first serve basis, and for one tick at most at a time, or until they yield, or block, whichever first. A round robin task is preempted by any other running task. Tasks of this type are stored in their own task queue. To create a task of this type, the application can call the function below where it must provide a function pointer to execute as the task and an argument which is retrievable by the task.
```c
PID Task_Create_RR(taskfuncptr f, int16_t arg);
```

---

A task can exist in one of the aforementioned 6 states - tracked by it's `state` variable.

### Dead state
A task which has it's state set to `DEAD` is never considered for rescheduling, and it's memory space is returned to the pool of available task memory. All available task memory is initialized with `DEAD` tasks, and a task may become dead if it performs a `return` instruction from its provided function (i.e.: at the bottom of it's stack), or if it ever calls `Task_Terminate` (i.e: regardless of the size of it's stack). `DEAD` tasks are removed from their respective task queue. Any priority task may become `DEAD`.

### Ready state
A task with state `READY` is non-blocked and currently not running. It is considered for dispatch when rescheduling. Any priority task may be `READY`. Tasks which have become unblocked by leaving any of the blocking states are made `READY`.

### Running state
A task with state `RUNNING` is non-blocked and currently running. It is not considered for dispatch when rescheduling as no tasks should be in the `RUNNING` state while the kernel is active. Any priority task may be `RUNNING`. Tasks may move to any other state from the `RUNNING` state, blocking states included. When a syscall is made a task is first made `READY`, then depending on the syscall it might be moved to a blocking state.

### Send Block state
A task with state `SEND_BLOCK` is blocked and currently not running. It is not considered for dispatch when rescheduling as it will be unable to continue. Only system and round robin priority level tasks may enter the `SEND_BLOCK` state. When another task makes a syscall that resolves the `SEND_BLOCK` state, the task may be moved to another blocking state or made `READY` again.

### Reply Block state
A task with state `REPLY_BLOCK` is blocked and currently not running. It is not considered for dispatch when rescheduling as it will be unable to continue. Only system and round robin priority level tasks may enter the `REPLY_BLOCK` state. When another task makes a syscall that resolves the `REPLY_BLOCK` state, the task may be moved to another blocking state or made `READY` again.

### Receive Block state
A task with state `RECV_BLOCK` is blocked and currently not running. It is not considered for dispatch when rescheduling as it will be unable to continue. Only system and round robin priority level tasks may enter the `RECV_BLOCK` state. When another task makes a syscall that resolves the `RECV_BLOCK` state, is made `READY` again.

More on SEND-RECEIVE-REPLY blocking state is given in the inter process communication section.


## Request Handler design

This kernel component is used in the kernel's event loop to handle kernel requests. Rather than having a giant, unreadable, `switch` statement for each type of kernel request, it was decided that the event loop would perform a table lookup to find the appropriate kernel request handler. This helps especially in testing and with scope management. The kernel event loop is shown below.

```c
static void Next_Kernel_Request() {
    request_handler_func request_handlers[NUM_KERNEL_REQUEST_TYPES] = {
        /* Must be in order of KERNEL_REQUEST_TYPE */
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
        Kernel_Request_Terminate
    };

    Dispatch();  /* select a new task to run */

    for(;;) {
        if (request_info) {
            /* Clear the caller's request type */
            request_info->request = NONE;
            /* Clear our reference to request_info */
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


        /* Run the appropriate handler */
        if (request_info->request >= NONE && request_info->request < NUM_KERNEL_REQUEST_TYPES) {
            /* It's up to the handler to decide if it should dispatch */
            request_handlers[request_info->request]();
        } else {
            OS_Abort(INVALID_REQ_INFO);
            return;
        }
    }
}
```

