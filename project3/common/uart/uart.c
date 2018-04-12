#include <avr/io.h>
#include <avr/interrupt.h>      // ISR handling.
#include <stdio.h>              // vsnprintf
#include "../os/common.h"
#include "../os/os.h"
#include "uart.h"

#define CHAN_OK(chan) (chan >= 0 && chan < 4)

bool uart_initialized[4] = {FALSE, FALSE, FALSE, FALSE};
volatile uint16_t* UBRRn[4] = {&UBRR0, &UBRR1, &UBRR2, &UBRR3};
volatile uint8_t*  UCSRnA[4] = {&UCSR0A, &UCSR1A, &UCSR2A, &UCSR3A};
volatile uint8_t*  UCSRnB[4] = {&UCSR0B, &UCSR1B, &UCSR2B, &UCSR3B};
volatile uint8_t*  UDRn[4]   = {&UDR0,   &UDR1,   &UDR2,   &UDR3  };

uint8_t TXENn[4] = {TXEN0, TXEN1, TXEN2, TXEN3};
uint8_t RXENn[4] = {RXEN0, RXEN1, RXEN2, RXEN3};
uint8_t RXCn[4]  = {RXC0,  RXC1,  RXC2,  RXC3};
uint8_t UDREn[4] = {UDRE0, UDRE1, UDRE2, UDRE3};

uint32_t current_bauds[4] = {0, 0, 0, 0};


void UART_Init(uint8_t chan, uint32_t baud_rate) {
    if (!CHAN_OK(chan)) {
        // Bad channel
        OS_Abort(UART_ERROR);
        return;
    }

    // Request to change baud rate?
    if (baud_rate == current_bauds[chan]) {
        // Baud is unchanged
        return;
    } else if (current_bauds[chan] != 0) {
        LOG("Changed baud rate for channel %u\n", chan);
    }

    uart_initialized[chan] = TRUE;
    current_bauds[chan] = baud_rate;
    *UBRRn[chan] = MYBRR(baud_rate);
    *UCSRnB[chan] = _BV(TXENn[chan]) | _BV(RXENn[chan]);
}


void UART_Transmit(uint8_t chan, uint8_t byte) {
    if (!CHAN_OK(chan) || !uart_initialized[chan]) {
        // Bad channel or not initialized
        OS_Abort(UART_ERROR);
        return;
    }

    // Busy wait for empty transmit buffer
    while (!((*UCSRnA[chan]) & _BV(UDREn[chan])))
    ;
    // Put data into buffer, sends the data
    *UDRn[chan] = byte;
}


uint8_t UART_Receive(uint8_t chan) {
    if (!CHAN_OK(chan) || !uart_initialized[chan]) {
        // Bad channel or not initialized
        OS_Abort(UART_ERROR);
        return -1;
    }

    // Busy wait for data to be received
    while (!((*UCSRnA[chan]) & _BV(RXCn[chan])))
    ;

    // Get and return received data from buffer
    return *UDRn[chan];
}


bool UART_Available(uint8_t chan) {
    if (!CHAN_OK(chan) || !uart_initialized[chan]) {
        // Bad channel or not initialized
        OS_Abort(UART_ERROR);
        return FALSE;
    }
    // Data has arrived
    return ((*UCSRnA[chan]) & _BV(RXCn[chan]));
}

bool UART_Writable(uint8_t chan){
    if (!CHAN_OK(chan) || !uart_initialized[chan]) {
        // Bad channel or not initialized
        OS_Abort(UART_ERROR);
        return FALSE;
    }
    // Transmit buffer empty
    return ((*UCSRnA[chan]) & _BV(UDREn[chan]));
}


bool UART_Async_Receive(uint8_t chan, uint8_t* out) {
    if (!CHAN_OK(chan) || !uart_initialized[chan]) {
        // Bad channel or not initialized
        OS_Abort(UART_ERROR);
        return FALSE;
    }

    if ((*UCSRnA[chan]) & _BV(RXCn[chan])) {
        *out = *UDRn[chan];
        return TRUE;
    }

    return FALSE;
}

void UART_print(uint8_t chan, const char* fmt, ...) {
    if (!CHAN_OK(chan) || !uart_initialized[chan]) {
        // Bad channel or not initialized
        OS_Abort(UART_ERROR);
        return;
    }

    uint16_t old_sreg = SREG;
    cli();

    uint8_t buffer[TX_BUFFER_SIZE];
    size_t size;
    va_list args;

    va_start(args, fmt);
    size = vsnprintf((char*)buffer, sizeof(buffer), fmt, args);
    va_end(args);

    UART_send_raw_bytes(chan, size, buffer);

    SREG = old_sreg;
}


void UART_send_raw_bytes(uint8_t chan, const uint8_t num_bytes, const uint8_t* data) {
    if (!CHAN_OK(chan) || !uart_initialized[chan]) {
        // Bad channel or not initialized
        OS_Abort(UART_ERROR);
        return;
    }

    uint8_t i;
    for (i = 0; i < num_bytes; i++) {
        UART_Transmit(chan, data[i]);
    }
}
