#ifndef __TRACE_H__
#define __TRACE_H__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>

#include "uart.h"
#include "../../os.h"

#define TRACE_ARRAY_SIZE         256

void add_to_trace(char c);
void clear_trace(void);
void print_trace(void);
int compare_trace(uint8_t arr[]);

#endif
