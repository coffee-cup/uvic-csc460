
#include <avr/io.h>
#include "tests.h"
#include "test_list.h"

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


    // Raise PD0 while any tests are active
    BIT_SET(PORTD, 0);
    // is TEST_QUEUE in the mask?
    if ((mask & TEST_QUEUE) == TEST_QUEUE) {
        BIT_SET(PORTD, 1);
        Task_Queue_Test();
        BIT_CLR(PORTD, 1);
    }

    /* Example for another masked test
    // is TEST_THING in the mask?
    if ((mask & TEST_THING) == TEST_THING) {
        BIT_SET(PORTD, 1);
        Test_Thing();
        BIT_CLR(PORTD, 1);
    }
    */

    // All tests passed, since if an asset fails the board hangs there
    // Set PD0 back to low
    BIT_CLR(PORTD, 0);
}
