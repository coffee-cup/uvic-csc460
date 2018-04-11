
#include "Motor.h"

extern "C" {
    #include "os.h"
    #include "common.h"
    #include "utils.h"
}

Motor::Motor() {
    pwm_init();
}

void Motor::attach(uint8_t arduino_pin) {
    switch (arduino_pin) {
        case 2:
            this->pwm_ocr = pwm_attach(PE4);
            break;

        case 3:
            this->pwm_ocr = pwm_attach(PE5);
            break;

        default:
            LOG("No PWM support for pin %d\n", arduino_pin);
            OS_Abort(PWM_ERROR);
    }
}

void Motor::write(uint8_t angle) {
    uint16_t pos = constrain_u(angle, 0, 180);
    pos = map_u(pos, 0, 180, Motor::MIN_PULSE, Motor::MAX_PULSE);
    pwm_write(this->pwm_ocr, pos);
}
