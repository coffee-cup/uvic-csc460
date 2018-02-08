#ifndef ARM_H
#define ARM_H

/* #include <Arduino.h> */
#include <Arduino.h>
#include <Servo.h>

typedef struct {
    uint8_t pos;
    int8_t speed;
    uint32_t lastRun;
    Servo servo;
} Joint;

class Arm {
  protected:
    static const uint8_t DEG_MIN = 1;
    static const uint8_t DEG_MAX = 179;
    static const uint8_t S_MAX_SPEED = 30; // deg / sec

    void setServoPos(Servo, uint8_t);
    void setServoSpeed(Joint &joint, int8_t s);

  public:

    Joint X;
    Joint Y;

    Arm();

    inline void setSpeedX(int8_t s) { setServoSpeed(this->Y, s); };
    inline void setSpeedY(int8_t s) { setServoSpeed(this->X, s); };

    void tick();
    void attach(uint8_t pinForX, uint8_t pinForY);
    static int8_t filterSpeed(int16_t value);
};

#endif
