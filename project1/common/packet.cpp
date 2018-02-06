#include "packet.h"

#include <stdint.h>

Packet create_packet(uint8_t buffer[5]) {
    return create_packet(
        (((uint16_t)buffer[1]) << 8) | buffer[0],
        (((uint16_t)buffer[3]) << 8) | buffer[2],
        buffer[4]
    );
}

Packet create_packet(uint16_t speedX, uint16_t speedY, uint8_t laserOn) {
    Packet p = {
        .speedX = speedX,
        .speedY = speedY,
        .laserOn = laserOn
    };

    return p;
}
