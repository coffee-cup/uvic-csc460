#include <stdint.h>

#include <Arduino.h>
#include "Joystick.h"
#include <LiquidCrystal.h> // ARDMK doesn't recognize that keypad requires this lib
#include "Keypad.h"
#include "Scheduler.h"
#include "Packet.h"

#define DELTA_CHAR '\x07'
#define YBAR_CHAR  '\x06'
#define XBAR_CHAR  '\x05'
#define SWON_CHAR  '\x04'
#define SWOFF_CHAR '\x03'

typedef struct {
    const uint8_t joy1X;
    const uint8_t joy1Y;
    const uint8_t joy1SW;
    const uint8_t joy2X;
    const uint8_t joy2Y;
    const uint8_t joy2SW;
    const uint8_t lightSensor;
    const uint8_t idle;
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

Packet packet = {
    .speedX = 0,
    .speedY = 0,
    .laserOn = 0
};

Joystick joystick(pin.joy1X, pin.joy1Y, pin.joy1SW);
Keypad pad;

uint8_t lightOn = 0;
char selectedLetter = 'A';

void updatePacket();
void readLightSensor();
void sendPacket();
void updateLcd();
void updateChar();

void setup() {
    Serial2.begin(9600);
    Serial.begin(9600);

    Scheduler_Init();
    Scheduler_StartTask(0,    50, readLightSensor);
    Scheduler_StartTask(10,   50, updateChar);
    Scheduler_StartTask(50,  100, updatePacket);
    Scheduler_StartTask(150, 250, sendPacket);
    Scheduler_StartTask(500, 500, updateLcd);
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
    sprintf(row_buf, "%c:%4d %c:%4d %c" , XBAR_CHAR, joystick.rawX, YBAR_CHAR, joystick.rawY, joystick.rawSW ? SWON_CHAR : SWOFF_CHAR);
    pad.print(Keypad::LCD_ROW::TOP, row_buf);

    sprintf(row_buf, "%c               ", selectedLetter);
    pad.print(Keypad::LCD_ROW::BOTTOM, row_buf);
}

void readLightSensor() {
    lightOn = digitalRead(pin.lightSensor);
}

void updateChar() {
    if (pad.getLastButton() != pad.pollButtons()) {
        Keypad::BUTTON pressed = pad.getLastButton();
        int8_t offset = 0;
        switch (pressed) {
            case Keypad::BUTTON::BUTTON_UP:
                offset = 1;
                break;

            case Keypad::BUTTON::BUTTON_RIGHT:
                offset = 5;
                break;

            case Keypad::BUTTON::BUTTON_DOWN:
                offset = -1;
                break;

            case Keypad::BUTTON::BUTTON_LEFT:
                offset = -5;
                break;

            default: break;
        }

        selectedLetter = ((selectedLetter - 'A' + offset) % 26 + 26) % 26 + 'A';
    }
}

void updatePacket() {
    packet.speedX = joystick.getX();
    packet.speedY = joystick.getY();
    packet.laserOn = joystick.getClick();
}
