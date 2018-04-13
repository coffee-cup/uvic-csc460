#ifndef _MOVE_H_
#define _MOVE_H_

#include <stdint.h>
#include "utils.h"
#include "common.h"

#define MAX_SPEED (300)
#define DEADBAND (20)

typedef struct move_type {
    int16_t left_speed;
    int16_t right_speed;
} Move;

// Stop both wheels
void stop(Move *move);

// Move both wheels forwards
void forward(Move *move, int16_t speed);

// Move both wheels backwards
void backward(Move *move, int16_t speed);

// Spin on a dime right
void spin_right(Move *move, int16_t speed);

// Spin on a dime left
void spin_left(Move *move, int16_t speed);

// Set direction of both wheels separately
void set_speeds(Move *move, int16_t left_speed, int16_t right_speed);

// Convert relative percentage based speed to absolute speed
int16_t roomba_speed(int16_t speed);

// Choose a user speed based on x and y joystick directions
void choose_user_move(Move *move, uint16_t x, uint16_t y, uint8_t mode);

#endif
