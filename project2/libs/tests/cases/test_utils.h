#ifndef _TEST_UTILS_H_
#define _TEST_UTILS_H_

#include "../../uart/uart.h"

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
    if (MASK_TEST_ALL(PORTE, 0x0F)) {    \
        MASK_CLR(PORTE, 0x0F);           \
    } else {                             \
        UART_print(                      \
            "Abort assertion failed "    \
            "at %s : %d\n",              \
            __FILE__, __LINE__);         \
        for (;;) {}                      \
    }                                    \
}

#define AssertNeverCalled()              \
{                                        \
    MASK_SET(PORTE, 0xF0);               \
}

#endif
