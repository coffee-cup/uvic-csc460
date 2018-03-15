#include <stdint.h>
#include <util/delay.h>
#include "kernel.h"
#include "os.h"
#include "uart.h"

/**
 * Aborts the RTOS and enters a "non-executing" state with an error code.
 * That is, all tasks will be stopped.
 */
void OS_Abort(ABORT_CODE error) {

    // Disable interrupts
    OS_DI();
    Kernel_Abort();
    UART_Init0(38400);

    // Blink the built-in LED in accordance with the error code
    BIT_SET(DDRB, 7); // Set PB7 as output
    BIT_CLR(PORTB, 7);
    uint8_t ctr;

    char buffer[20];
    sprintf(buffer, "OS Abort. Error code: %d\n", error);

    for(;;) {
        UART_print(buffer);
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
    return info.out_pid;
}

PID Task_Create_RR(taskfuncptr f, int16_t arg) {

    KERNEL_REQUEST_PARAMS info = {
        .request = CREATE,
        .priority = RR,
        .code = f,
        .arg = arg
    };

    Kernel_Request(&info);
    return info.out_pid;
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
    return info.out_pid;
}

void Task_Next() {
    KERNEL_REQUEST_PARAMS info = {
        .request = NEXT
    };

    Kernel_Request(&info);
}

int16_t Task_GetArg() {
    KERNEL_REQUEST_PARAMS info = {
        .request = GET_ARG
    };

    Kernel_Request(&info);

    return info.arg;
}

PID Task_Pid() {
    KERNEL_REQUEST_PARAMS info = {
        .request = GET_PID
    };

    Kernel_Request(&info);
    return info.out_pid;
}

/**
 * The calling task terminates itself.
 */
void Task_Terminate() {
    KERNEL_REQUEST_PARAMS info = {
        .request = TERMINATE
    };

    Kernel_Request(&info);
}

void Msg_Send(PID id, MTYPE t, uint16_t* v) {
    KERNEL_REQUEST_PARAMS info = {
        .request = MSG_SEND,
        .msg_ptr_data = v,
        .msg_mask = t,
        .msg_to = id
    };

    Kernel_Request(&info);
    v = &info.msg_data;
}

PID Msg_Recv(MASK m, uint16_t* v) {
    KERNEL_REQUEST_PARAMS info = {
        .request = MSG_RECV,
        .msg_mask = m
    };

    Kernel_Request(&info);
    v = info.msg_ptr_data;
    return info.out_pid;
}

void Msg_Rply(PID id, uint16_t r) {
    KERNEL_REQUEST_PARAMS info = {
        .request = MSG_RPLY,
        .msg_to = id,
        .msg_data = r
    };

    Kernel_Request(&info);
}

void Msg_ASend(PID id, MTYPE t, uint16_t v) {
    KERNEL_REQUEST_PARAMS info = {
        .request = MSG_ASEND
    };

    Kernel_Request(&info);
}

uint16_t Now() {
    KERNEL_REQUEST_PARAMS info = {
        .request = GET_NOW
    };

    Kernel_Request(&info);
    return info.out_now;
}
