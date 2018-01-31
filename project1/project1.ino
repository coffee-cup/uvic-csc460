#include <Arduino.h>
#include "joystick.h"
#include "arm.h"
#include "LCDKeypad.h"
#include <LiquidCrystal.h>

Joy stick;
Arm arm;

const int servoPinX = 8;
const int servoPinY = 7;
int pos = 0;

void clearScreen();
void updateArm();

LCDKeypad pad;
String strings[LCDKeypad::LCD_BUTTONS::COUNT_BUTTONS] = {
    "--      ",
    "RIGHT   ",
    "UP      ",
    "DOWN    ",
    "LEFT    ",
    "SELECT  ",
};

void setup() {
  Serial.begin(9600);
  arm = init_Arm(servoPinX, servoPinY);
  stick = init_Joy(A0, A1, 20, -70, 70);

  pad = LCDKeypad();
  pad.clear();

  setSpeedX(&arm, 10);
}

void loop() {
  /* clearScreen(); */

  pad.print(LCDKeypad::LCD_ROW::TOP, "Hello!");

  // Make sure getLastButton is called first, pollButtons will update it
  if(pad.getLastButton() != pad.pollButtons()); {
      // update the display
      pad.print(LCDKeypad::LCD_ROW::BOTTOM, strings[pad.getLastButton()]);
  }


  updateArm();

  /* delay(50); */
}

void updateArm() {
  int xVal = getX(stick);
  setSpeedX(&arm, xVal);

  int yVal = getY(stick);
  setSpeedY(&arm, yVal);

  /* Serial.print(xVal); */
  /* Serial.print(" "); */
  /* Serial.print(arm.speedX); */
  /* Serial.print(" "); */
  /* Serial.print(arm.posX); */

  /* Serial.print("\n"); */
  /* Serial.print(yVal); */
  /* Serial.print(" "); */
  /* Serial.print(arm.speedY); */
  /* Serial.print(" "); */
  /* Serial.print(arm.posY); */

  tick(&arm);
}

void clearScreen() {
  Serial.write(27);       // ESC command
  Serial.print("[2J");    // clear screen command
  Serial.write(27);
  Serial.print("[H");     // cursor to home command
}

