#include "../../../common.h"
#include "../../../os.h"
#include "../../trace/trace.h"
#include "test_utils.h"
#include <avr/io.h>
#include <util/delay.h>

/*
 * Creating more than MAXTHREAD tasks should not crash
 */

void Task_Limit_Test() {
    // Expect the system to recover if too many tasks are created
    _delay_ms(100);
}

void Task_Create_MaxThread() {
    int i;
    for (i = 0; i < MAXTHREAD + 1; i += 1) {
        Task_Create_RR(Task_Limit_Test, i);
    }

    // Wait for tasks to be run
    _delay_ms(1100);
}

/*
 * Creating a task with null function should OS abort
 */

void Task_Create_Null() {
    Task_Create_RR(NULL, 0);
    AssertAborted();

}

/*
 * A system task should run as soon as it is created
 */

void Task_System() {
    add_to_trace('a');
}

void Task_Create_Priority() {
    clear_trace();
    add_to_trace('s');

    Task_Create_System(Task_System, 0);
    add_to_trace('b');

    add_to_trace('f');

    uint8_t arr[] = {'s', 'a', 'b', 'f'};
    Assert(compare_trace(arr) == 1);
}

void Task_Test() {
    Task_Create_MaxThread();
    Task_Create_Null();
    Task_Create_Priority();
}
