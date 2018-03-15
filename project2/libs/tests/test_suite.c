#include "../../common.h"
#include "tests.h"
#include "test_list.h"
#include <avr/io.h>
#include "uart.h"

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

    UART_Init0(38400);

    // Raise PD0 while any tests are active
    BIT_SET(PORTD, 0);
    UART_print("Starting tests...\n");

    // is TEST_QUEUE in the mask?
    if ((mask & TEST_QUEUE) == TEST_QUEUE) {
        UART_print("Starting queue test...\n");
        BIT_SET(PORTD, 1);
        Task_Queue_Test();
        BIT_CLR(PORTD, 1);
        UART_print("Queue test complete\n");
    }

    if ((mask & TEST_MSG) == TEST_MSG) {
        UART_print("Starting msg test...\n");
        BIT_SET(PORTD, 1);
        Msg_Test();
        BIT_CLR(PORTD, 1);
        UART_print("Msg test complete\n");
    }

    if ((mask & TEST_OSFN) == TEST_OSFN) {
        UART_print("Starting OSFN test...\n");
        BIT_SET(PORTD, 1);
        OSFN_Test();
        BIT_CLR(PORTD, 1);
        UART_print("OSFN test complete...\n");
    }

    /* Example for another masked test
    // is TEST_THING in the mask?
    if ((mask & TEST_THING) == TEST_THING) {
        BIT_SET(PORTD, 1);
        Test_Thing();
        BIT_CLR(PORTD, 1);
    }
    */

    // If Port E is not 0x00 then a test has failed
    if (PORTE != 0x00) {
        UART_print("PORTE not 0x00\n");
        for (;;) {}
    }

    UART_print("Test complete!\n");

    // All tests passed, since if an asset fails the board hangs there
    // Set PD0 back to low
    BIT_CLR(PORTD, 0);

}
