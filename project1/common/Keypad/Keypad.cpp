#include "Keypad.h"

Keypad::Keypad() : lcd(8, 9, 4, 5, 6, 7) {
    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.clear();

    // Define some custom LCD characters
    // ̅X in position 5, ̅Y in position 6, and Δ in position 7

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

void Keypad::print(LCD_ROW row, String text, bool resetCursor = true) {
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

Keypad::LCD_BUTTONS Keypad::pollButtons() {
    unsigned int adcValue = analogRead(0);

    if (adcValue > 1000) return (lastButton = Keypad::LCD_BUTTONS::BUTTON_NONE);
    if (adcValue < 50)   return (lastButton = Keypad::LCD_BUTTONS::BUTTON_RIGHT);
    if (adcValue < 250)  return (lastButton = Keypad::LCD_BUTTONS::BUTTON_UP);
    if (adcValue < 350)  return (lastButton = Keypad::LCD_BUTTONS::BUTTON_DOWN);
    if (adcValue < 500)  return (lastButton = Keypad::LCD_BUTTONS::BUTTON_LEFT);
    if (adcValue < 900)  return (lastButton = Keypad::LCD_BUTTONS::BUTTON_SELECT);
    else                 return (lastButton = Keypad::LCD_BUTTONS::BUTTON_NONE);
}

Keypad::LCD_BUTTONS Keypad::getLastButton() {
    return lastButton;
}
