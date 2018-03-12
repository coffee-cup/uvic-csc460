#include <stdint.h>
#include <util/delay.h>
#include "kernel.h"
#include "os.h"

/**
 * Aborts the RTOS and enters a "non-executing" state with an error code.
 * That is, all tasks will be stopped.
 */
void OS_Abort(ABORT_CODE error) {

    // Disable interrupts
    OS_DI();

    // Blink the built-in LED in accordance with the error code
    BIT_SET(DDRB, 7); // Set PB7 as output
    BIT_CLR(PORTB, 7);
    uint8_t ctr;
	for(;;) {
		for (ctr = 0; ctr < error; ctr += 1) {
            BIT_SET(PORTB, 7);
            _delay_ms(200);
            BIT_CLR(PORTB, 7);
            _delay_ms(200);
        }
		_delay_ms(1000);
	}
}

PID Task_Create_System(taskfuncptr f, int16_t arg) {

    KERNEL_REQUEST_PARAMS info = {
        .request = CREATE,
        .priority = SYSTEM,
        .code = f,
        .arg = arg
    };

    Kernel_Request(&info);
    return -1;
}

PID Task_Create_RR(taskfuncptr f, int16_t arg) {

    KERNEL_REQUEST_PARAMS info = {
        .request = CREATE,
        .priority = RR,
        .code = f,
        .arg = arg
    };

    Kernel_Request(&info);
    return -1;
}

PID Task_Create_Period(taskfuncptr f, int16_t arg, TICK period, TICK wcet, TICK offset) {

    KERNEL_REQUEST_PARAMS info = {
        .request = CREATE,
        .priority = SYSTEM,
        .code = f,
        .arg = arg,
        .period = period,
        .wcet = wcet,
        .offset = offset
    };

    Kernel_Request(&info);
    return -1;
}

void Task_Next() {
    KERNEL_REQUEST_PARAMS info = {
        .request = NEXT
    };

    Kernel_Request(&info);
}

int16_t Task_GetArg() {
    return -1;
}

PID Task_Pid() {
    return -1;
}

/**
 * The calling task terminates itself.
 */
void Task_Terminate()
{
    KERNEL_REQUEST_PARAMS info = {
        .request = TERMINATE
    };

    Kernel_Request(&info);
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

int main(void) {

    BIT_SET(DDRB, 7);
    BIT_CLR(PORTB, 7);

    BIT_SET(DDRB, 0);
    BIT_SET(DDRB, 1);

    Kernel_Init();

    /* Can't add tasks here since Kernel_Request doesn't return until KernelActive is truthy */
    Kernel_Start();

    /* Control should never reach this point */
    OS_Abort(FAILED_START);

    return -1;
}
