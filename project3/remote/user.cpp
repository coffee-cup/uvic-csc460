#include <avr/io.h>
#include <util/delay.h>
#include "Roomba.h"
#include "Packet.h"

Roomba roomba(/*Serial*/ 3, /*Port A pin*/ 0);

extern "C" {
    #include "kernel.h"
    #include "os.h"
    #include "common.h"
    #include "uart.h"
    #include "utils.h"
    #include "timings.h"
    void create(void);
}

Packet packet(512, 512, 0, 512, 512, 0);

void getData(void);
// void printPacket();

int main(void) {
    Kernel_Begin();
}

void create(void) {

    // Outputs for Tasks's
    BIT_SET(DDRB, 0);
    BIT_SET(DDRB, 1);
    BIT_CLR(PORTB, 0);
    BIT_CLR(PORTB, 1);

    // if (!roomba.init()) {
    //     OS_Abort(FAILED_START);
    // }

    // roomba.set_mode(Roomba::OI_MODE_TYPE::SAFE_MODE);
    // roomba.leds(Roomba::OI_LED_MASK_ARGS::CHECK_ROBOT_LED, 0, 0);

    Task_Create_Period(getData, 0, GET_DATA_PERIOD, GET_DATA_WCET, GET_DATA_DELAY);
        // Task_Create_Period(Move, 0, 100, 20, 5);

        // This function was called by the OS as a System task.
        // If a task executes a return statement it is terminated.
        // Could also call Kernel_Request(TERMINATE) here.

        return;
}

void Move(void) {
    uint8_t i = 0;
    TASK ({
        BIT_FLIP(PORTB, 0);
            LOG("Move\n");
            roomba.drive(100, i % 2 ? 1 : -1);
            if (++i >= 5) {
                roomba.power_off();
                Task_Terminate();
            }
    })
}

void getData() {
    uint8_t channel = 1;
    UART_Init(channel, 38400);

    for (;;) {
        if (UART_Available(channel)) {
            uint8_t r = UART_Receive(channel);

            // If the first byte isn't magic, don't attempt to read a packet
            if (r == PACKET_MAGIC) {
                uint8_t packet_buf[PACKET_SIZE];

                // Read in a packet and check its size
                int i;
                for (i = 0; i < PACKET_SIZE; i += 1) {
                    packet_buf[i] = UART_Receive(channel);
                }
                packet = Packet(packet_buf);
                LOG("%d\n", packet.field.joy1X);

                // Read and discard anything else available
                while (UART_Available(channel)) {
                    UART_Receive(channel);
                }
            }
        }

        Task_Next();
    }
}
