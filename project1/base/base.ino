// Required
#include <LiquidCrystal.h>
#include <Servo.h>
// Compiler always tries to compile Keypad.cpp/Arm.cpp since it's in the LOCAL_CPP_SRCS env

#include <stdint.h>

#include "Joystick.h"
#include "Arm.h"
#include "Keypad.h"
#include "Scheduler.h"
#include "Packet.h"

#define DELTA_CHAR "\x07"
#define YBAR_CHAR  "\x06"
#define XBAR_CHAR  "\x05"

typedef struct {
    const unsigned int joy1X;
    const unsigned int joy1Y;
    const unsigned int joy1SW;
    const unsigned int joy2X;
    const unsigned int joy2Y;
    const unsigned int joy2SW;
    const unsigned int lightSensor;
    const unsigned int idle;
} PinDefs;

PinDefs pin = {
    .joy1X  = A1,
    .joy1Y  = A2,
    .joy1SW = 30,
    .joy2X  = A3,
    .joy2Y  = A4,
    .joy2SW = 31,
    .lightSensor = 32,
    .idle   = 50
};

int lightOn = 0;
Packet packet = {
    .speedX = 0,
    .speedY = 0,
    .laserOn = 0
};

Joystick  joystick(pin.joy1X, pin.joy1Y, pin.joy1SW);
Keypad pad;

void updatePacket();
void readLightSensor();
void sendPacket();
void updateLcd();

void setup() {
    Serial2.begin(9600);
    Serial.begin(9600);

    Scheduler_Init();
    Scheduler_StartTask(0, 50, updatePacket);
    // Scheduler_StartTask(50, 1000, readLightSensor);
    // Scheduler_StartTask(5, 500, sendPacket);
    Scheduler_StartTask(7, 20, updateLcd);
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


void sendPacket() {
    if (Serial2.availableForWrite()) {
        Serial2.write((byte*)&packet, sizeof(packet));
    }
}

void updateLcd() {
    char row_buf[16];
    sprintf(row_buf, " " XBAR_CHAR ":%4d  " YBAR_CHAR ":%4d", joystick.rawX, joystick.rawY);
    pad.print(Keypad::LCD_ROW::TOP, row_buf);
    pad.print(Keypad::LCD_ROW::BOTTOM, joystick.rawSW ? "ON " : "OFF");
}

void readLightSensor() {
    lightOn = digitalRead(pin.lightSensor);
}

void updatePacket() {
    packet.speedX = joystick.getX();
    packet.speedY = joystick.getY();
    packet.laserOn = joystick.getClick();
}
