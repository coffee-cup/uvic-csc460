
#include "os.h"
#include "test_utils.h"
#include "trace.h"
#include <avr/io.h>
#include <util/delay.h>

static const MASK END = 0x01;
static const MASK RELEASE = 0x02;

// Process that just adds its arg to a trace
void Task_Trace(void) {
    // Wait for 1 tick
    _delay_ms(10);
    uint8_t arg = (uint8_t)Task_GetArg();
    add_to_trace(arg);
}

void Simple_Msg_Trace(void) {
    clear_trace();
    Task_Create_RR(Task_Trace, 'b');
    Task_Create_System(Task_Trace, 'a');

    _delay_ms(1000);

    uint8_t arr[] = {'a', 'b'};
    Assert(compare_trace(arr) == 1);
}

/*
 * Chaining of task creation which unfold in the opposite way
 */

void Msg_Chain_1(void) {
    _delay_ms(100);
    add_to_trace('1');

    uint16_t x = 'a';
    Msg_Send(Task_GetArg(), RELEASE, &x);
}

void Msg_Chain_2(void) {
    add_to_trace('2');
    Task_Create_RR(Msg_Chain_1, Task_Pid());

    uint16_t x;
    PID from = Msg_Recv(RELEASE, &x);
    add_to_trace(x);
    Msg_Rply(from, 0);

    x = 'b';
    Msg_Send(Task_GetArg(), RELEASE, &x);
}

void Msg_Chain_3(void) {
    add_to_trace('3');

    Task_Create_RR(Msg_Chain_2, Task_Pid());

    uint16_t x;
    PID from = Msg_Recv(RELEASE, &x);
    add_to_trace(x);
    Msg_Rply(from, 0);

    x = 'c';
    Msg_Send(Task_GetArg(), RELEASE, &x);
}

void Msg_Chain_Start(void) {
    clear_trace();

    add_to_trace('s');

    Task_Create_RR(Msg_Chain_3, Task_Pid());

    uint16_t x;
    PID from = Msg_Recv(RELEASE, &x);
    add_to_trace(x);
    Msg_Rply(from, 0);
    add_to_trace('f');

    uint8_t arr[] = {'s', '3', '2', '1', 'a', 'b', 'c', 'f'};
    Assert(compare_trace(arr) == 1);
}

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

/*
 * If a higher priority task is blocked from receiving, it is run
 * first after being unblocked
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

/*
 * If a higher priority task sends a message to a lower priority task,
 * the higher priority task is run after lower priority task replies
 */

void Msg_System_Send() {
    add_to_trace('a');

    uint16_t x = 0;
    Msg_Send(Task_GetArg(), ANY, &x);

    add_to_trace('c');
}

void Msg_RR_Recv() {
    clear_trace();
    add_to_trace('s');

    Task_Create_System(Msg_System_Send, Task_Pid());

    uint16_t x;
    PID from = Msg_Recv(ANY, &x);
    add_to_trace('b');
    Msg_Rply(from, 0);
    add_to_trace('d');

    add_to_trace('f');

    uint8_t arr[] = {'s', 'a', 'b', 'c', 'd', 'f'};
    Assert(compare_trace(arr) == 1);
}

/*
 * Msg mask is respected
 */

void Msg_Mask_Recv() {
    uint16_t x;
    PID from;

    from = Msg_Recv(0x01, &x);
    add_to_trace(x);
    Msg_Rply(from, 0);

    from = Msg_Recv(0x10, &x);
    add_to_trace(x);
    Msg_Rply(from, 0);
}

void Msg_Mask_Send_2() {
    uint16_t x = 'a';
    Msg_Send(Task_GetArg(), 0x01, &x);
}

void Msg_Mask_Send() {
    clear_trace();
    add_to_trace('s');

    PID pid = Task_Create_RR(Msg_Mask_Recv, 0);
    Task_Create_RR(Msg_Mask_Send_2, pid);

    uint16_t x = 'b';
    Msg_Send(pid, 0x10, &x);

    add_to_trace('f');

    uint8_t arr[] = {'s', 'a', 'b', 'f'};
    Assert(compare_trace(arr) == 1);
}

void Msg_Trace_Test() {
    clear_trace();
    Simple_Msg_Trace();

    clear_trace();
    Msg_Chain_Start();

    clear_trace();
    Msg_FIFO_Trace();

    clear_trace();
    Msg_RR_Send();

    clear_trace();
    Msg_RR_Recv();

    clear_trace();
    Msg_Mask_Send();
}
