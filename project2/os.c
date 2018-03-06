#include "os.h"

void OS_Abort(unsigned int error) {}

PID   Task_Create_System(void (*f)(void), int arg) {}

PID   Task_Create_RR(    void (*f)(void), int arg) {}

PID   Task_Create_Period(void (*f)(void), int arg, TICK period, TICK wcet, TICK offset) {}

void Task_Next(void) {}

int  Task_GetArg(void) {}

PID  Task_Pid(void) {}

void Msg_Send( PID  id, MTYPE t, unsigned int *v ) {}

PID  Msg_Recv( MASK m,           unsigned int *v ) {}

void Msg_Rply( PID  id,          unsigned int r ) {}

void Msg_ASend( PID  id, MTYPE t, unsigned int v ) {}

unsigned int Now() {}
