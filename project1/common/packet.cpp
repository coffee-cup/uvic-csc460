#include "packet.h"

#include <stdint.h>

Packet create_packet(uint8_t speedX, uint8_t speedY, uint8_t laserOn) {
    Packet p = {
        .speedX = speedX,
        .speedY = speedY,
        .laserOn = laserOn
    };

    return p;
}
