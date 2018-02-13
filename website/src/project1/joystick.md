## Joystick

The joystick allows the user to input X, Y, and Z axis positions using their thumb. The horizontal and vertical axes are an analog signal which is controlled internally by a variable resistor. The resistance changes depending on where the joystick is moved relative to the center position of each axis. This behaves similarly to a potentiometer, except that the resistance is dependent on the location of the joystick.

The third Z axis is controlled by an internal switch which grounds the signal when closed. This means that the signal is high when not pressed, and low when pressed. Since the only states possible are high (5V) and low (0V), this can be connected to a digital pin.

![Joystick][joystick]

Since the signals coming from the joystick for x and y position are analog, they need to be converted to digital using an ADC converter. We connect the horizontal and vertical pins to analog A1 and A2, respectively. The z axis switch is connected to digital pin 30. The connections are depicted in the following schematic.

![Joystick Schematic][joystick_schematic]

[joystick]: https://i.imgur.com/v6BjQxS.png "Joystick" { width=50% }

[joystick_schematic]: https://i.imgur.com/uGEt6Qf.png "Joystick Schematic"

Input into the ATMega2560 analog pins is automatically quantized from an analog signal to a digital value. The values will be in the range 0 to 1023 as only the ATMega2560 has a 10 bit resolution for its analog to digital converter (ADC). In the case of the joystick, when it is in the middle - "zero" - position, the value will be 512. When reading values for the horizontal and vertical positions, the Arduino `analogRead` function is used. It was observed that there was some jitter in the joystick. When the joystick was at its center position, the values returned varied by about 20. Near the edges of the joystick the values also did not also max out at 0 or 1023. Because of this we introduced a _deadband_ range of 20 for the center, minimum, and maximum positions. In this deadband, all values would be treated the same. The deadband was implemented in software by mapping and constraining the values 20-1003 to 0-1023.

An internal pull up resistor was used for the joystick switch pin. This pull up resistor will invert the behavior of the pin such that a high signal means the switch is off and a low signal means the switch is on. The following code using the Arduino library sets pin 30 on the ATMega2560 as input with a pull up resistor.

```c
pinMode(30, INPUT_PULLUP);
```
