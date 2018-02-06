#include <LiquidCrystal.h>
#include <stdint.h>

#include "../common/joystick.h"
#include "../common/arm.h"
#include "../common/LCDKeypad.h"
#include "../common/Scheduler.h"
#include "../common/packet.h"

#define DELTA_CHAR "\x07"

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

String button_names[LCDKeypad::LCD_BUTTONS::COUNT_BUTTONS] = {
    "--      ",
    "RIGHT   ",
    "UP      ",
    "DOWN    ",
    "LEFT    ",
    "SELECT  ",
};

Joy stick;
Arm arm;
LCDKeypad pad;
char row_buf[16];

void updateArm();
void readLightSensor();
void sendPacket();
void updateLcd();

void setup() {
    Serial2.begin(9600);
    Serial.begin(9600);
    stick = init_Joy(pin.joy1X, pin.joy1Y, pin.joy1SW);

    pad = LCDKeypad();
    pad.clear();

    // Define a delta character in the LCD's custom character
    // memory in position number 7 (of 7 [0-7])
    byte delta[8] = {
        0b00000,
        0b00100,
        0b00100,
        0b01010,
        0b01010,
        0b10001,
        0b11111
    };
    pad.getLCD()->createChar(7, delta);

    Scheduler_Init();
    Scheduler_StartTask(0, 50, updateArm);
    Scheduler_StartTask(3, 20, readLightSensor);
    Scheduler_StartTask(5, 150, sendPacket);
    Scheduler_StartTask(7, 1000, updateLcd);
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
    char row_buf[256];
    sprintf(row_buf, DELTA_CHAR"X:%4d "DELTA_CHAR"Y:%4d", packet.speedX, packet.speedY);
    pad.print(LCDKeypad::LCD_ROW::TOP, row_buf);

    pad.print(LCDKeypad::LCD_ROW::BOTTOM, lightOn ? "ON " : "OFF");
}

void readLightSensor() {
    lightOn = digitalRead(pin.lightSensor);
}

void updateArm() {
    int x_val = getX(stick);
    packet.speedX = (uint16_t)x_val;

    int y_val = getY(stick);
    packet.speedY = (uint16_t)y_val;

    int z_val = getClick(stick);
    packet.laserOn = (uint8_t)z_val;

    tick(&arm);
}
