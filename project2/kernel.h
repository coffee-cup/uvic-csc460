#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "common.h"
#include "process.h"

void Kernel_Task_Create(voidfuncptr f);
void Kernel_Request(KERNEL_REQUEST_TYPE type);

// TODO: These should not be public
void Kernel_Init();
void Kernel_Start();

#endif
