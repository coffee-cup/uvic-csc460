#ifndef _TEST_UTILS_H_
#define _TEST_UTILS_H_

#include "uart.h"

#define Assert(expr)                     \
{                                        \
    if (!(expr)) {                       \
        UART_print(                      \
            "Assert Failed! "            \
            "%s : %d\n",                 \
            __FILE__, __LINE__);         \
        for (;;) {}                      \
    }                                    \
}

#define AssertAborted()                  \
{                                        \
    if ((PORTE & 0x0F) == 0x00) {        \
        UART_print(                      \
            "Abort assertion failed "    \
            "at %s : %d\n",              \
            __FILE__, __LINE__);         \
        for (;;) {}                      \
    } else {                             \
        PORTE &= ~0x0F;                  \
    }                                    \
}

#define AssertNeverCalled()              \
{                                        \
    PORTE |= 0xF0;                       \
}

#endif
