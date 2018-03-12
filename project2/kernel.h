#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "common.h"
#include "process.h"

void Kernel_Request(KERNEL_REQUEST_PARAMS* info);

// TODO: These should not be public
void Kernel_Init();
void Kernel_Start();

#endif
