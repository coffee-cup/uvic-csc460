#include <avr/io.h>
#include <util/delay.h>
#include "os.h"
#include "kernel.h"
#include "common.h"
#include "libs/tests/test_suite.c"

#define RUN_TESTS 0

/**
 * A cooperative "Ping" task.
 * Added testing code for LEDs.
 */
void Ping(void) TASK
({
    BIT_FLIP(PORTB, 0);
})

/**
 * A cooperative "Pong" task.
 * Added testing code for LEDs.
 */
void Pong(void) TASK
({
    BIT_FLIP(PORTB, 1);
})

/*
 * Runs the tests
 */
void run_tests(void) {
    Test_Suite(TEST_OSFN);

    // Do not go to idle
    for (;;) {}
}

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

    if (RUN_TESTS) {
        Task_Create_RR(run_tests, 0);
        return;
    }

    Task_Create_Period(Ping, 0, 2, 1, 0);
    Task_Create_Period(Pong, 0, 2, 1, 1);

    // This function was called by the OS as a System task.
    // If a task executes a return statement it is terminated.
    // Could also call Kernel_Request(TERMINATE) here.
    return;
}
