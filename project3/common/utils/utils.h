#ifndef _UTILS_H_
#define _UTILS_H_

#include <avr/io.h>
#include "common.h"

// Maps the value x which is in range [in_min, in_max] to [out_min, out_max]
long map_u(long x, long in_min, long in_max, long out_min, long out_max);

// Constrains the value x to [min, max]
long constrain_u(long x, long min, long max);

// Initializes all analog ports on the board
void analog_init();

// Reads an analog signal from a channel
uint16_t analog_read(uint8_t channel);

#endif
