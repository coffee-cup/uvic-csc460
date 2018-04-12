#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "process.h"
#include "message.h"

#define DELEGATE_MAIN() \
int main(void) __attribute__ ((weak)); \
int main(void) { }

void Kernel_Request(KERNEL_REQUEST_PARAMS* info);

#endif
