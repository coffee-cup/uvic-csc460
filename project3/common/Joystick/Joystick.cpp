#include "Joystick.h"

Joystick::Joystick(uint8_t pinForX, uint8_t pinForY, uint8_t pinForClick)
    : pinX(pinForX),
      pinY(pinForY),
      pinSW(pinForClick) {

    analog_init();

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

uint16_t Joystick::getX() {
    this->rawX = analog_read(this->pinX);
    return reduceRange(this->rawX);
}

uint16_t Joystick::getY() {
    this->rawY = analog_read(this->pinY);
    return reduceRange(this->rawY);
}

uint8_t Joystick::getClick() {
    // this->rawSW = !digitalRead(this->pinSW);
    // return this->rawSW;
    return 0;
}
