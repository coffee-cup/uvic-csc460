#include "../../../common.h"
#include "../../../os.h"
#include "test_utils.h"
#include <avr/io.h>
#include <util/delay.h>

void Task_Limit_Test() {
    // Expect the system to abort because too many tasks are created
    _delay_ms(1000);
}

void Task_Test() {
    int i = 0;
    for (; i < 17; i += 1) {
        Task_Create_RR(Task_Limit_Test, 0);
    }
    AssertAborted();

    Task_Create_RR(NULL, 0);
    AssertAborted();

    // Wait for tasks to be run
    _delay_ms(100);
}
