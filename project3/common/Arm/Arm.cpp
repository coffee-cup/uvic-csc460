#include "Arm.h"

extern "C" {
    #include "os.h"
    #include "utils.h"
}

Arm::Arm():
    X({
        .pos     = 90,
        .speed   = 0,
        .lastRun = Now(),
        .servo   = Motor()
    }),
    Y({
        .pos     = 90,
        .speed   = 0,
        .lastRun = Now(),
        .servo   = Motor()
    })
{ }

void Arm::attach(uint8_t pinX, uint8_t pinY) {
    X.servo.attach(pinX);
    Y.servo.attach(pinY);
}

void Arm::setServoPos(Motor servo, uint8_t pos) {
    pos = constrain_u(pos, Arm::DEG_MIN, Arm::DEG_MAX);
    servo.write(pos);
}

void Arm::setJointSpeed(Joint &joint, int8_t s) {
    joint.speed = constrain_u(s, -1 * Arm::S_MAX_SPEED, Arm::S_MAX_SPEED);
}

int8_t Arm::filterSpeed(int16_t value) {
    // Value should be a reading from the joystick [0-1023]
    int8_t speed = map_u(value, 20, 1003, -1 * Arm::S_MAX_SPEED, Arm::S_MAX_SPEED);
    speed = constrain_u(speed, -1 * Arm::S_MAX_SPEED, Arm::S_MAX_SPEED);

    // threshold
    if (abs_u(speed) <= 5) {
        speed = 0;
    }

    return speed;
}

void Arm::tick() {
    uint32_t now = Now();

    uint32_t deltaX = now - this->X.lastRun;
    uint32_t deltaY = now - this->Y.lastRun;

    // Number of ticks required to pass to update X by 1 deg
    uint16_t threshX = 100 / abs_u(this->X.speed);
    if (deltaX > threshX) {
        this->X.pos = constrain_u(
            this->X.speed < 0 ? this->X.pos - 1 : this->X.pos + 1,
            Arm::DEG_MIN,
            Arm::DEG_MAX
        );
        setServoPos(this->X.servo, this->X.pos);
        this->X.lastRun = now;
    }

    // Number of ticks required to pass to update Y by 1 deg
    uint16_t threshY = 100 / abs_u(this->Y.speed);
    if (deltaY > threshY) {
        this->Y.pos = constrain_u(
            this->Y.speed < 0 ? this->Y.pos - 1 : this->Y.pos + 1,
            Arm::DEG_MIN,
            Arm::DEG_MAX
        );
        setServoPos(this->Y.servo, this->Y.pos);
        this->Y.lastRun = now;
    }
}
