#include <avr/io.h>
#include <util/delay.h>

#include "Arm.h"
#include "Joystick.h"
#include "Motor.h"

#include "Packet.h"

Arm arm;
Joystick joy1(15, 14, 2);
Joystick joy2(13, 12, 3);

Packet packet(512, 512, 0, 512, 512, 0);

extern "C" {
    #include "kernel.h"
    #include "os.h"
    #include "uart.h"
    #include "utils.h"
    void create(void);
}

// Weak attribute allows other functions to redefine
// We want kernel main to be the actual main
DELEGATE_MAIN()

/**
 *
 */
void UpdateArm(void) TASK ({
    arm.setSpeedX(Arm::filterSpeed(packet.joy1X()));
    arm.setSpeedY(Arm::filterSpeed(packet.joy1Y()));
})

/**
 *
 */
void TickArm(void) {
    // 30 seconds === 30000 ms
    uint32_t numLaserTicks = 30000 / MSECPERTICK;

    TASK({
        arm.tick();
        if (joy1.getClick() && numLaserTicks > 1) {
            BIT_SET(PORTC, 0);
            numLaserTicks -= 2;
        } else {
            BIT_CLR(PORTC, 0);
        }
    })
}

void RXData(void) {
    uint8_t data_channel = 2;
    uint8_t buffer[PACKET_SIZE];
    uint8_t byte, i;
    UART_Init(data_channel, 38400);

    TASK({
        if (UART_Available(data_channel)) {
            if (UART_Async_Receive(data_channel, &byte)) {
                if (byte == HIGH_BYTE(PACKET_MAGIC)) {
                    buffer[0] = byte;

                    if (UART_Async_Receive(data_channel, &byte)) {
                        if (byte == LOW_BYTE(PACKET_MAGIC)) {
                            buffer[1] = byte;

                            while(!UART_BytesAvailable(data_channel, PACKET_SIZE - 2))
                                ;

                            for (i = 2; i < PACKET_SIZE; i += 1) {
                                if (UART_Async_Receive(data_channel, &byte)){
                                    buffer[i] = byte;
                                } else {
                                    LOG("Expected a packet to be available!\n");
                                    OS_Abort(0);
                                }
                            }
                            packet = Packet(buffer);
                            LOG(">> [%X]:[%u]:[%u]:[%u]:[%u]:[%c]:[%c]\n", packet.magic(),
                                packet.joy1X(), packet.joy1Y(),
                                packet.joy2X(), packet.joy2Y(),
                                packet.joy1SW() ? '#' : '/',
                                packet.joy2SW() ? '#' : '/'
                            );

                        } else {
                            LOG("<< More Trash! [%X]\n", byte);
                        }
                    }
                } else {
                    LOG("<< Trash [%X]\n", byte);
                    UART_Flush(data_channel);
                }
            }
        } else {
            LOG("--");
        }
    })
}

/**
 *
 */
void create(void) {

    // Outputs for Tasks's
    BIT_SET(DDRB, 0);
    BIT_SET(DDRB, 1);
    BIT_SET(DDRC, 0); // Laser

    BIT_CLR(PORTB, 0);
    BIT_CLR(PORTB, 1);
    BIT_CLR(PORTC, 0);

    arm.attach(2, 3);

    Task_Create_Period(RXData, 0, 10, 5, 1);
    // Task_Create_Period(TickArm, 0, 2, 1, 2);
    // Task_Create_Period(UpdateArm, 0, 10, 2, 5);


    // This function was called by the OS as a System task.
    // If a task executes a return statement it is terminated.
    // Could also call Kernel_Request(TERMINATE) here.

    return;
}
