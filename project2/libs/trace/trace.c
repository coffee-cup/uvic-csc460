#include "trace.h"

/** array that holds all the elements of the trace */
// Array to hold the elements in the trace
unsigned char trace_array[TRACE_ARRAY_SIZE];

// Current element in the trace
uint16_t volatile trace_counter = 0;

// Adds and byte to the trace
void add_to_trace(char c) {
    if (trace_counter < TRACE_ARRAY_SIZE) {
        trace_array[trace_counter] = c;
        trace_counter += 1;
    }
}

// Clears all the elements in the trace
void clear_trace(void) {
    int i;
    for (i = 0; i < TRACE_ARRAY_SIZE; i += 1) {
        trace_array[i] = 0x00;
    }
    trace_counter = 0;
}

// Prints all the elements of the trace to UART
void print_trace(void) {
    UART_Init0(38400);

    int i;
    for (i = 0; i < trace_counter; i += 1) {
        UART_Transmit0(trace_array[i]);
    }
}
