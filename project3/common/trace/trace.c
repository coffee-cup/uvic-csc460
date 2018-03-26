#include "trace.h"

// Array to hold the elements in the trace
uint8_t trace_array[TRACE_ARRAY_SIZE];

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
    UART_Init0(LOGBAUD);

    int i;
    for (i = 0; i < trace_counter; i += 1) {
        UART_Transmit0(trace_array[i]);
    }
    UART_Transmit0('\n');
}

// Compares an array with the trace
// Returns 0 if it does not match
int compare_trace(uint8_t arr[]) {
    int i;
    for (i = 0; i < trace_counter; i += 1) {
        // 0 represents empty spot in array, do not compare
        if (arr[i] != trace_array[i]) {
            return 0;
        }
    }
    return 1;
}
