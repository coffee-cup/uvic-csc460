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

    enum BUTTON {
        BUTTON_NONE,
        BUTTON_RIGHT,
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_LEFT,
        BUTTON_SELECT,
        COUNT_BUTTONS // Must be last
    };

  protected:
    BUTTON lastButton;
    LiquidCrystal lcd;

  public:
    Keypad();
    void clear(void);
    void clear(LCD_ROW);
    void print(LCD_ROW, char*, bool = true);
    BUTTON pollButtons();
    BUTTON getLastButton();
};

#endif
