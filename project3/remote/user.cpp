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

void commandRoomba() {
    if (!roomba.init()) {
        OS_Abort(FAILED_START);
    }

    roomba.set_mode(Roomba::OI_MODE_TYPE::SAFE_MODE);

    Move move;
    for (;;) {
        choose_move(&move);

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

        speed = roomba_speed(speed);
        roomba.drive(speed, dir);

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
