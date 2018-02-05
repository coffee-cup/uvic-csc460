#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

typedef __attribute__((__packed__)) struct {
    uint16_t speedX;
    uint16_t speedY;
    uint8_t laserOn;
} Packet;

Packet create_packet(uint16_t speedX, uint16_t speedY, uint8_t laserOn);

#endif
