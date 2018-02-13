## LCD Display
[FritzingKeypad]: https://i.imgur.com/SQZo0JL.png "LCD Keypad Shield"
[FritzingLCD]: https://i.imgur.com/A8IAOlx.png "I2C LCD"

Two types of LCD display are available. The first is a display only that utilizes $I^2C$ for communications, the second is a keypad shield that operates over a parallel interface.

| ![][FritzingKeypad] | ![][FritzingLCD] |
|:-------------------:|:----------------:|
| LCD Keypad shield   |     I2C LCD      |

We chose to use the DFRobot LCD Keypad shield, as we might decide later on to make use of the buttons available on the shield. The Keypad shield sits on top of the ATmega2560 and utilizes pins 4, 5, 6, 7, 8, 9 and 10. This is in contrast from the $I^2C$ display which only uses 4 pins ($V_{cc}$, $GND$, $SDA$, $SCL$). Although the Keypad shield occupies more pins, it is not an issue as the keypad still provides jumpers to some of the pins it blocks and our pin out requirements can still be met when using the Keypad shield.

We control the LCD using the Arduino *LiquidCrystal* library. A LCD can be initialized using the declaration:

```c
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
```

Controlling an LCD via the *LiquidCrystal* library is easy, and essentially boils down to the same commands one would use for sending serial data.

