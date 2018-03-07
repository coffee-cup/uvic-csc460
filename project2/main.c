#include "os.h"
#include "kernel.h"
#include <avr/io.h>

/**
 * A cooperative "Ping" task.
 * Added testing code for LEDs.
 */
void Ping()
{
    for(;;){
        PORTB = 0b00000010;

        _delay_ms(1000);
        Task_Next();
    }
}

/**
 * A cooperative "Pong" task.
 * Added testing code for LEDs.
 */
void Pong()
{
    for(;;) {
        PORTB = 0b00000001;

        _delay_ms(1000);
        Task_Next();
    }
}

/**
 * This function creates two cooperative tasks, "Ping" and "Pong". Both
 * will run forever.
 */
void main() {
    DDRB = 0b11111111; // All outputs

    // TODO: Shouldn't have to manually init and start os.
    // Kernel should start user code instead
    Kernel_Init();
    Task_Create(Pong);
    Task_Create(Ping);
    Kernel_Start();
}
