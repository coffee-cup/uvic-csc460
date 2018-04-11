#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <stdlib.h>

#define PACKET_MAGIC 0x41
#define TO16BIT(h, l) (h << 8 | l)
#define PACKET_SIZE 10

struct Packet {
    union {
        struct {
            uint16_t joy1X;
            uint16_t joy1Y;
            uint16_t joy2X;
            uint16_t joy2Y;
            uint8_t joy1SW;
            uint8_t joy2SW;
        } field;
        uint8_t data[PACKET_SIZE];
    };

    Packet(uint8_t* buffer) {
        this->field.joy1X = TO16BIT(buffer[1], buffer[0]);
        this->field.joy1Y = TO16BIT(buffer[3], buffer[2]);

        this->field.joy2X = TO16BIT(buffer[5], buffer[4]);
        this->field.joy2Y = TO16BIT(buffer[7], buffer[6]);

        this->field.joy1SW = buffer[8];
        this->field.joy2SW = buffer[9];
    };

    Packet(uint16_t joy1X, uint16_t joy1Y, uint8_t joy1SW, uint16_t joy2X, uint16_t joy2Y, uint8_t joy2SW) {
        this->field.joy1X = joy1X;
        this->field.joy1Y = joy1Y;
        this->field.joy1SW = joy1SW;
        this->field.joy2X = joy2X;
        this->field.joy2Y = joy2Y;
        this->field.joy2SW = joy2SW;
    };
};

#endif
