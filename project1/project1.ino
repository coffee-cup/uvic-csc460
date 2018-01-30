#include "LCDKeypad.h"
#include <LiquidCrystal.h>

LCDKeypad pad;
String strings[LCDKeypad::LCD_BUTTONS::COUNT_BUTTONS] = {
    "--      ",
    "RIGHT   ",
    "UP      ",
    "DOWN    ",
    "LEFT    ",
    "SELECT  ",
};

void setup()
{
    pad = LCDKeypad();
    pad.clear();
}

void loop()
{
    pad.print(LCDKeypad::LCD_ROW::TOP, "Hello!");

    // Make sure getLastButton is called first, pollButtons will update it
    if(pad.getLastButton() != pad.pollButtons()); {
        // update the display
        pad.print(LCDKeypad::LCD_ROW::BOTTOM, strings[pad.getLastButton()]);
    }
}