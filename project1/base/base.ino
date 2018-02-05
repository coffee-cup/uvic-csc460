#include <Servo.h>
#include <LiquidCrystal.h>
#include "../common/joystick.h"
#include "../common/arm.h"
#include "../common/LCDKeypad.h"
#include "../common/Scheduler.h"

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
    const unsigned int idle;
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
    .laser  = 40,
    .idle   = 50
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

    Scheduler_Init();
    Scheduler_StartTask(0, 10, updateArm);

}

// idle task
void idle(uint32_t idle_period)
{
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
	if (idle_period)
	{
		idle(idle_period);
	}
}

void updateArm() {
    int x_val = getX(stick);
    setSpeedX(&arm, x_val);

    int y_val = getY(stick) * -1;
    setSpeedY(&arm, y_val);

    digitalWrite(pin.laser, getClick(stick) ? HIGH : LOW);

    tick(&arm);
}
