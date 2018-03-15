#ifndef _TEST_UTILS_H_
#define _TEST_UTILS_H_

<<<<<<< HEAD
#include "uart.h"

#define Assert(expr)                         \
    {                                        \
        if (!(expr)) {                       \
            UART_print("Assert Failed!\n");  \
            for (;;) {}                      \
        }                                    \
=======
#define Assert(expr)                            \
    {                                           \
        if (!(expr)) while (TRUE);              \
>>>>>>> dev/zev-project2
    }

#define AssertNeverCalled()                     \
    {                                           \
        PORTE = 0xFF;                           \
    }

#endif
