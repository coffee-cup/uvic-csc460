#ifndef JOYSTICK_H
#define JOYSTICK_H

class Joystick {
  protected:
    uint8_t pinX;
    uint8_t pinY;
    uint8_t pinSW;


    uint16_t reduceRange(uint16_t value);

    static const uint16_t MAX_ADC  = 1023;
    static const uint16_t DEADBAND = 20;

  public:
    Joystick(uint8_t pinX, uint8_t pinY, uint8_t pinSW);

    uint16_t rawX;
    uint16_t rawY;
    uint8_t  rawSW;

    uint16_t getX();
    uint16_t getY();
    uint8_t  getClick();
};

#endif
