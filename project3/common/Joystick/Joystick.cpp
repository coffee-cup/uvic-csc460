#include "Joystick.h"

Joystick::Joystick(uint8_t pinForX, uint8_t pinForY, uint8_t pinForClick)
    : pinX(pinForX),
      pinY(pinForY),
      pinSW(pinForClick) {

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

    // pinMode(this->pinSW, INPUT_PULLUP);
}

uint16_t Joystick::reduceRange(uint16_t value) {
    // Value should be a reading from the joystick, [0-1023]
    // Bring the max and min readings in from the edge of the joystick's range by `Joystick::DEADBAND` units.
    // This can be used to compensate for a 3D printed joystick mount reducing the range of the joystick's movement.
    // Returns a value from [0-1023], for input from [20-1003]

    // Run the internal calculation as a signed number
    // since the result of map for values < `Joystick::DEADBAND` will be < 0.
    int32_t result = map_u(
                         value,
                         0 + Joystick::DEADBAND, Joystick::MAX_ADC - Joystick::DEADBAND,
                         0, Joystick::MAX_ADC
                         );

    result = constrain_u(result, 0, Joystick::MAX_ADC);

    return result;
}

uint16_t Joystick::analogRead(uint16_t channel) {
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

uint16_t Joystick::getX() {
    this->rawX = analogRead(this->pinX);
    return reduceRange(this->rawX);
}

uint16_t Joystick::getY() {
    this->rawY = analogRead(this->pinY);
    return reduceRange(this->rawY);
}

uint8_t Joystick::getClick() {
    // this->rawSW = !digitalRead(this->pinSW);
    // return this->rawSW;
    return 0;
}
