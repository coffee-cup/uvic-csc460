#ifndef _TEST_UTILS_H_
#define _TEST_UTILS_H_

#include "uart.h"

#define Assert(expr)                         \
    {                                        \
        if (!(expr)) {                       \
            UART_print("Assert Failed!\n");  \
            for (;;) {}                      \
        }                                    \
    }

#define AssertNeverCalled()                     \
    {                                           \
        PORTE = 0xFF;                           \
    }

#endif
