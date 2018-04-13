#include <avr/io.h>
#include <util/delay.h>
#include "Roomba.h"
#include "Packet.h"
#include "Arm.h"
#include "Joystick.h"
#include "Motor.h"
#include "Packet.h"

extern "C" {
    #include "kernel.h"
    #include "os.h"
    #include "common.h"
    #include "utils.h"
    #include "uart.h"
    #include "timings.h"
    #include "move.h"
    void create(void);
}

#define DEAD_SONG 0
#define LASER_SONG 0
#define FREE_SONG 1
#define STAY_SONG 2
#define START_SONG 3

Roomba roomba(/*Serial*/ 3, /*Port A pin*/ 0);
Packet packet(512, 512, 0, 512, 512, 0);

Arm arm;
Joystick joy1(15, 14, 2);
Joystick joy2(13, 12, 3);

volatile uint8_t mode = NO_MODE;
volatile bool game_on = false;

void choose_move(Move* move) {
    bool is_wall = roomba.check_virtual_wall();
    bool is_leftb = roomba.check_left_bumper();
    bool is_rightb = roomba.check_right_bumper();

    // Set base move to stop
    stop(move);

    if (is_wall) {
        backward(move, 50);
        return;
    }

    if (is_leftb && is_rightb) {
        backward(move, 50);
        return;
    }

    if (is_leftb) {
        spin_right(move, 50);
        return;
    }

    if (is_rightb) {
        spin_left(move, 50);
        return;
    }

    choose_user_move(move, joy1.getX(), 1023 - joy1.getY(), mode);
}

// Weak attribute allows other functions to redefine
// We want kernel main to be the actual main
DELEGATE_MAIN()

/**
 * A simple periodic task to update the data for the pan and tilt kit
 */
void UpdateArm(void) {
    arm.attach(2, 3);

    TASK({
        arm.setSpeedX(Arm::filterSpeed(packet.joy1X()));
        arm.setSpeedY(Arm::filterSpeed(packet.joy1Y()));
    })
}

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

void modeChange(void) {
    mode = (mode == FREE_MODE)
        ? STAY_MODE
        : FREE_MODE;
    LOG("mode %d\n", mode);
}

void start_game() {
    if (!game_on) {
        game_on = true;
        mode = STAY_MODE;
        roomba.play_song(START_SONG);
        // Task_Create_Period(modeChange, 0, MODE_PERIOD, MODE_WCET, MODE_DELAY);
    }
}

void getData(void) TASK ({
    // packet.field.joy1X = joy1.getX();
    // packet.field.joy1Y = joy1.getY();
    // packet.field.joy1SW = joy1.getClick() == 1 ? 0xFF : 0x00;

    // LOG("%d\n", packet.field.joy1X);
})

void load_songs() {
    uint8_t d = 8;
    uint8_t s = 6;

    uint8_t laser_song[] = {100, d};
    roomba.set_song(LASER_SONG, 1, laser_song);

    uint8_t stay_song[] = {95, d, 100, d * 4};
    roomba.set_song(STAY_SONG, 2, stay_song);

    uint8_t free_song[] = {79, d, 83, d, 86, d, 91, d, 95, d, 80, s, 84, d, 87, s, 92, s, 96, d, 82, s, 86, d, 89, d, 94, s, 98, s};
    roomba.set_song(FREE_SONG, 15, free_song);

    uint8_t start_song[] = {88, d, 91, d * 3, 100, d, 96, d, 98, d * 2, 103, d};
    roomba.set_song(START_SONG, 6, start_song);
}

void die(void) {
    // Death song
    uint8_t d = 16;
    uint8_t dead_song[] = {60, d, 59, d, 57, d, 55, d, 53, d, 52, d, 50, d, 48, d * 5};
    roomba.set_song(DEAD_SONG, 8, dead_song);
    roomba.play_song(DEAD_SONG);
}

// A RR task to play a sound for how much laser we have left
void laser_strength(uint8_t n) {
    // uint8_t n = Task_GetArg();
    for (int i = 0; i < n; i += 1) {
        roomba.play_song(LASER_SONG);
    }
}

void commandRoomba() {
    roomba.init();
    // if (!roomba.init()) {
    //     OS_Abort(FAILED_START);
    // }

    roomba.set_mode(Roomba::OI_MODE_TYPE::SAFE_MODE);
    load_songs();

    laser_strength(4);
    // roomba.play_song(_SONG);

    Move move;
    for (;;) {
        if (joy1.getClick() && !game_on) {
            start_game();
        }

        choose_move(&move);
        // LOG("%d\t%d\n", move.left_speed, move.right_speed);

        int16_t left_speed = roomba_speed(move.left_speed);
        int16_t right_speed = roomba_speed(move.right_speed);

        roomba.direct_drive(left_speed, right_speed);

        Task_Next();
    }
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

    // Task_Create_Period(ArmMove, 0, 10, 2, 1);
    // Task_Create_Period(Tick, 0, ARM_TICK_PERIOD, ARM_TICK_WCET, ARM_TICK_DELAY);

    Task_Create_Period(RXData, 0, GET_DATA_PERIOD, GET_DATA_WCET, GET_DATA_DELAY);
    Task_Create_Period(logPacket, 0, 10, 5, 15);
    Task_Create_Period(commandRoomba, 0, COMMAND_ROOMBA_PERIOD, COMMAND_ROOMBA_WCET, COMMAND_ROOMBA_DELAY);

    // This function was called by the OS as a System task.
    // If a task executes a return statement it is terminated.
    // Could also call Kernel_Request(TERMINATE) here.

    return;
}
