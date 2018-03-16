#ifndef _TEST_UTILS_H_
#define _TEST_UTILS_H_

#include "uart.h"

#define Assert(expr)                         \
    {                                        \
        if ((!(expr)) || PORTE != 0) {       \
            UART_print(                      \
                "Assert Failed! %s : %d\n",  \
                __FILE__, __LINE__);         \
            for (;;) {}                      \
        }                                    \
    }

#define AssertNeverCalled()                     \
    {                                           \
        PORTE = 0xFF;                           \
    }

#endif
