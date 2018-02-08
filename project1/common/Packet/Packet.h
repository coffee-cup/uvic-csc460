#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <stdlib.h>

struct Packet {
    union {
        struct {
            uint16_t joy1X :10;
            uint16_t joy1Y :10;
            uint16_t joy2X :10;
            uint16_t joy2Y :10;
            uint8_t joy1SW : 1;
            uint8_t joy2SW : 1;
        } field;
        uint8_t data[10];
    };

    Packet(uint8_t (&buffer)[10]) {
        for (uint8_t i = 0; i < 10; i += 1){
            this->data[i] = buffer[i];
        }
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
