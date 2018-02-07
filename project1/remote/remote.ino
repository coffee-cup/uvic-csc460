// Required
#include <LiquidCrystal.h>
#include <Servo.h>
// Compiler always tries to compile Keypad.cpp/Arm.cpp since it's in the LOCAL_CPP_SRCS env

#include <stdint.h>

#include "Arm.h"
#include "Scheduler.h"
#include "Packet.h"

typedef struct {
    const uint8_t servoX;
    const uint8_t servoY;
    const uint8_t laser;
    const uint8_t idle;
} PinDefs;

PinDefs pin = {
    .servoX = 11,
    .servoY = 12,
    .laser = 40,
    .idle = 50
};

Arm arm;
Packet p = create_packet(504, 517, 0);

void getData();
void updateArm();

void setup() {
    Serial.begin(38400);
    Serial2.begin(9600);

    arm.attach(pin.servoX, pin.servoY);

    pinMode(pin.laser, OUTPUT);
    pinMode(pin.idle, OUTPUT);

    Scheduler_Init();
    Scheduler_StartTask(0, 50, updateArm);
    Scheduler_StartTask(15, 100, getData);
}

// idle task
void idle(uint32_t idle_period) {
    // this function can perform some low-priority task while the scheduler has nothing to run.
    // It should return before the idle period (measured in ms) has expired.  For example, it
    // could sleep or respond to I/O.

    // example idle function that just pulses a pin.
    digitalWrite(pin.idle, HIGH);
    delay(idle_period);
    digitalWrite(pin.idle, LOW);
}

void loop() {
    uint32_t idle_period = Scheduler_Dispatch();
    if (idle_period) {
        idle(idle_period);
    }
}

void getData() {
    if (5 <= Serial2.available()) {
        uint8_t buffer[5];

        if (Serial2.readBytes(buffer, 5)) {
            p = create_packet(buffer);
        }
    }
}

void updateArm() {
    arm.setSpeedX(arm.filterSpeed(p.speedX));
    arm.setSpeedY(arm.filterSpeed(p.speedY));

    digitalWrite(pin.laser, p.laserOn ? HIGH : LOW);

    Serial.println("---");
    Serial.print("SpeedX: ");  Serial.println(arm.X.speed, DEC);
    Serial.print("SpeedY: ");   Serial.println(arm.Y.speed, DEC);

    Serial.print(" : XPOS = "); Serial.println(arm.X.pos);
    Serial.print(" : YPOS = "); Serial.println(arm.Y.pos);

    arm.tick();
}
