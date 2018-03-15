#include "../../common.h"
#include "../../os.h"
#include "test_utils.h"
#include <avr/io.h>
#include <util/delay.h>

uint16_t arg_val = 42;
void Arg_Test() {
    int16_t arg = Task_GetArg();
    Assert(arg == arg_val);
}

void Now_Test() {
    uint16_t n1 = Now();
    _delay_ms(2);
    uint16_t n2 = Now();

    Assert(n1 < n2);
}

void Pid_Test() {
    // Hard to know what pid we are supposed to have
    // Just check that the pid is valid
    PID pid = Task_Pid();
    Assert(pid >= 0 && pid < MAXTHREAD);
}

void OSFN_Test() {
    /* Task_Create_RR(Arg_Test, arg_val); */
    /* Now_Test(); */
    Pid_Test();

    // Wait for tasks to be run
    _delay_ms(100);
}
