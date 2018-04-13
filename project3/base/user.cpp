#include <avr/io.h>
#include <util/delay.h>

#include "Joystick.h"
#include "Packet.h"

extern "C" {
    #include "kernel.h"
    #include "os.h"
    #include "common.h"
    #include "uart.h"
    #include "utils.h"
    #include "timings.h"
    void create(void);
}

typedef struct {
    const uint8_t joy1X;
    const uint8_t joy1Y;
    const uint8_t joy1SW;
    const uint8_t joy2X;
    const uint8_t joy2Y;
    const uint8_t joy2SW;
} PinDefs;

PinDefs pin = {
    .joy1X  = 15,
    .joy1Y  = 14,
    .joy1SW = 1, // PORTA
    .joy2X  = 12,
    .joy2Y  = 13,
    .joy2SW = 0 // PORTA
};

Packet packet(512, 512, 0, 512, 512, 0);

Joystick joystick1(pin.joy1X, pin.joy1Y, pin.joy1SW);
Joystick joystick2(pin.joy2X, pin.joy2Y, pin.joy2SW);

DELEGATE_MAIN();
uint8_t data_channel = 2;

void updatePacket(void) {
    TASK({
        if (UART_Available(data_channel)) {
            UART_Flush(data_channel);
        }
        packet.joy1X(joystick1.getX());
        packet.joy1Y(joystick1.getY());
        packet.joy1SW(joystick1.getClick() ? 0xFF : 0x00);
        packet.joy2X(joystick2.getX());
        packet.joy2Y(joystick2.getY());
        packet.joy2SW(joystick2.getClick() ? 0xFF : 0x00);
    })
}

void TXData(void) {
    TASK({
        if (UART_Writable(data_channel)) {
            UART_send_raw_bytes(data_channel, PACKET_SIZE, packet.data);
            LOG(">");
        }
    })
}

void create(void) {
    UART_Init(data_channel, 38400);
    // Create tasks
    Task_Create_Period(updatePacket, 0, 1,  0, 1);
    Task_Create_Period(TXData,       0, 20, 4, 5);

    return;
}
