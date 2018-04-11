#include <avr/io.h>
#include <util/delay.h>
#include "Roomba.h"

Roomba roomba(/*Serial*/ 3, /*Port A pin*/ 0);

extern "C" {
    #include "kernel.h"
    #include "os.h"
    #include "uart.h"
    void create(void);
}

int main(void) {
    Kernel_Begin();
}

/**
 * A cooperative "Ping" task.
 * Added testing code for LEDs.
 */
void Move(void) {
    uint8_t i = 0;
    TASK ({
        BIT_FLIP(PORTB, 0);
            LOG("Move\n");
            roomba.drive(100, i % 2 ? 1 : -1);
            if (++i >= 5) {
                roomba.power_off();
                Task_Terminate();
            }
    })
}

/**
 * This function creates two cooperative tasks, "Ping" and "Pong". Both
 * will run forever.
 */
void create(void) {

    // Outputs for Tasks's
    BIT_SET(DDRB, 0);
    BIT_SET(DDRB, 1);
    BIT_CLR(PORTB, 0);
    BIT_CLR(PORTB, 1);

    if (roomba.init()) {
        roomba.set_mode(Roomba::OI_MODE_TYPE::SAFE_MODE);
        roomba.leds(Roomba::OI_LED_MASK_ARGS::CHECK_ROBOT_LED, 0, 0);

        Task_Create_Period(Move, 0, 100, 20, 5);
    } else {
        OS_Abort(FAILED_START);
    }

    // This function was called by the OS as a System task.
    // If a task executes a return statement it is terminated.
    // Could also call Kernel_Request(TERMINATE) here.

    return;
}
