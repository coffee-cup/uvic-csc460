#include "../../../common.h"
#include "../../../os.h"
#include "test_utils.h"
#include <avr/io.h>
#include <util/delay.h>

void Task_Limit_Test() {
    // Expect the system to recover if too many tasks are created
    _delay_ms(100);
}

void Task_Test() {
    // Creating task with null function should os abort
    Task_Create_RR(NULL, 0);
    AssertAborted();

    int i;
    for (i = 0; i < MAXTHREAD + 1; i += 1) {
        Task_Create_RR(Task_Limit_Test, i);
    }

    // Wait for tasks to be run
    _delay_ms(1100);
}
