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

void loop()
{
    pad.print(LCDKeypad::LCD_ROW::TOP, "Hello!");
    pad.getLCD()->write(7);

    // Make sure getLastButton is called first, pollButtons will update it
    if(pad.getLastButton() != pad.pollButtons()); {
        // update the display
        pad.print(LCDKeypad::LCD_ROW::BOTTOM, strings[pad.getLastButton()]);
    }
}
