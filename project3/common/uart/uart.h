#ifndef __UART_H__
#define __UART_H__

#include <avr/common.h>

#define TX_BUFFER_SIZE 64
#define RX_BUFFER_SIZE 32

#define MYBRR(baud_rate) (F_CPU / 16 / (baud_rate) - 1)

void UART_Init(uint8_t chan, uint32_t baud_rate);
void UART_Transmit(uint8_t chan, uint8_t byte);
bool UART_Async_Receive(uint8_t chan, uint8_t* out);

bool UART_Available(uint8_t chan);
bool UART_BytesAvailable(uint8_t chan, uint16_t num);

bool UART_Writable(uint8_t chan);

void UART_print(uint8_t chan, const char* fmt, ...);
void UART_send_raw_bytes(uint8_t chan, const uint8_t num_bytes, const uint8_t* data);

#endif
