#ifndef KEYPAD_H
#define KEYPAD_H

#include <Arduino.h>
#include <LiquidCrystal.h>

class Keypad {

public:
    enum LCD_ROW {
        TOP = 0,
        BOTTOM
    };

    enum LCD_BUTTONS {
        BUTTON_NONE,
        BUTTON_RIGHT,
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_LEFT,
        BUTTON_SELECT,
        COUNT_BUTTONS // Must be last
    };

protected:
    LiquidCrystal lcd;
    LCD_BUTTONS lastButton;

public:
    Keypad();
    void clear(void);
    void clear(LCD_ROW);
    void print(LCD_ROW, String, bool=true);
    LCD_BUTTONS pollButtons();
    LCD_BUTTONS getLastButton();
};

#endif
