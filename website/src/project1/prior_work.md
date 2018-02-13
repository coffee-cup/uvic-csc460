# Prior Work

Before beginning any work on this project we were requred to complete several exercises to guage our understanding of important concepts related to this project. These included exercises in working with UART, Logic Analysers, PWM, and ADCs. Also, these exercises provided a chance to familiarize ourselves with the development environment. While most of these concepts will be discussed in the remainder of this report, this section will briefly expand on UART, ADCs, and the development environment so that some ground level awareness is in place when these concepts arise later.

## UART

Universal asynchronous receiver-transmitter (UART) devices are integrated circuits which enable serial communications between devices. UART devices use a configurable baud rate - i.e. bitrate - to agree upon transmission speed. The Arduino ATmega2560 has 4 full-duplex UART ports which can all simultaneously receive and transmit data at the same time. Serial devices transmit data as individual bits in sequence over a single communication channel, by decomposing bytes into a sequence of single bits that are then transmitted. We make substantial use of UART in this project.

## ADCs

An analog to digital converter (ADC) is another hardware device used extensively in this report. We use ADCs to quantize a real valued voltage level to an integer value. The Arduino libraries invoke the ATmega2560's ADC whenever the `analogRead` function is called. Our microcontrollers have 10-bit ADCs meaning that values between $0V$ and $5V$ can be converted to a value between 0 - 1023 inclusive. As such, each increment in the value from the ADC corresponds to a resolution of 0.0049 volts per unit.

## Development Environment

We completed exercises 1 through 5 using the Arduino IDE version 1.8.5. Unfortunately the IDE does not support console input over serial communications, so by the beginning of this project we switched our development environment to utilize workflows powered by unix `Makefile`. This was facilitated by the [sudar/Arduino-Makefile](https://github.com/sudar/Arduino-Makefile) github project. Using this makefile we are able to use our own preferred editors, and upload to specific boards via console command `make upload`. This also allows use to change upload targets faster than was possible in the Arduino IDE. The Arduino-Makefile project uses `avr-g++` as a compiler and `avrdude` to communicate with and upload programs to the boards over USB serial.
