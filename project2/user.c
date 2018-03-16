#include <avr/io.h>
#include <util/delay.h>
#include "os.h"
#include "kernel.h"
#include "common.h"

/**
 * A cooperative "Ping" task.
 * Added testing code for LEDs.
 */
void Ping(void) TASK
({
    //uint16_t x = 6;
    BIT_FLIP(PORTB, 0);
    //Msg_Send(1, ANY, &x);
    _delay_ms(20);
})

/**
 * A cooperative "Pong" task.
 * Added testing code for LEDs.
 */
void Pong(void) TASK
({
    //uint16_t x;
    //Msg_Recv(ANY, &x);
    BIT_FLIP(PORTB, 1);
    _delay_ms(20);
})


/**
 * This function creates two cooperative tasks, "Ping" and "Pong". Both
 * will run forever.
 */
void setup(void) {

    // Outputs for LED's
    BIT_SET(DDRB, 0);
    BIT_SET(DDRB, 1);
    BIT_CLR(PORTB, 0);
    BIT_CLR(PORTB, 1);

    Task_Create_System(Ping, 0);
    Task_Create_System(Pong, 0);

    // This function was called by the OS as a System task.
    // If a task executes a return statement it is terminated.
    // Could also call Kernel_Request(TERMINATE) here.
    return;
}
