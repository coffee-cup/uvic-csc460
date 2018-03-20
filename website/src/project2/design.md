# Design

In this section we cover the specific design choices made before and during implementation of our RTOS.

## Hardware choices

This year, each group was provided access to several Arduino Mega 2560 development boards. The Arduino Mega 2560 gets its name from the central processing unit present on the board; the AMTEL ATmega2560. Specific documentation for the ATmega2560 can be [found here](http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2549-8-bit-AVR-Microcontroller-ATmega640-1280-1281-2560-2561_datasheet.pdf) but a brief description follows.

At the board level, the Arduino Mega 2560 provides 54 digital configurable input / output pins, 15 of which can be configured to output pulse width modulated signals. Furthermore, there are 16 analog input pins. The Arduino Mega 2560 has one non-volatile secondary storage device which can hold up to 256 kB of data (although 8 kB are commonly reserved for the bootloader). Additionally, the Arduino Mega 2560 has 8 kB of SRAM - this device however is volatile as it requires power to retain information.

At the microprocess level, the ATmega2560 operates on 8-bit registers at 16 MHz using AVR. Although, undocumented AVR is commonly accepted to stand for **A**lf-Egil and **V**egard's **R**ISC processor. The AVR architecture is a modified Harvard architecture which is part of the reduced instruction set computer (RISC) microprocessor family. The ATmega2560 has 32 8-bit general purpose registers, four Universal Synchronous/Asynchronous Receiver-Transmitters, and support for many peripherals including several 8 and 16 bit counter-timers, four 8-bit PWM channels and an additional twelve programmable (2 to 16-bit) resolution PWM channels.

## Kernel Design


## Task Design


## Scheduler / Handler design


