#include "../../common.h"
#include "../../os.h"
#include "test_utils.h"
#include "../trace/trace.h"
#include <avr/io.h>
#include <util/delay.h>

#define MSG_END 0x20

/*
 * Blocked on Send/Recv Test
 */

// This function should never complete
void Msg_Send_Never() {
    uint16_t x;
    Msg_Send(0, 0x10, &x);

    AssertNeverCalled();
}

// This function should never complete
void Msg_Recv_Never() {
    uint16_t x;
    Msg_Recv(0x01, &x);

    AssertNeverCalled();
}

/*
 * Bad Mask Test
 */

// This function should never complete because the mask is incorrect
void Msg_Recv_Bad_Mask() {
    uint16_t x;
    PID from = Msg_Recv(0x01, &x);
    Msg_Rply(from, 0);

    AssertNeverCalled();
}

// This function should never complete because no one will receive the sent message
void Msg_Send_Bad_Mask() {
    PID pid = Task_Create_RR(Msg_Recv_Bad_Mask, 0);

    uint16_t x = 0;
    Msg_Send(pid, 0x02, &x);

    AssertNeverCalled();
}

/*
 * Recv Before Send Test
 */

void Msg_Recv_Before_Send_Recv() {
    uint16_t x = 0;
    PID from = Msg_Recv(0x01, &x);

    x += 1;
    Msg_Rply(from, x);
}

void Msg_Recv_Before_Send_Send() {
    PID pid = Task_Create_RR(Msg_Recv_Before_Send_Recv, 0);

    _delay_ms(100);
    uint16_t x = 34;
    Msg_Send(pid, 0x01, &x);

    Assert(x == 35);

    PID arg_pid = Task_GetArg();
    Msg_Send(arg_pid, MSG_END, &x);
}

/*
 * Send Before Recv Test
 */
void Msg_Send_Before_Recv_Recv() {
    uint16_t x = 0;

    _delay_ms(100);
    PID from = Msg_Recv(0x04, &x);

    x -= Task_GetArg();
    Msg_Rply(from, x);
}

void Msg_Send_Before_Recv_Send() {
    uint16_t arg = 31;
    PID pid = Task_Create_RR(Msg_Send_Before_Recv_Recv, arg);

    uint16_t x = 90;
    Msg_Send(pid, 0x04, &x);

    Assert(x == (90 - arg));

    PID arg_pid = Task_GetArg();
    Msg_Send(arg_pid, MSG_END, &x);
}

/*
 * Async Send test
 */

void Msg_Async_Recv() {
    uint16_t x = 0;

    Msg_Recv(0x08, &x);
    x -= 4;
    Msg_Send(x, MSG_END, &x);
}

void Msg_Async_Send() {
    PID pid = Task_Create_RR(Msg_Async_Recv, 0);

    _delay_ms(200);
    Msg_ASend(pid, 0x08, Task_GetArg() + 4);
}

/*
 * Stress test
 */
void Msg_Stress_Recv() {
    uint16_t x;

    int i;
    for (i = 0; i < 20000; i += 1) {
        PID from = Msg_Recv(0x10, &x);
        x += 1;
        Msg_Rply(from, x);
    }
}

void Msg_Stress_Send() {
    PID pid = Task_Create_RR(Msg_Stress_Recv, 0);

    uint16_t x, y = 0;
    int i;
    for (i = 0; i < 20000; i += 1) {
        y = x;
        Msg_Send(pid, 0x10, &x);
        Assert(x == y + 1);
    }

    Msg_Send(Task_GetArg(), MSG_END, &x);
}

/*
 * Out of order messages
 */
void Msg_Out_Order_Recv() {
    uint16_t x;
    PID from;

    from = Msg_Recv(0x01, &x);
    x += 30;
    Msg_Rply(from, x);

    from = Msg_Recv(0x02, &x);
    x *= 2;
    Msg_Rply(from, x);
}

void Msg_Out_Order_Send_2() {
    PID pid = Task_GetArg();

    uint16_t x = 0;

    _delay_ms(100);

    x = 10;
    Msg_Send(pid, 0x01, &x);
    Assert(x == 10 + 30);
}

void Msg_Out_Order_Send_1() {
    PID pid = Task_Create_RR(Msg_Out_Order_Recv, 0);
    Task_Create_RR(Msg_Out_Order_Send_2, pid);

    uint16_t x = 0;

    x = 60;
    Msg_Send(pid, 0x02, &x);
    Assert(x == 60 * 2);

    Msg_Send(Task_GetArg(), MSG_END, &x);
}

void Msg_Test() {
    Task_Create_RR(Msg_Send_Never, 0);
    Task_Create_RR(Msg_Recv_Never, 0);
    Task_Create_RR(Msg_Send_Bad_Mask, 0);

    PID my_pid = Task_Pid();

    // Send this pid as arg to these tasks that we will wait on
    // to confirm that they finish
    Task_Create_RR(Msg_Recv_Before_Send_Send, my_pid);
    Task_Create_RR(Msg_Send_Before_Recv_Send, my_pid);
    Task_Create_RR(Msg_Async_Send, my_pid);
    Task_Create_RR(Msg_Stress_Send, my_pid);
    Task_Create_RR(Msg_Out_Order_Send_1, my_pid);

    uint16_t x;

    // Wait on all the tasks that need to run to completion
    Msg_Recv(MSG_END, &x);
    Msg_Recv(MSG_END, &x);
    Msg_Recv(MSG_END, &x);
    Msg_Recv(MSG_END, &x);
    Msg_Recv(MSG_END, &x);
}
