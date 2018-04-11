#include <avr/io.h>
#include <util/delay.h>

#include "Arm.h"
#include "Joystick.h"
#include "Motor.h"

Arm arm;
Joystick joy(15, 14, 3);

extern "C" {
    #include "kernel.h"
    #include "os.h"
    #include "uart.h"
    #include "utils.h"
    void create(void);
}

int main(void) {
    Kernel_Begin();
}


/**
 * A simple task to move a Roomba on the spot.
 */
void Move(void) TASK ({
    BIT_FLIP(PORTB, 0);
    arm.setSpeedX(Arm::filterSpeed(joy.getX()));
    arm.setSpeedY(Arm::filterSpeed(joy.getY()));
})


void Tick(void) TASK({
    BIT_FLIP(PORTB, 1);
    arm.tick();
})

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

    arm.attach(2, 3);

    Task_Create_Period(Tick, 0, 2, 1, 2);
    Task_Create_Period(Move, 0, 10, 2, 1);


    // This function was called by the OS as a System task.
    // If a task executes a return statement it is terminated.
    // Could also call Kernel_Request(TERMINATE) here.

    return;
}
