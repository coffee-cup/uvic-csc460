#include "move.h"

void stop(Move *move) {
    move->left_speed = 0;
    move->right_speed = 0;
}

void forward(Move *move, int16_t speed) {
    move->left_speed = speed;
    move->right_speed = speed;
}

void backward(Move *move, int16_t speed) {
    move->left_speed = -speed;
    move->right_speed = -speed;
}

void spin_right(Move *move, int16_t speed) {
    move->left_speed = speed;
    move->right_speed = -speed;
}

void spin_left(Move *move, int16_t speed) {
    move->left_speed = -speed;
    move->right_speed = speed;
}

void set_speeds(Move *move, int16_t left_speed, int16_t right_speed) {
    move->left_speed = left_speed;
    move->right_speed = right_speed;
}

int16_t roomba_speed(int16_t speed) {
    speed = constrain_u(speed, -100, 100);
    return speed < 0
        ? map_u(speed, -100, 0, -MAX_SPEED, 0)
        : map_u(speed, 0, 100, 0, MAX_SPEED);
}

void choose_user_move(Move *move, uint16_t x_, uint16_t y_) {
    // Map x and y from [0, 1023] to [100, 100]
    int16_t x = cmap_u(x_, 0, 1023, -100, 100);
    int16_t y = cmap_u(y_, 0, 1023, -100, 100);

    if (abs_u(x) > DEADBAND && abs_u(y) > DEADBAND) {
        // Angled drive
        x = cmap_u(x, -100, 100, -25, 25);
        int16_t left_x = 25 + x;
        int16_t right_x = 25 - x;

        if (y < 0) {
            left_x *= -1;
            right_x *= -1;
        }

        y = cmap_u(y, -100, 100, -25, 25);

        set_speeds(move,
                   cmap_u(left_x + y, -75, 75, -100, 100),
                   cmap_u(right_x + y, -75, 75, -100, 100));
    } else if (abs_u(y) > DEADBAND) {
        // Straight drive
        forward(move, y);
    } else if (abs_u(x) > DEADBAND) {
        // Spin drive
        spin_right(move, x);
    } else {
        // Stop
        stop(move);
    }
}
