#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "common.h"
#include "process.h"
#include "message.h"

/*
 * inline assembly code to disable/enable maskable interrupts
 * (N.B. Use with caution.)
 */
#define OS_DI()    asm volatile("cli"::)  /* disable all interrupts */
#define OS_EI()    asm volatile("sei"::)  /* enable all interrupts */
#define OS_JUMP(f) asm("jmp " #f::)       /* direct jump to assembly label */

void Kernel_Request(KERNEL_REQUEST_PARAMS* info);

#endif
