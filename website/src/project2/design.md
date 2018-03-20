# Design

In this section we cover the specific design choices made before and during implementation of our RTOS.

## Hardware choices

This year, each group was provided access to several Arduino Mega 2560 development boards. The Arduino Mega 2560 gets its name from the central processing unit present on the board; the AMTEL ATmega2560. Specific documentation for the ATmega2560 can be [found here](http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2549-8-bit-AVR-Microcontroller-ATmega640-1280-1281-2560-2561_datasheet.pdf) but a brief description follows.

[mega2560]: https://i.imgur.com/O6ApT45.jpg "Arduino Mega 2560"
![Arduino Mega 2560][mega2560]

At the board level, the Arduino Mega 2560 provides 54 digital configurable input / output pins, 15 of which can be configured to output pulse width modulated signals. Furthermore, there are 16 analog input pins. The Arduino Mega 2560 has one non-volatile secondary storage device which can hold up to 256 kB of data (although 8 kB are commonly reserved for the bootloader). Additionally, the Arduino Mega 2560 has 8 kB of SRAM - this device however is volatile as it requires power to retain information.

At the microprocess level, the ATmega2560 operates on 8-bit registers at 16 MHz using AVR. Although, undocumented AVR is commonly accepted to stand for **A**lf-Egil and **V**egard's **R**ISC processor. The AVR architecture is a modified Harvard architecture which is part of the reduced instruction set computer (RISC) microprocessor family. The ATmega2560 has 32 8-bit general purpose registers, four Universal Synchronous/Asynchronous Receiver-Transmitters, and support for many peripherals including several 8 and 16 bit counter-timers, four 8-bit PWM channels and an additional twelve programmable (2 to 16-bit) resolution PWM channels.

## Kernel Design

The kernel is critical part of our RTOS. As such, it received the most attention during implementation. It is also the owner of the `c` program's `main()` function. As discussed previously, a real-time operating system can be implemented using a time-sharing or event-driven scheme, or a hybrid of the two. Our operating system is a hybrid system. Tasks can generate 'events' by making system calls, meanwhile a counter-timer generates interrupts at a prespecified frequency - we typically use 100 Hz or every 10 ms. This has the advantage of being more flexable when concerning usability. Tasks can volantarilty yield their share of the hardware, and if they do not, tasks of equal priority will still be given equal shares. The following diagram shows, at a high level, how the operating system starts its execution when the program begins. Some details are omitted for brevity.

[OSEntry]: https://i.imgur.com/5BxPZzZ.png "Entry point execution diagram"
![RTOS entry point execution diagram][OSEntry]

With reference to the entry point diagram above, the `main()` function calls `Kernel_Init()` which configures the system clock, clears then initalized preallocated required memory, then creates a high priority task for the application code to run its initialization code. It is assumed that all applications have a `setup()` function. This function is in place of the `a_main()` function used in other sample operating systems. When the `setup()` function returns, the operating system is ready to enter it's main execution loop. At this point control is passed all the back to the `main()` function. It then calls the `Kernel_Start()` function, which enables the counter-timer and dispatches a new task. The counter-timer will interrupt the currently executing task every 10 ms, during this interruption the kernel will perform a reschedule and possibly context switch to a new task.

In our project we tried to isolate the kernel functions from user programs as much as possible. This can be done by reducing the number of functions that are declared in the kernel's header file, and instead routing calls to the kernel (i.e.: syscalls) though a dedicated function. Below is the entirety of our kernel.h file, note that the only way of 'outside' communication with the kernel is though the `Kernel_Request()` function.

```c
#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "common.h"

void Kernel_Request(KERNEL_REQUEST_PARAMS* info);

#endif
```

## Task Design



## Scheduler / Handler design


