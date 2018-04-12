#include <avr/io.h>
#include <util/delay.h>

#include "Arm.h"
#include "Joystick.h"
#include "Motor.h"

Arm arm;
Joystick joy1(15, 14, 2);
Joystick joy2(13, 12, 3);

extern "C" {
    #include "kernel.h"
    #include "os.h"
    #include "uart.h"
    #include "utils.h"
    void create(void);
}

// Weak attribute allows other functions to redefine
// We want kernel main to be the actual main
DELEGATE_MAIN()

/**
 * A simple task to move a Roomba on the spot.
 */
void Move(void) TASK ({
    arm.setSpeedX(Arm::filterSpeed(joy1.getX()));
    arm.setSpeedY(Arm::filterSpeed(joy2.getY()));
})


void Tick(void) {
    // 30 seconds === 30000 ms
    uint32_t numLaserTicks = 30000 / MSECPERTICK;

    TASK({
        arm.tick();
        if (joy1.getClick() && numLaserTicks > 1) {
            BIT_SET(PORTC, 0);
            numLaserTicks -= 2;
        } else {
            BIT_CLR(PORTC, 0);
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
    BIT_SET(DDRC, 0); // Laser

    BIT_CLR(PORTB, 0);
    BIT_CLR(PORTB, 1);
    BIT_CLR(PORTC, 0);

    arm.attach(2, 3);

    Task_Create_Period(Tick, 0, 2, 1, 2);
    Task_Create_Period(Move, 0, 10, 2, 1);


    // This function was called by the OS as a System task.
    // If a task executes a return statement it is terminated.
    // Could also call Kernel_Request(TERMINATE) here.

    return;
}
