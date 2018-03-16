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
    /* PID from = Msg_Recv(0x01, &x); */
    UART_print("Value is %d\n", x);

    x += 1;
    /* Msg_Rply(from, x); */
}

void Msg_Recv_Before_Send_Send() {
    /* PID pid = Task_Create_RR(Msg_Recv_Before_Send_Recv, 0); */

    PID pid = 3;
    _delay_ms(100);
    uint16_t x = 34;
    Msg_Send(pid, ANY, &x);

    UART_print("In send Value is %d\n", x);
    Assert(x == 35);

    MASK arg_pid = Task_GetArg();
    Msg_Send(arg_pid, MSG_END, &x);
}

void Msg_Test() {
    /* Task_Create_RR(Msg_Send_Never, 0); */
    /* Task_Create_RR(Msg_Recv_Never, 0); */
    /* Task_Create_RR(Msg_Send_Bad_Mask, 0); */

    PID my_pid = Task_Pid();

    // Send this pid as arg to these tasks that we will wait on
    // to confirm that they finish
    Task_Create_RR(Msg_Recv_Before_Send_Send, my_pid);

    uint16_t x;
    Msg_Recv(MSG_END, &x);

    // Wait for tasks to be run
    _delay_ms(1000);
}
