
#include "LCDKeypad.h"

LCDKeypad::LCDKeypad() : lcd(8, 9, 4, 5, 6, 7) {
    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print("Push the buttons");
}

void LCDKeypad::print(LCD_ROW row, String text) {
    lcd.setCursor(0, row);
    lcd.print(text);
}

void LCDKeypad::clear() {
    lcd.clear();
}

void LCDKeypad::clear(LCD_ROW row) {
    lcd.setCursor(0, row);
    lcd.print("                ");
    lcd.setCursor(0, row);
}

LCDKeypad::LCD_BUTTONS LCDKeypad::pollButtons() {
    unsigned int adcValue = analogRead(0);

    if (adcValue > 1000) return (lastButton = LCDKeypad::LCD_BUTTONS::BUTTON_NONE);
    if (adcValue < 50)   return (lastButton = LCDKeypad::LCD_BUTTONS::BUTTON_RIGHT);
    if (adcValue < 250)  return (lastButton = LCDKeypad::LCD_BUTTONS::BUTTON_UP);
    if (adcValue < 350)  return (lastButton = LCDKeypad::LCD_BUTTONS::BUTTON_DOWN);
    if (adcValue < 500)  return (lastButton = LCDKeypad::LCD_BUTTONS::BUTTON_LEFT);
    if (adcValue < 900)  return (lastButton = LCDKeypad::LCD_BUTTONS::BUTTON_SELECT);
    else                 return (lastButton = LCDKeypad::LCD_BUTTONS::BUTTON_NONE);
}

LCDKeypad::LCD_BUTTONS LCDKeypad::getLastButton() {
    return lastButton;
}

LiquidCrystal* LCDKeypad::getLCD() {
    return &lcd;
}
