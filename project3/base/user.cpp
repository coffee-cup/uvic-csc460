#include <avr/io.h>
#include <util/delay.h>
#include <LiquidCrystal.h>
#include "Keypad.h"
#include "Joystick.h"

extern "C" {
    #include "kernel.h"
    #include "os.h"
    #include "common.h"
    #include "uart.h"
    #include "utils.h"
    void create(void);
}

Keypad pad;
Joystick joystick1(15, 14, 0);

int main(void) {
    Kernel_Begin();
}

void init_io(void) {
    // 1 = output, 0 = input
    DDRB = 0xFF;

    PORTB = 0xF0;
}

void joystickUpdate(void) {
    for (;;) {
        uint16_t val = joystick1.getX();
        LOG("%d\n", val);
        Task_Next();
    }
}

void LcdUpdate(void) {
    char buf[16];
    TICK now;

    pad.clear();
    for (;;) {
        now = Now();
        sprintf(buf, "Tick: %d", now);

        pad.print(Keypad::LCD_ROW::TOP, buf);
        Task_Next();
    }
}

/**
 * This function creates two cooperative tasks, "Ping" and "Pong". Both
 * will run forever.
 */
void create(void) {
    init_io();

    Task_Create_Period(LcdUpdate, 0, 50, 5, 4);
    Task_Create_Period(joystickUpdate, 0, 2, 1, 0);

    // Task_Create_Period(Ping, 0, 2, 1, 0);
    // Task_Create_Period(Pong, 0, 2, 1, 1);

    // This function was called by the OS as a System task.
    // If a task executes a return statement it is terminated.
    // Could also call Kernel_Request(TERMINATE) here.
    return;
}
