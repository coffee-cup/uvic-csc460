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

    #ifdef RUN_TESTS
        // Is there a pending abort request?
        if (MASK_TEST_ALL(PORTE, 0x0F)) {
            LOG("Unhandled OS Abort while testing. Error code: %d\n", error);
            for (;;);
        } else {
            // We might have been expecting an abort,
            // don't quit just yet.
            MASK_SET(PORTE, 0x0F);
        }
    #else

    KERNEL_REQUEST_PARAMS info = {
        .request = ABORT,
        .abort_code = error
    };
    Kernel_Request(&info);

    #endif
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
        .priority = PERIODIC,
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
    *v = info.msg_data;
}

PID Msg_Recv(MASK m, uint16_t* v) {
    KERNEL_REQUEST_PARAMS info = {
        .request = MSG_RECV,
        .msg_mask = m
    };

    Kernel_Request(&info);

    if (info.msg_ptr_data != NULL) {
        // Data normal send
        *v = *info.msg_ptr_data;
    } else {
        // Data from async send
        *v = info.msg_data;
    }
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
        .request = MSG_ASEND,
        .msg_mask = t,
        .msg_to = id,
        .msg_data = v
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
