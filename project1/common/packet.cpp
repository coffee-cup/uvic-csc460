#include "packet.h"

#include <stdint.h>

Packet create_packet(uint16_t speedX, uint16_t speedY, uint8_t laserOn) {
    Packet p = {
        .speedX = speedX,
        .speedY = speedY,
        .laserOn = laserOn
    };

    return p;
}
