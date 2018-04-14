#ifndef ARM_H
#define ARM_H

#include "Motor.h"

typedef struct {
    uint8_t pos;
    int8_t speed;
    uint32_t lastRun;
    Motor servo;
} Joint;

class Arm {
  protected:
    static const uint8_t DEG_MIN = 1;
    static const uint8_t DEG_MAX = 179;
    static const uint8_t S_MAX_SPEED = 15; // deg / sec

    void setServoPos(Motor, uint8_t);
    void setJointSpeed(Joint &joint, int8_t s);

  public:
    Joint X;
    Joint Y;

    Arm();

    inline void setSpeedX(int8_t s) { setJointSpeed(this->Y, s); };
    inline void setSpeedY(int8_t s) { setJointSpeed(this->X, s); };

    void tick();
    void attach(uint8_t pinForX, uint8_t pinForY);
    static int8_t filterSpeed(int16_t value);
};

#endif
