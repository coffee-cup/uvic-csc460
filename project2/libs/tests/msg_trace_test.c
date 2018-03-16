#include "../../common.h"
#include "../../os.h"
#include "test_utils.h"
#include "../trace/trace.h"
#include <avr/io.h>
#include <util/delay.h>

static const MASK END = 0x01;
static const MASK RELEASE = 0x02;

// Process that just adds its arg to a trace
void Task_Trace(void) {
    int i;
    for (i = 0; i < 1000; i += 1) {}
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

void Msg_Trace_Test() {
    Simple_Msg_Trace();
    Msg_Chain_Start();
}
