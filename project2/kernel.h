#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "common.h"
#include "process.h"
#include <string.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#define LOW_BYTE(X) (((uint16_t)X) & 0xFF)
#define HIGH_BYTE(X) ((((uint16_t)X) >> 8) & 0xFF)
#define ZeroMemory(X, N) memset(&(X), 0, N)

void Kernel_Task_Create(voidfuncptr f);

void Kernel_Request(KERNEL_REQUEST_TYPE type);

// TODO: These should not be public
void Kernel_Init();
void Kernel_Start();

#endif
