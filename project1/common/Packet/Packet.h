#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t speedX;
    uint16_t speedY;
    uint8_t laserOn;
} Packet;
#pragma pack(pop)

Packet create_packet(uint8_t buffer[5]);
Packet create_packet(uint16_t speedX, uint16_t speedY, uint8_t laserOn);

#endif
