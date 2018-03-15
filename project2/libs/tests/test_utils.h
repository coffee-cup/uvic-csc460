#ifndef _TEST_UTILS_H_
#define _TEST_UTILS_H_

#define Assert(expr)                            \
    {                                           \
        if (!(expr)) while (TRUE);              \
    }

#define AssertNeverCalled()                     \
    {                                           \
        PORTE = 0xFF;                           \
    }

#endif
