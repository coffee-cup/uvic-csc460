#include "joystick.h"
#include "arm.h"
#include "LCDKeypad.h"
#include <LiquidCrystal.h>

#define DELTA_CHAR "\x07"

typedef struct {
    const unsigned int servoX;
    const unsigned int servoY;
    const unsigned int joy1X;
    const unsigned int joy1Y;
    const unsigned int joy1SW;
    const unsigned int joy2X;
    const unsigned int joy2Y;
    const unsigned int joy2SW;
    const unsigned int laser;
} PinDefs;

PinDefs pin = {
    .servoX = 11,
    .servoY = 12,
    .joy1X  = A1,
    .joy1Y  = A2,
    .joy1SW = 30,
    .joy2X  = A3,
    .joy2Y  = A4,
    .joy2SW = 31,
    .laser  = 40
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

void setup() {
    arm = init_Arm(pin.servoX, pin.servoY);
    stick = init_Joy(pin.joy1X, pin.joy1Y, pin.joy1SW, -30, 30);
    pinMode(pin.laser, OUTPUT);

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
}

void loop() {

    /// Weee printf
    sprintf(row_buf, DELTA_CHAR"X:%3d | "DELTA_CHAR"Y:%3d", arm.speedX, arm.speedY);
    pad.print(LCDKeypad::LCD_ROW::TOP, row_buf);

    // Make sure getLastButton is called first, pollButtons will update it
    if(pad.getLastButton() != pad.pollButtons()); {
        // update the display
        pad.print(LCDKeypad::LCD_ROW::BOTTOM, button_names[pad.getLastButton()]);
    }

    updateArm();
}

void updateArm() {
    int x_val = getX(stick);
    setSpeedX(&arm, x_val);

    int y_val = getY(stick) * -1;
    setSpeedY(&arm, y_val);

    digitalWrite(pin.laser, getClick(stick) ? HIGH : LOW);

    tick(&arm);
}
