#ifndef __UART_H__
#define __UART_H__

#include <avr/common.h>

#define TX_BUFFER_SIZE 64

#define MYBRR(baud_rate) (F_CPU / 16 / (baud_rate) - 1)

void UART_Init0(uint32_t baud_rate);
void UART_Transmit0(unsigned char data);
unsigned char UART_Receive0();
void UART_Init1(uint32_t baud_rate);
void UART_Transmit1(unsigned char data);
unsigned char UART_Receive1();
void UART_print(const char* fmt, ...);
void UART_send_raw_bytes(const uint8_t num_bytes, const uint8_t* data);
int8_t UART_Receive1_Non_Blocking();

#endif
