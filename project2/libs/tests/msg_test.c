#include "../../common.h"
#include "../../os.h"
#include "test_utils.h"
#include "../trace/trace.h"
#include <avr/io.h>
#include <util/delay.h>

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

void Msg_Test() {
    Task_Create_RR(Msg_Send_Never, 0);
    Task_Create_RR(Msg_Recv_Never, 0);

    // Wait for tasks to be run
    _delay_ms(100);
}
