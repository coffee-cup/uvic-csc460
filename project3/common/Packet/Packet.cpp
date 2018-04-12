
#include "Packet.h"
#include "common.h"

/**
 * Construct a packet from a buffer
 * All packet fields are set to 0 if the buffer doesn't start with PACKET_MAGIC
 */
Packet::Packet(uint8_t* buffer)
{
    if (TO16BIT(buffer[0], buffer[1]) == PACKET_MAGIC) {
        memcpy(this->data, buffer, PACKET_SIZE);
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
}

/** Packet magic value getter */
uint16_t Packet::magic() {
    return TO16BIT(this->data[0], this->data[1]);
}


/** Packet Joy1X field getter */
uint16_t Packet::joy1X() {
    return TO16BIT(this->data[2], this->data[3]);
}

/** Packet Joy1X field setter */
void Packet::joy1X(uint16_t value) {
    this->data[2] = HIGH_BYTE(value);
    this->data[3] = LOW_BYTE(value);
}


/** Packet Joy1Y field getter */
uint16_t Packet::joy1Y() {
    return TO16BIT(this->data[4], this->data[5]);
}

/** Packet Joy1Y field setter */
void Packet::joy1Y(uint16_t value) {
    this->data[4] = HIGH_BYTE(value);
    this->data[5] = LOW_BYTE(value);
}


/** Packet Joy2X field getter */
uint16_t Packet::joy2X() {
    return TO16BIT(this->data[6], this->data[7]);
}

/** Packet Joy2X field setter */
void Packet::joy2X(uint16_t value) {
    this->data[6] = HIGH_BYTE(value);
    this->data[7] = LOW_BYTE(value);
}


/** Packet Joy2Y field getter */
uint16_t Packet::joy2Y() {
    return TO16BIT(this->data[8], this->data[9]);
}

/** Packet Joy2Y field setter */
void Packet::joy2Y(uint16_t value) {
    this->data[8] = HIGH_BYTE(value);
    this->data[9] = LOW_BYTE(value);
}


/** Packet Joy1SW field getter */
uint8_t Packet::joy1SW() {
    return this->data[10];
}

/** Packet Joy1SW field setter */
void Packet::joy1SW(uint8_t value) {
    this->data[10] = value;
}


/** Packet Joy2SW field getter */
uint8_t Packet::joy2SW() {
    return this->data[11];
}

/** Packet Joy2SW field setter */
void Packet::joy2SW(uint8_t value) {
    this->data[11] = value;
}

