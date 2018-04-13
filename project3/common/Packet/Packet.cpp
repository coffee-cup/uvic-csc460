
#include "Packet.h"


/**
 * Construct a packet from a buffer
 * All packet fields are set to 0 if the buffer doesn't start with PACKET_MAGIC
 */
Packet::Packet(uint8_t* buffer)
{
    if (TO16BIT(buffer[0], buffer[1]) == PACKET_MAGIC) {
        memcpy(this->data, buffer, PACKET_SIZE);
        uint16_t checksum = TO16BIT(this->data[12], this->data[13]);
        updateChecksum();
        if (TO16BIT(this->data[12], this->data[13]) != checksum) {
            // Calculated checksum doesn't match what was rx'd
            ZeroMemory(this->data, PACKET_SIZE);
        }
    } else {
        ZeroMemory(this->data, PACKET_SIZE);
    }
}

/**
 * Construct a packet from individual values
 * Always produces a valid packet, and inserts PACKET_MAGIC at the first two bytes
 */
Packet::Packet(uint16_t joy1X, uint16_t joy1Y, uint8_t joy1SW, uint16_t joy2X, uint16_t joy2Y, uint8_t joy2SW)
{
    this->data[0] = HIGH_BYTE(PACKET_MAGIC);
    this->data[1] = LOW_BYTE(PACKET_MAGIC);

    this->data[2] = HIGH_BYTE(joy1X);
    this->data[3] = LOW_BYTE(joy1X);

    this->data[4] = HIGH_BYTE(joy1Y);
    this->data[5] = LOW_BYTE(joy1Y);

    this->data[6] = HIGH_BYTE(joy2X);
    this->data[7] = LOW_BYTE(joy2X);

    this->data[8] = HIGH_BYTE(joy2Y);
    this->data[9] = LOW_BYTE(joy2Y);

    this->data[10] = LOW_BYTE(joy1SW);
    this->data[11] = LOW_BYTE(joy2SW);

    updateChecksum();
}

void Packet::updateChecksum() {
    uint8_t* data = this-> data;
    uint16_t len  = PACKET_SIZE - 2;

    // Fletcher's checksum
    // https://en.wikipedia.org/wiki/Fletcher%27s_checksum

    uint32_t c0, c1;
    unsigned int i;

    for (c0 = c1 = 0; len >= 5802; len -= 5802) {
            for (i = 0; i < 5802; ++i) {
                    c0 = c0 + *data++;
                    c1 = c1 + c0;
            }
            c0 = c0 % 255;
            c1 = c1 % 255;
    }

    for (i = 0; i < len; ++i) {
            c0 = c0 + *data++;
            c1 = c1 + c0;
    }
    c0 = c0 % 255;
    c1 = c1 % 255;

    // uint16_t checksum = (c1 << 8) | c0;
    this->data[12] = c1; // HIGH_BYTE(checksum);
    this->data[13] = c0; // LOW_BYTE(checksum);
}
