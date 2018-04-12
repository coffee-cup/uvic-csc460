#include "utils.h"
#include "os.h" // OS_Abort

long map_u(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

long constrain_u(long x, long min, long max) {
    if (x < min) return min;
    else if (x > max) return max;
    return x;
}

long cmap_u(long x, long in_min, long in_max, long out_min, long out_max) {
    return constrain_u(map_u(x, in_min, in_max, out_min, out_max), out_min, out_max);
}

long abs_u(long x) {
    if (x > 0) return x;
    if (x < 0) return -x;
    return 0;
}

/*
 * Use ADC on the board
 */
void analog_init() {
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

uint16_t analog_read(uint8_t channel) {
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
    BIT_SET(ADCSRA, ADSC);

    /* We wait on the ADC to complete the operation, when it completes, the hardware
       will set the ADSC bit to 0. */
    while (BIT_TEST(ADCSRA, ADSC));

    /* We setup the ADC to shift input to left, so we simply return the High register. */
    uint8_t lowADC = ADCL;
    uint8_t highADC = ADCH;
    return (lowADC >> 6) | (highADC << 2);
}

volatile uint16_t* pwm_attach(uint8_t port_pin) {
    // Enable PWM waveform generation on a PIN
    // returns the Output Compare Register for that pin

    switch (port_pin) {
        case PE4:
            // PWM pins as output
            BIT_SET(DDRE, PE4);

            // Set output compare mode
            //  - Clear output on compare match,
            //  - Set   output at bottom
            BIT_SET(TCCR3A, COM3B1);

            // Set compare match value
            // 5000 / 2000 * 150 = 375
            OCR3B = 375; // 1.5 ms pulse
            return &OCR3B;
            break;

        case PE5:
            // PWM pins as output
            BIT_SET(DDRE, PE5);

            // Set output compare mode
            //  - Clear output on compare match,
            //  - Set   output at bottom
            BIT_SET(TCCR3A, COM3C1);

            // Set compare match value
            // 5000 / 2000 * 150 = 375
            OCR3C = 375; // 1.5 ms pulse
            return &OCR3C;
            break;

        default:
            LOG("No PWM support implemented for pin %d\n", port_pin);
            OS_Abort(PWM_ERROR);
            return NULL;
    }
}

void pwm_init() {
    // Set Wave Generation mode to Fast PWM (mode 15)
    // Counter counts up and resets to BOTTOM (value 0) at TOP
    MASK_SET(TCCR3A, _BV(WGM30) | _BV(WGM31));
    MASK_SET(TCCR3B, _BV(WGM32) | _BV(WGM33));

    // Set prescaler to 64
    MASK_SET(TCCR3B, _BV(CS31) | _BV(CS30));

    // Set TOP value for a 20 ms period
    OCR3A = 5000;
}

void pwm_write(volatile uint16_t* OCR3n, uint16_t micro_seconds) {
    // Min = 5000 / 2000 * 50 = 125
    // Max = 5000 / 2000 * 250 = 625
    uint16_t value = map_u(micro_seconds, 500, 2500, 125, 625);
    *OCR3n = value;
}
