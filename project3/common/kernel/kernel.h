#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "process.h"
#include "message.h"

void Kernel_Request(KERNEL_REQUEST_PARAMS* info);
int Kernel_Begin(void);

#endif
