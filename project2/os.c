#include <stdint.h>
#include "os.h"

void OS_Abort(uint16_t error) {}

void Task_Create(voidfuncptr f) {
    Kernel_Task_Create(f);
}

PID Task_Create_System(voidfuncptr f, int16_t arg) {
    return -1;
}

PID Task_Create_RR(voidfuncptr f, int16_t arg) {
    return -1;
}

PID Task_Create_Period(voidfuncptr f, int16_t arg, TICK period, TICK wcet, TICK offset) {
    return -1;
}

void Task_Next() {
    Kernel_Request(NEXT);
}

int16_t Task_GetArg() {
    return -1;
}

PID Task_Pid() {
    return -1;
}

void Msg_Send(PID id, MTYPE t, uint16_t* v) {

}

PID Msg_Recv(MASK m, uint16_t* v) {
    return -1;
}

void Msg_Rply(PID id, uint16_t r) {

}

void Msg_ASend(PID id, MTYPE t, uint16_t v) {

}

uint16_t Now() {
    return -1;
}
