#include <avr/io.h>
#include <util/delay.h>
#include "Roomba.h"
#include "Packet.h"
#include "Arm.h"
#include "Joystick.h"
#include "Motor.h"

extern "C" {
    #include "kernel.h"
    #include "os.h"
    #include "common.h"
    #include "uart.h"
    #include "utils.h"
    #include "timings.h"
    #include "move.h"
    void create(void);
}

#define DEAD_SONG 0
#define FREE_SONG 1
#define STAY_SONG 2
#define START_SONG 3

Roomba roomba(/*Serial*/ 3, /*Port A pin*/ 0);
Packet packet(512, 512, 0, 512, 512, 0);

Arm arm;
Joystick joy1(15, 14, 2);
Joystick joy2(13, 12, 3);

void choose_move(Move* move) {
    bool is_wall = roomba.check_virtual_wall();
    bool is_leftb = roomba.check_left_bumper();
    bool is_rightb = roomba.check_right_bumper();

    // Set base move to stop
    move_change(move, STOP, 0);

    if (is_wall) {
        move_change(move, BACKWARD, 50);
        return;
    }

    if (is_leftb && is_rightb) {
        move_change(move, BACKWARD, 50);
        return;
    }

    if (is_leftb) {
        move_change(move, RIGHT, 50);
        return;
    }

    if (is_rightb) {
        move_change(move, LEFT, 50);
        return;
    }

    int x = cmap_u(joy1.getX(), 0, 1023, -100, 100);
    if (abs_u(x) <= 5) x = 0;
    if (x < 10) {
        move_change(move, LEFT, abs_u(x));
    } else if (x > 10) {
        move_change(move, RIGHT, x);
    }
}

// Weak attribute allows other functions to redefine
// We want kernel main to be the actual main
DELEGATE_MAIN()

void ArmMove(void) TASK ({
    arm.setSpeedX(Arm::filterSpeed(joy1.getX()));
    arm.setSpeedY(Arm::filterSpeed(joy2.getY()));
})

void Tick(void) {
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

void getData(void) TASK ({
    packet.field.joy1X = joy1.getX();
    packet.field.joy1Y = joy1.getY();
    packet.field.joy1SW = joy1.getClick() == 1 ? 0xFF : 0x00;

    // LOG("%d\n", packet.field.joy1X);
})

void songs() {
    uint8_t d = 16;
    uint8_t s = 6;

    // Death song
    uint8_t dead_song[] = {60, d, 59, d, 57, d, 55, d, 53, d, 52, d, 50, d, 48, d * 5};
    roomba.set_song(DEAD_SONG, 8, dead_song);

    d = 8;
    uint8_t stay_song[] = {95, d, 100, d * 6};
    roomba.set_song(STAY_SONG, 2, stay_song);

    uint8_t free_song[] = {79, d, 83, d, 86, d, 91, d, 95, d, 80, s, 84, d, 87, s, 92, s, 96, d, 82, s, 86, d, 89, d, 94, s, 98, s};
    roomba.set_song(FREE_SONG, 15, free_song);

    uint8_t start_song[] = {88, d, 91, d * 3, 100, d, 96, d, 98, d * 2, 103, d};
    roomba.set_song(START_SONG, 6, start_song);
}

void commandRoomba() {
    roomba.init();
    // if (!roomba.init()) {
    //     OS_Abort(FAILED_START);
    // }

    roomba.set_mode(Roomba::OI_MODE_TYPE::SAFE_MODE);
    songs();
    roomba.play_song(START_SONG);

    Move move;
    for (;;) {
        // choose_move(&move);

        uint16_t dir = 0;
        uint16_t speed = move.speed;

        switch (move.dir) {
        case FORWARD:
            dir = STRAIGHT;
            break;
        case BACKWARD:
            dir = STRAIGHT;
            speed *= -1;
            break;
        case RIGHT:
            dir = -1;
            break;
        case LEFT:
            dir = 1;
            break;
        case STOP:
            speed = 0;
            break;
        }

        // speed = roomba_speed(speed);
        // roomba.drive(speed, dir);

        Task_Next();
    }
}

void create(void) {

    // Outputs for Tasks's
    BIT_SET(DDRB, 0);
    BIT_SET(DDRB, 1);
    BIT_SET(DDRC, 0); // Laser

    BIT_CLR(PORTB, 0);
    BIT_CLR(PORTB, 1);
    BIT_CLR(PORTC, 0);

    arm.attach(2, 3);

    // Task_Create_Period(ArmMove, 0, 10, 2, 1);
    // Task_Create_Period(Tick, 0, ARM_TICK_PERIOD, ARM_TICK_WCET, ARM_TICK_DELAY);
    // Task_Create_Period(getData, 0, GET_DATA_PERIOD, GET_DATA_WCET, GET_DATA_DELAY);
    Task_Create_Period(commandRoomba, 0, COMMAND_ROOMBA_PERIOD, COMMAND_ROOMBA_WCET, COMMAND_ROOMBA_DELAY);

    return;
}
