#include "move.h"

void move_change(Move *move, DIRECTION dir, int16_t speed) {
    move->dir = dir;
    move->speed = constrain_u(speed, 0, 100);
}

int16_t roomba_speed(int16_t speed) {
    return speed < 0
        ? map_u(speed, -100, 0, -MAX_SPEED, 0)
        : map_u(speed, 0, 100, 0, MAX_SPEED);
}
