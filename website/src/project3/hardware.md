# Hardware

Most of the hardware used in this project was also used in [project 1](/project1). However, in project 1 we used the Arduino libraries to interface with the hardware. In this project no Arduino libraries were used. This section will discuss how we implemented with the hardware components using ATMega2560 registers.

## ATMega2560

## Servos

## Joysticks

We used the same joysticks as we used in project 1. They allow the user to input X, Y, and Z axis positions using their thumb. The X and Y positions are analog signals which is controlled internally by a variable resistor. In order to read the positions of the X and Y values, the internal analog to digital converter must be used.

![Joystick](https://i.imgur.com/v6BjQxS.png)

## Lasers
