#ifndef _MOVE_H_
#define _MOVE_H_

#include <stdint.h>
#include "utils.h"

#define MAX_SPEED (200)

typedef enum {
    STOP = 0,
    FORWARD,
    BACKWARD,
    RIGHT,
    LEFT
} DIRECTION;

typedef struct move_type {
    DIRECTION dir;
    int16_t speed;
} Move;

void move_change(Move *move, DIRECTION dir, int16_t speed);
int16_t roomba_speed(Move *move);

#endif
