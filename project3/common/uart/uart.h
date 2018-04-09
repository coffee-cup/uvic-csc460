#ifndef __UART_H__
#define __UART_H__

#include <avr/common.h>

#define TX_BUFFER_SIZE 64

#define MYBRR(baud_rate) (F_CPU / 16 / (baud_rate) - 1)

void UART_Init(uint8_t chan, uint32_t baud_rate);
void UART_Transmit(uint8_t chan, uint8_t byte);
uint8_t UART_Receive(uint8_t chan);
void UART_print(uint8_t chan, const char* fmt, ...);
void UART_send_raw_bytes(uint8_t chan, const uint8_t num_bytes, const uint8_t* data);
int8_t UART_Async_Receive(uint8_t chan, uint8_t* out);

#endif
