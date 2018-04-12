#ifndef _MOTOR_H_
#define _MOTOR_H_

#include <stdint.h>
#include <avr/io.h>

class Motor {

  public:
    Motor();

    void attach(uint8_t pin);
    void write(uint8_t pos);

  private:
    static const uint16_t MIN_PULSE = 544;
    static const uint16_t MAX_PULSE = 2400;
    volatile uint16_t* pwm_ocr;

};

#endif
