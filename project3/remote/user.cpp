#include <avr/io.h>
#include <util/delay.h>

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
void Ping(void) TASK
({
    BIT_FLIP(PORTB, 0);
    UART_print(0, "Ping\n");
})

/**
 * A cooperative "Pong" task.
 * Added testing code for LEDs.
 */
void Pong(void) TASK
({
    BIT_FLIP(PORTB, 1);
    UART_print(3, "Pong\n");
})

/**
 * This function creates two cooperative tasks, "Ping" and "Pong". Both
 * will run forever.
 */
void create(void) {

    // Outputs for LED's
    BIT_SET(DDRB, 0);
    BIT_SET(DDRB, 1);
    BIT_CLR(PORTB, 0);
    BIT_CLR(PORTB, 1);

    UART_Init(0, LOGBAUD);
    UART_Init(3, LOGBAUD);
    Task_Create_Period(Ping, 0, 200, 1, 0);
    Task_Create_Period(Pong, 0, 200, 1, 1);

    // This function was called by the OS as a System task.
    // If a task executes a return statement it is terminated.
    // Could also call Kernel_Request(TERMINATE) here.

    return;
}
