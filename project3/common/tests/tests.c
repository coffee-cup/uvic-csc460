#include <avr/io.h>
#include "../os/common.h"
#include "tests.h"
#include "test_list.h"
#include "uart.h"

#define Test_Case(m, mask, name, fn) \
    { \
        if ((m & mask) == mask) { \
            LOG("Starting %s test...\n", name); \
            BIT_SET(PORTD, 1); \
            fn(); \
            Check_PortE(); \
            BIT_CLR(PORTD, 1); \
            LOG("%s test complete\n", name); \
        } \
    }

void Check_PortE() {
    // If Port E is not 0x00 then a test has failed
    if (PORTE != 0x00) {
        LOG("PORTE not 0x00\n");
        LOG("TESTS FAILED!\n");
        for (;;) {}
    }
}

void Test_Suite(TEST_MASKS mask) {
    // Set everything to output low
    DDRA  = 0xFF;
    PORTA = 0;
    DDRB  = 0xFF;
    PORTB = 0;
    DDRC  = 0xFF;
    PORTC = 0;
    DDRD  = 0xFF;
    PORTD = 0;

    // User Port E as way to indicate a statement that should be unreachable was reached
    // If Port E is not 0x00 then the test failed
    DDRE  = 0xFF;
    PORTE = 0;

    // Raise PD0 while any tests are active
    BIT_SET(PORTD, 0);
    LOG("Starting tests...\n");

    Test_Case(mask, TEST_QUEUE, "Queue", Task_Queue_Test);
    Test_Case(mask, TEST_MSG, "Msg", Msg_Test);
    Test_Case(mask, TEST_OSFN, "OSFN", OSFN_Test);
    Test_Case(mask, TEST_MSG_TRACE, "Msg Trace", Msg_Trace_Test);
    Test_Case(mask, TEST_TASKS, "Task", Task_Test);

    Check_PortE();

    // All tests passed, since if an asset fails the board hangs there
    // Set PD0 back to low
    BIT_CLR(PORTD, 0);

    LOG("All Tests passed!\n");
}
