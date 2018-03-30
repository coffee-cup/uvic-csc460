#include "utils.h"

long map_u(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

long constrain_u(long x, long min, long max) {
    if (x < min) return min;
    else if (x > max) return max;
    return x;
}

void analog_init() {
    /*
     * Use ADC on the board
     */

    // Configure analog inputs using ADC
    BIT_SET(ADCSRA, ADPS2);
    BIT_SET(ADCSRA, ADPS1);
    BIT_SET(ADCSRA, ADPS0);

    // Set ADC reference to AVCC
    BIT_SET(ADMUX, REFS0);

    // Left adjust ADC result to allow easy 8 bit reading
    BIT_SET(ADMUX, ADLAR);

    // Enable ADC
    BIT_SET(ADCSRA, ADEN);

    // Start a conversion to warmup the ADC
    BIT_SET(ADCSRA, ADSC);

    // Set PORTC to input for the switch
    PORTC = 0xFF;

}

// Reads an analog signal from a channel
uint16_t analog_read(int channel) {
    /* We're using Single Ended input for our ADC readings, this requires some
     * work to correctly set the mux values between the ADMUX and ADCSRB registers.
     * ADMUX contains the four LSB of the multiplexer, while the fifth bit is kept
     * within the ADCSRB register. Given the specifications, we want to keep the
     * three least significant bits as is, and check to see if the fourth bit is set, if it
     * is, we need to set the mux5 pin.
     */

    /* Set the three LSB of the Mux value. */
    /* Caution modifying this line, we want MUX4 to be set to zero, always */
    ADMUX = (ADMUX & 0xF0 ) | (0x07 & channel);

    /* We set the MUX5 value based on the fourth bit of the channel, see page 292 of the
     * ATmega2560 data sheet for detailed information */
    ADCSRB = (ADCSRB & 0xF7) | (channel & (1 << MUX5));

    /* We now set the Start Conversion bit to trigger a fresh sample. */
    ADCSRA |= (1 << ADSC);

    /* We wait on the ADC to complete the operation, when it completes, the hardware
       will set the ADSC bit to 0. */
    while ((ADCSRA & (1 << ADSC)));

    /* We setup the ADC to shift input to left, so we simply return the High register. */
    int lowADC = ADCL;
    int highADC = ADCH;
    return (lowADC>>6) | (highADC<<2);
}
