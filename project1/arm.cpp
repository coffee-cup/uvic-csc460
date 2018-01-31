#include "arm.h"

#include <Arduino.h>
#include <Servo.h>

unsigned long last_run;

Arm init_Arm(int pinX, int pinY) {
    Servo servoX;
    Servo servoY;
    Arm arm = (Arm) {
        pinX,
        pinY,
        90, // posX
        90, // posY
        0,  // speedX
        0,  // speedY
        0,  // lastRunX
        0,  // lastRunY
        servoX,
        servoY
    };

    servoX.attach(pinX);
    servoY.attach(pinY);

    return arm;
}

void setServoPos(Servo servo, int pos) {
    pos = constrain(pos, S_MIN, S_MAX);
    servo.write(pos);
}

void setSpeedX(Arm *arm, int s) {
    arm->speedX = constrain(s, -1 * S_MAX_SPEED, S_MAX_SPEED);
}

void setSpeedY(Arm *arm, int s) {
    arm->speedY = constrain(s, -1 * S_MAX_SPEED, S_MAX_SPEED);
}

void tick(Arm *arm) {
    unsigned long now = millis();

    unsigned long deltaX = now - arm->lastRunX;
    unsigned long deltaY = now - arm->lastRunY;

    // Number of ms required to pass to update X by 1 deg
    unsigned int threshX = 1000 / abs(arm->speedX);
    if (deltaX > threshX) {
        int v = 1;
        if (arm->speedX < 0) {
            v = -1;
        }

        arm->posX = constrain(arm->posX + v, S_MIN, S_MAX);
        setServoPos(arm->servoX, arm->posX);
        arm->lastRunX = now;
    }

    // Number of ms required to pass to update Y by 1 deg
    unsigned int threshY = 1000 / abs(arm->speedY);
    if (deltaY > threshY) {
        int v = 1;
        if (arm->speedY < 0) {
            v = -1;
        }
        arm->posY = constrain(arm->posY + v, S_MIN, S_MAX);
        setServoPos(arm->servoY, arm->posY);
        arm->lastRunY = now;
    }
}
