# Testing

A crucial aspect of any complex software system is testing. How can the developers be sure the software they developed is working as originally designed? How can the users be sure the software they are using is working as described? The answer to this is testing.

We used several testing techniques throughout the development of our RTOS. These include

- tracing,
- logging, and
- an automated test suite

## Tracing

Tracing can be used to understand the order of execution of system calls, without the added overhead of the `printf` style of debugging. In a real-time operating system, timing is essential. Adding logging message or `printf`'s throughout your code can dramatically change the behaviour. Your software may not work in a production system, but operate normally when you add `printf`'s everywhere. This is because logging and printing messages as the program is running is a very slow process. Tracing enables the developer to understand the order which functions are being called without altering the behaviour.

We used tracing in our RTOS by first developing a small tracing library. The function signatures are,

```c
void add_to_trace(char c);
void clear_trace(void);
void print_trace(void);
int compare_trace(uint8_t arr[]);
```

Internally, the trace is represented as a `static uint8_t` array of size `256`. When using this library, the trace is initially cleared of its contents. Then throughout execution of the program or RTOS, 8 bit pieces of data, such as `char`'s, can be added to the end of the trace. Upon OS abortion or termination, the trace can be printed and examined. The `compare_trace` function is used to compare the internal trace to another array. This is useful when doing automated testing.

### Task Trace Tests

The order of execution of RTOS tasks (processes) is critical. There is a strict priority which must be adhered to. That is, `system > periodic > round robin`. We tested this priority was followed by using traces. In our [test suite](#test-suite) we create automated tests which created tasks of all priorities. All the task did was add some data to the trace. One such test which ensured that a system task ran as soon as it was created (assuming no other system tasks exist) is shown in the following code,

```c
void Task_System() {
    add_to_trace('a');
}

void Task_Create_Priority() {
    clear_trace();
    add_to_trace('s');

    Task_Create_System(Task_System, 0);
    add_to_trace('b');

    add_to_trace('f');

    uint8_t arr[] = {'s', 'a', 'b', 'f'};
    Assert(compare_trace(arr) == 1);
}
```

These tests are simple, but they give us the guarantee that our scheduler is working correctly.

### Message Passing Trace Tests

Another situation where we used traces is when testing message passing. There are several cases to consider. Between a single sender and receiver we need to consider when

- the receiver is a higher priority than the sender
- the sender is a higher priority than the receiver
- the sender and receiver are equal priorities.

As an example lets consider the first case, where the receiver is a higher priority than the sender. In this situation, when the sender unblocks the receiver by sending a message to it, the receiver should immediately wake up and the sender is moved to the Send block state. However, when the higher priority task replies to the lower priority sender, the higher priority task should remain running and context should not be switched to the sender. The following test uses our tracing library to confirm the above behaviour.

```c
/*
 * If a higher priority task is blocked from receiving, it is run
 * first after being unblocked and continues to run after replying
 */

void Msg_System_Recv() {
    add_to_trace('a');
    uint16_t x;
    PID from = Msg_Recv(ANY, &x);
    add_to_trace('c');
    Msg_Rply(from, 0);
    add_to_trace('d');
}

void Msg_RR_Send() {
    clear_trace();
    add_to_trace('s');

    PID pid = Task_Create_System(Msg_System_Recv, 0);
    add_to_trace('b');

    uint16_t x = 0;
    Msg_Send(pid, ANY, &x);
    add_to_trace('e');

    add_to_trace('f');

    uint8_t arr[] = {'s', 'a', 'b', 'c', 'd', 'e', 'f'};
    Assert(compare_trace(arr) == 1);
}
```

We also need to test the behaviour when multiple processes are sending a message to a single receiver. The cases we need to account for are

- the ordering of the received messages and
- selectively receiving messages which match the receivers mask.

As an example, lets look at the test for the ordering of received messages. According to our implementation the first message sent should be the first message received by a process. To test this we first create two sending processes and have one process delay for a short amount of time (so we can be sure of the order of sending). We also create a receiving process which waits for a longer time than both senders. Both sending processes send data in their messages which gets added to a trace by the receiver. This trace can be compared to the expected trace. The code for this test is below,

```c
/*
 * First in first out messages
 * Order of sent messages is respected when receiving
 */

void Msg_FIFO_Recv() {
    PID from;
    uint16_t x;

    _delay_ms(100);

    from = Msg_Recv(ANY, &x);
    add_to_trace(x);
    Msg_Rply(from, 0);

    from = Msg_Recv(ANY, &x);
    add_to_trace(x);
    Msg_Rply(from, 0);
}

void Msg_FIFO_Send_1() {
    uint16_t x = 'a';
    Msg_Send(Task_GetArg(), ANY, &x);
}

void Msg_FIFO_Trace() {
    PID pid = Task_Create_RR(Msg_FIFO_Recv, 0);

    add_to_trace('s');
    // This task should have a higher pid than the current task
    Task_Create_RR(Msg_FIFO_Send_1, pid);

    _delay_ms(10);
    uint16_t x = 'b';
    Msg_Send(pid, ANY, &x);

    add_to_trace('f');

    uint8_t arr[] = {'s', 'a', 'b', 'f'};
    Assert(compare_trace(arr) == 1);
}
```

We have shown our simpler tests for brevity, but we encourage the reader to look at our [message](https://github.com/coffee-cup/uvic-csc460/blob/master/project2/libs/tests/cases/msg_trace_test.c) [tests](https://github.com/coffee-cup/uvic-csc460/blob/master/project2/libs/tests/cases/msg_test.c) in our test suite.

## DEBUG Mode / LOG

Trace tests are important as they don't have a noticeable effect on the programs running time. However, in situations where timing is not critical, it can be useful see program output as it is running. For this reason, we included a UART library with our RTOS. This enables us to send streams of characters over a serial port back to our terminals. Below are some macros we use to simplify the logging process.

```c
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
```

A `DEBUG` flag is used so we can easily disable the log messages when not testing. One situation in which we use logging is to print the abort code when the RTOS crashes.

```c
LOG("OS Abort. Error code: %d\n", error);
```

## Test Suite

We wanted to be sure our RTOS was implemented exactly as designed and we also didn't want to break existing features as we developed new ones. A solution to this problem was to develop an automated testing suite.

Through the use of an environment variable, we can run all or some of our tests at once.

```make
ifdef TEST
	CFLAGS += -DRUN_TESTS
endif
```

When this flag is set, our testing code is included and run. Using a flag set at compile time is convenient and has the added benefit of not including our testing code when running _production_ or user programs. This means that the tests do not take up memory when not necessary.

Our tests are controlled with 1 byte masks. Individual tests can be run by explicitly setting the mask or all the tests can be run with the `0xFF` mask.

```c
typedef enum {
    TEST_QUEUE          = 0x01,
    TEST_MSG            = 0x02,
    TEST_OSFN           = 0x04,
    TEST_MSG_TRACE      = 0x08,
    TEST_TASKS          = 0x10,
    TEST_ALL            = 0xFF
} TEST_MASKS;
```

To invoke the tests we simply need to call

```c
Test_Suite(TEST_MASK);
```

We make heavy use of C preprocessor macros when creating tests. The following macro creates a new test which will run when the correct mask is provided. It also will first log when the test has started and finished over UART.

```c
#define Test_Case(m, mask, name, fn) \
    { \
        if ((m & mask) == mask) { \
            UART_print("Starting %s test...\n", name); \
            BIT_SET(PORTD, 1); \
            fn(); \
            Check_PortE(); \
            BIT_CLR(PORTD, 1); \
            UART_print("%s test complete\n", name); \
        } \
    }
```

We then [use this macro](https://github.com/coffee-cup/uvic-csc460/blob/master/project2/libs/tests/tests.c) like so,

```c
Test_Case(mask, TEST_QUEUE, "Queue", Task_Queue_Test);
Test_Case(mask, TEST_MSG, "Msg", Msg_Test);
Test_Case(mask, TEST_OSFN, "OSFN", OSFN_Test);
Test_Case(mask, TEST_MSG_TRACE, "Msg Trace", Msg_Trace_Test);
Test_Case(mask, TEST_TASKS, "Task", Task_Test);
```

Our test suite includes five automated tests which test

- the process queues,
- message passing,
- message passing with traces,
- general os functions, and
- task creation.

In the tests we use several different `Assert` macros, which log the filename and line if the assert fails.

### OS Abort Tests

When developing the test suite we realized it would be useful to test that the RTOS correctly aborted when expected. However, since our RTOS normally disables interrupts and halts execution when an abort occurs, we needed to add additional behaviour to treat OS aborts differently when running the test suite. We use a trick with IO port E on the micro controller to detect when an abort as occurred. If we are currently running the tests and an OS abort occurs, we set the low 4 bits of `PORT E` to HIGH. In the suite we can then check the state of `PORT E` to test whether or not an abort has occurred.

The above feature is utilized in the task tests. When creating a task with a `NULL` function, we expect an abort to be called. Using the above trick we can confirm an abort occurred without stopping the RTOS.

```c
/*
 * Creating a task with null function should OS abort
 */

void Task_Create_Null() {
    Task_Create_RR(NULL, 0);
    AssertAborted();
}
```
