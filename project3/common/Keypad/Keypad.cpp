#include "Keypad.h"

extern "C" {
    #include "common.h"
}

Keypad::Keypad() : lcd(8, 9, 4, 5, 6, 7) {
    analog_init();

    // Set top 4 bits of PORTB as output
    MASK_SET(DDRB, 0xF0);
    MASK_SET(PORTB, 0xF0);

    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.clear();

    // Define some custom LCD characters

    byte sw_off[8] = {
        0b11111,
        0b00100,
        0b00100,
        0b00100,
        0b00000,
        0b11111,
        0b10001,
        0b00000
    };
    lcd.createChar(3, sw_off);

    byte sw_on[8] = {
        0b00000,
        0b00000,
        0b00000,
        0b11111,
        0b00100,
        0b11111,
        0b10101,
        0b00000,
    };
    lcd.createChar(4, sw_on);

    byte xbar[8] = {
        0b11111,
        0b00000,
        0b10001,
        0b01010,
        0b00100,
        0b01010,
        0b10001,
        0b00000
    };
    lcd.createChar(5, xbar);

    byte ybar[8] = {
        0b11111,
        0b00000,
        0b10001,
        0b01010,
        0b00100,
        0b00100,
        0b00100,
        0b00000,
    };
    lcd.createChar(6, ybar);

    byte delta[8] = {
        0b00000,
        0b00100,
        0b00100,
        0b01010,
        0b01010,
        0b10001,
        0b11111,
        0b00000
    };
    lcd.createChar(7, delta);
}

void Keypad::print(LCD_ROW row, char* text, bool resetCursor) {
    if (resetCursor) lcd.setCursor(0, row);
    lcd.print(text);
}

void Keypad::clear() {
    lcd.clear();
}

void Keypad::clear(LCD_ROW row) {
    lcd.setCursor(0, row);
    lcd.print("                " /* 16x spaces */);
    lcd.setCursor(0, row);
}

Keypad::BUTTON Keypad::pollButtons() {
    unsigned int adcValue = analog_read(0);

    if (adcValue > 1000) return (lastButton = Keypad::BUTTON::BUTTON_NONE);
    if (adcValue < 50)   return (lastButton = Keypad::BUTTON::BUTTON_RIGHT);
    if (adcValue < 250)  return (lastButton = Keypad::BUTTON::BUTTON_UP);
    if (adcValue < 350)  return (lastButton = Keypad::BUTTON::BUTTON_DOWN);
    if (adcValue < 500)  return (lastButton = Keypad::BUTTON::BUTTON_LEFT);
    if (adcValue < 900)  return (lastButton = Keypad::BUTTON::BUTTON_SELECT);
    else                 return (lastButton = Keypad::BUTTON::BUTTON_NONE);
}

Keypad::BUTTON Keypad::getLastButton() {
    return lastButton;
}
