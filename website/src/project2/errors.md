# Errors and Exceptions

As our RTOS will be used as a base for our project 3, it is important any and all errors are handled properly. Some errors are recoverable and the operating system will simply no-op. Others are unrecoverable and the operating system will abort and report and error code. Others are unrecoverable and may result in the operating system completely crashing.

The following errors are recoverable and will result in a no-op.

- Creating one or more than `MAXTHREAD` tasks
- Sending or replying to a non-existent task

The following errors are unrecoverable and undetectable

- A task using more stack space than `WORKSPACE` bytes
- Deadlocks caused by sending and receiving using incompatible masks by two tasks

## Abort Codes

There are several errors which will result in the operating system disabling interrupts and aborting from normal execution. However, these errors do not result in a full board crash and are meant to help the programmer develop their program. Each of these errors is associated with a unique abort code. When an `OS_Abort` is called, one of these error codes is provided. Upon `OS_Abort` the RTOS will disable interrupts so that the system clock stops and will blink the onboard LED a number corresponding to the error code. The blinking will happen at 1 second intervals indefinitely. If `DEBUG` mode is enabled, the error code is also logged to UART.

Our RTOS has the following abort codes.

```c
typedef enum {
    TIMING_VIOLATION = 1,
    INVALID_REQ_INFO,
    FAILED_START,
    NO_REQUEST_INFO,
    WRONG_TASK_ORDER,
    INVALID_PRIORITY,
    PERIODIC_MSG,
    NULL_TASK_FUNCTION
} ABORT_CODE;
```

### Timing Violation

A timing violation occurs when a periodic task uses more than its worst case execution time. This happens when the task does not call `Task_Next` in time. This error can also be caused if there is a timing conflict with a periodic task. If a task is not started within 1 clock tick of its period, a timing violation occurs. This allows the programmer (user code) to catch when there is an error with the way their periodic tasks are setup.

### Invalid Request Info

This is an error caused when a kernel request is made with invalid request info. An example of this happening is when a kernel request is made to send a message to another process, but the process id given is invalid.

### Failed Start

This error occurs when the kernel does not take over execution of the system. It will only happen if `Kernel_Start` fails.

### No Request Info

This error occurs when a kernel request is made but there is no `request_info` associated with the request. When this happens the kernel does not know what to do so throws this error.

### Wrong Task Order

This error will be thrown when the first task of any of the task queues does not correspond to the currently running task. This error was mainly used for development and testing and should never happen for user programs.

### Invalid Priority

This error is thrown when some kernel requests are made while the currently running task is the idle process. For example, it does not make sense for the idle task to send a message.

### Periodic Message

The periodic message error is thrown when a periodic task calls any of the synchronous message passing functions. This error may happen as a result of an invalid user program.

### Null Task Function

This error is thrown when a task is created with a `NULL` function.
