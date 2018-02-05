#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

typedef struct {
    uint8_t speedX;
    uint8_t speedY;
    uint8_t laserOn;
} Packet;

Packet create_packet(uint8_t speedX, uint8_t speedY, uint8_t laserOn);

#endif
