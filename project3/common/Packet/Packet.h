#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <stdlib.h>
#include "common.h"

#define PACKET_MAGIC 0xFEED
#define TO16BIT(h, l) ((uint16_t)((h & 0xFF) << 8 | (l & 0xFF)))

#define PACKET_VALUE_ASSIGN(i, value) \
    this->data[i] = HIGH_BYTE(value); this->data[i+1] = LOW_BYTE(value);

#define PACKET_MAGIC_SIZE    1*sizeof(PACKET_MAGIC)
#define PACKET_XY_SIZE       4*sizeof(uint16_t)
#define PACKET_SW_SIZE       2*sizeof(uint8_t)
#define PACKET_CHECKSUM_SIZE 1*sizeof(uint16_t)

#define PACKET_SIZE PACKET_MAGIC_SIZE + PACKET_XY_SIZE + PACKET_SW_SIZE + PACKET_CHECKSUM_SIZE


class Packet {
  public:
    uint8_t data[PACKET_SIZE];

    Packet(uint8_t* buffer);
    Packet(uint16_t joy1X, uint16_t joy1Y, uint8_t joy1SW, uint16_t joy2X, uint16_t joy2Y, uint8_t joy2SW);

    /* Inline Packet field getters */
    inline uint16_t    magic() { return TO16BIT(this->data[0], this->data[1]); }
    inline uint16_t    joy1X() { return TO16BIT(this->data[2], this->data[3]); }
    inline uint16_t    joy1Y() { return TO16BIT(this->data[4], this->data[5]); }
    inline uint16_t    joy2X() { return TO16BIT(this->data[6], this->data[7]); }
    inline uint16_t    joy2Y() { return TO16BIT(this->data[8], this->data[9]); }
    inline uint8_t    joy1SW() { return this->data[10]; }
    inline uint8_t    joy2SW() { return this->data[11]; }
    inline uint16_t checksum() { return TO16BIT(this->data[12], this->data[13]); }

    /* Inline Packet field setters */
    inline void joy1X(uint16_t value) { PACKET_VALUE_ASSIGN(2, value); updateChecksum(); }
    inline void joy1Y(uint16_t value) { PACKET_VALUE_ASSIGN(4, value); updateChecksum(); }
    inline void joy2X(uint16_t value) { PACKET_VALUE_ASSIGN(6, value); updateChecksum(); }
    inline void joy2Y(uint16_t value) { PACKET_VALUE_ASSIGN(8, value); updateChecksum(); }
    inline void joy1SW(uint8_t value) { this->data[10] = value;        updateChecksum(); }
    inline void joy2SW(uint8_t value) { this->data[11] = value;        updateChecksum(); }

    void updateChecksum();
};

#endif
