#include <avr/io.h>
#include <util/delay.h>
#include "os.h"
#include "kernel.h"
#include "common.h"

/**
 * A cooperative "Ping" task.
 * Added testing code for LEDs.
 */
void Ping() TASK
({
    BIT_FLIP(PORTB, 1);
    _delay_ms(1000);
})

/**
 * A cooperative "Pong" task.
 * Added testing code for LEDs.
 */
void Pong() TASK
({
    BIT_FLIP(PORTB, 0);
    _delay_ms(1000);
})


/**
 * This function creates two cooperative tasks, "Ping" and "Pong". Both
 * will run forever.
 */
int main() {
    DDRB = 0b11111111; // All outputs

    // TODO: Shouldn't have to manually init and start os.
    // Kernel should start user code instead
    Kernel_Init();
    Task_Create(Pong);
    Task_Create(Ping);
    Kernel_Start();

    /* Never reaches this point */
    return -1;
}
