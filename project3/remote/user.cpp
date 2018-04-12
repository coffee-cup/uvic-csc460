#include <avr/io.h>
#include <util/delay.h>
#include "Roomba.h"
#include "Packet.h"

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

void getData(void);
void commandRoomba(void);

void choose_move(Move* move) {
    bool is_wall = roomba.check_virtual_wall();
    bool is_leftb = roomba.check_left_bumper();
    bool is_rightb = roomba.check_right_bumper();

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

    // Choose user move based on packet
    move_change(move, STOP, 50);
}

int main(void) {
    Kernel_Begin();
}

void create(void) {

    // Outputs for Tasks's
    BIT_SET(DDRB, 0);
    BIT_SET(DDRB, 1);
    BIT_CLR(PORTB, 0);
    BIT_CLR(PORTB, 1);

    if (!roomba.init()) {
        OS_Abort(FAILED_START);
    }

    // Task_Create_Period(getData, 0, GET_DATA_PERIOD, GET_DATA_WCET, GET_DATA_DELAY);
    Task_Create_Period(commandRoomba, 0, COMMAND_ROOMBA_PERIOD, COMMAND_ROOMBA_WCET, COMMAND_ROOMBA_DELAY);
    // Task_Create_Period(Move, 0, 100, 20, 5);

    return;
}

void commandRoomba() {
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
