#include <avr/io.h>
#include <util/delay.h>
#include "os.h"
#include "common.h"

static PID server_pid;

void Server(void) {
    uint16_t x;
    for (;;) {
        Msg_Recv(ANY, &x);
        PORTB = 0x00;
        BIT_SET(PORTB, x);
    }
}

void Client(void) {
    uint16_t pin = Task_GetArg();
    for (;;) {
        Msg_ASend(server_pid, 1 << pin, pin);
        Task_Next();
    }
}

void setup_1(void) {
    // All outputs
    DDRB = 0xFF;
    PORTB = 0x00;

    server_pid = Task_Create_RR(Server, 0);
    _delay_ms(1000);

    int16_t speed = 5;
    int16_t n = 8;
    int16_t p = speed * n;

    int i;
    for (i = 0; i < n; i += 1) {
        Task_Create_Period(Client, i, p, 1, speed * i);
    }

    return;
}
