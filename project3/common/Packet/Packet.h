#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <stdlib.h>

#define PACKET_MAGIC 0xFEED
#define TO16BIT(h, l) ((uint16_t)((h & 0xFF) << 8 | (l & 0xFF)))
#define PACKET_SIZE 12

class Packet {
  public:
    uint8_t data[PACKET_SIZE];

    Packet(uint8_t* buffer);
    Packet(uint16_t joy1X, uint16_t joy1Y, uint8_t joy1SW, uint16_t joy2X, uint16_t joy2Y, uint8_t joy2SW);

    uint16_t magic();

    uint16_t joy1X();
    void joy1X(uint16_t);

    uint16_t joy1Y();
    void joy1Y(uint16_t);

    uint16_t joy2X();
    void joy2X(uint16_t);

    uint16_t joy2Y();
    void joy2Y(uint16_t);

    uint8_t joy1SW();
    void joy1SW(uint8_t);

    uint8_t joy2SW();
    void joy2SW(uint8_t);
};

#endif
