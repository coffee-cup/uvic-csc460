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
    #include "timings.h"
    void create(void);
}

// Weak attribute allows other functions to redefine
// We want kernel main to be the actual main
DELEGATE_MAIN()

/**
 * A simple periodic task to update the data for the pan and tilt kit
 */
void UpdateArm(void) TASK ({
    arm.setSpeedX(Arm::filterSpeed(packet.joy1X()));
    arm.setSpeedY(Arm::filterSpeed(packet.joy1Y()));
})

/**
 * A simple periodic task to update the pan and tilt kit's position
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

/**
 * A Periodic task to receive packets from a sender.
 */
void RXData(void) {
    uint8_t data_channel = 2;
    uint8_t buffer[PACKET_SIZE];
    uint8_t byte, i;
    bool magic_ok, checksum_ok;
    Packet rx_packet = packet;
    UART_Init(data_channel, 38400);

    TASK({
        BIT_SET(PORTB, 0);
        if (UART_Available(data_channel)) {


            // Available byte is the high byte of packet magic
            if (UART_Async_Receive(data_channel, &byte) && byte == HIGH_BYTE(PACKET_MAGIC)) {
                buffer[0] = byte;

                // Next available byte is low byte of packet magic
                if (UART_Async_Receive(data_channel, &byte) && byte == LOW_BYTE(PACKET_MAGIC)) {
                    buffer[1] = byte;

                    // Rest of packet is available
                    if (UART_BytesAvailable(data_channel, PACKET_SIZE - PACKET_MAGIC_SIZE)) {

                        // Read in rest of packet
                        for (i = 2; i < PACKET_SIZE; i += 1) {
                            if (UART_Async_Receive(data_channel, &byte)){
                                buffer[i] = byte;
                            } else {
                                LOG("Expected a packet to be available!\n");
                                buffer[i] = 0;
                            }
                        }

                        // Store buffer as packet, automatically
                        // calculates the checksum on it's own
                        rx_packet = Packet(buffer);

                        magic_ok = (rx_packet.magic() == PACKET_MAGIC &&
                                    rx_packet.magic() == TO16BIT(
                                        buffer[0],
                                        buffer[1]
                                    )
                        );

                        checksum_ok = rx_packet.checksum() == TO16BIT(
                            buffer[PACKET_SIZE - 2],
                            buffer[PACKET_SIZE - 1]
                        );

                        // Sanity check, magic and checksum match what came in
                        if (magic_ok && checksum_ok) {
                            packet = Packet(buffer);
                        }

                        BIT_CLR(PORTB, 0);
                        // Hacky continue, fall through for everything else
                        // causes UART to flush the RX buffer. We don't actually
                        // want to flush if we just got a packet
                        continue;
                    } /* Packet remainder available */
                } /* Rx low byte */
            } /* Rx high byte */

            UART_Flush(data_channel);

        } /* Rx available */

        BIT_CLR(PORTB, 0);
    })
}

void logPacket(void) TASK({
    BIT_SET(PORTB, 1);
    LOG(">> [%X]:[%u]:[%u]:[%u]:[%u]:[%c]:[%c]:{0x%X}\n", packet.magic(),
        packet.joy1X(), packet.joy1Y(),
        packet.joy2X(), packet.joy2Y(),
        packet.joy1SW() ? '#' : '/',
        packet.joy2SW() ? '#' : '/',
        packet.checksum());
    BIT_CLR(PORTB, 1);
})


/**
 *
 */
void create(void)
{
    // Outputs for Tasks's
    BIT_SET(DDRB, 0);
    BIT_SET(DDRB, 1);
    BIT_SET(DDRB, 2);

    BIT_CLR(PORTB, 0);
    BIT_CLR(PORTB, 1);
    BIT_CLR(PORTB, 2);

    // Laser ouput
    BIT_SET(DDRC, 0);
    BIT_CLR(PORTC, 0);

    arm.attach(2, 3);

    Task_Create_Period(RXData, 0, 10, 9, 10);
    Task_Create_Period(logPacket, 0, 10, 5, 15);

    // Task_Create_Period(TickArm, 0, 2, 1, 2);
    // Task_Create_Period(UpdateArm, 0, 10, 2, 5);


    // This function was called by the OS as a System task.
    // If a task executes a return statement it is terminated.
    // Could also call Kernel_Request(TERMINATE) here.

    return;
}
