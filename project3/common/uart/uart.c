#include <avr/io.h>
#include <avr/interrupt.h>      // ISR handling.
#include <stdio.h>              // vsnprintf
#include "../os/common.h"
#include "../os/os.h"
#include "uart.h"

#define CHAN_OK(chan) (chan >= 0 && chan < 4)

/*
 Global Variables:
 Variables appearing in both ISR/Main are defined as 'volatile'.
*/
static volatile uint16_t _RXIn[4] = {0, 0, 0, 0};      // buffer 'element' counter.
static volatile uint16_t _RXRn[4] = {0, 0, 0, 0};      // index of last read.
static volatile bool     _RXWn[4] = {FALSE, FALSE, FALSE, FALSE}; // Ring buffer wrapped
static volatile uint8_t  _RXBUFn[4][RX_BUFFER_SIZE]; // buffer of 'char'.

bool uart_initialized[4] = {FALSE, FALSE, FALSE, FALSE};

volatile uint16_t* UBRRn[4]  = {&UBRR0,  &UBRR1,  &UBRR2,  &UBRR3 };
volatile uint8_t*  UCSRnA[4] = {&UCSR0A, &UCSR1A, &UCSR2A, &UCSR3A};
volatile uint8_t*  UCSRnB[4] = {&UCSR0B, &UCSR1B, &UCSR2B, &UCSR3B};
volatile uint8_t*  UDRn[4]   = {&UDR0,   &UDR1,   &UDR2,   &UDR3  };


uint8_t RXCIEn[4] = {RXCIE0, RXCIE1, RXCIE2, RXCIE3};
uint8_t TXENn[4]  = {TXEN0,  TXEN1,  TXEN2,  TXEN3};
uint8_t RXENn[4]  = {RXEN0,  RXEN1,  RXEN2,  RXEN3};
uint8_t RXCn[4]   = {RXC0,   RXC1,   RXC2,   RXC3};
uint8_t UDREn[4]  = {UDRE0,  UDRE1,  UDRE2,  UDRE3};

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
    *UCSRnB[chan] = _BV(TXENn[chan]) | _BV(RXENn[chan]) | _BV(RXCIEn[chan]);

}


void UART_Transmit(uint8_t chan, uint8_t byte) {
    if (!CHAN_OK(chan) || !uart_initialized[chan]) {
        // Bad channel or not initialized
        OS_Abort(UART_ERROR);
        return;
    }

    uint16_t old_sreg = SREG;
    cli();

    // Busy wait for empty transmit buffer
    while (!((*UCSRnA[chan]) & _BV(UDREn[chan])))
        ;

    // Put data into buffer, sends the data
    *UDRn[chan] = byte;

    SREG = old_sreg;
}


bool UART_Async_Receive(uint8_t chan, uint8_t* out) {
    if (!CHAN_OK(chan) || !uart_initialized[chan]) {
        // Bad channel or not initialized
        OS_Abort(UART_ERROR);
        return -1;
    }

    if (UART_Available(chan)) {

        uint16_t old_sreg = SREG;
        cli();

        // Return a byte from the buffer
        *out = _RXBUFn[chan][_RXRn[chan]];

        // Update the last byte read index
        _RXRn[chan] = (_RXRn[chan] + 1) % RX_BUFFER_SIZE;

        // Check for read index wrap / overflow
        if (_RXRn[chan] == 0) {
            if (_RXWn[chan]) {
                _RXWn[chan] = FALSE;
            } else {
                // The read index is ahead of the write index.
                // Evidence is that RXRn wrapped, but RXWn shows
                // that RXIn hasn't wrapped
                LOG("Out of bounds UART read\n");
                OS_Abort(UART_ERROR);
            }
        }

        SREG = old_sreg;

        return TRUE;
    } else {
        return FALSE;
    }
}


bool UART_Available(uint8_t chan) {
    if (!CHAN_OK(chan) || !uart_initialized[chan]) {
        // Bad channel or not initialized
        OS_Abort(UART_ERROR);
        return FALSE;
    }

    uint16_t old_sreg = SREG;
    cli();
    bool avail = _RXWn[chan] || _RXRn[chan] < _RXIn[chan];
    SREG = old_sreg;

    // Data has arrived since last read
    return avail;
}


bool UART_BytesAvailable(uint8_t chan, uint16_t num) {
    if (!CHAN_OK(chan) || !uart_initialized[chan]) {
        // Bad channel or not initialized
        OS_Abort(UART_ERROR);
        return FALSE;
    }

    uint16_t old_sreg = SREG;
    cli();
    uint16_t last_read = _RXRn[chan];
    uint16_t last_rxd  = _RXIn[chan];
    bool     has_wrapped = _RXWn[chan];

    SREG = old_sreg;

    if (!has_wrapped) {
        return last_rxd - last_read >= num;
    } else {
        return RX_BUFFER_SIZE - last_read + last_rxd >= num;
    }
}

void UART_Flush(uint8_t chan) {
    if (!CHAN_OK(chan) || !uart_initialized[chan]) {
        // Bad channel or not initialized
        OS_Abort(UART_ERROR);
        return;
    }

    uint16_t old_sreg = SREG;
    cli();

    _RXRn[chan] = _RXIn[chan];
    _RXWn[chan] = FALSE;

    SREG = old_sreg;
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
    size = vsnprintf((char*)buffer, TX_BUFFER_SIZE, fmt, args);
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

void UART_ISR(uint8_t chan) {
    if (!CHAN_OK(chan) || !uart_initialized[chan]) {
        // Bad channel or not initialized
        OS_Abort(UART_ERROR);
        return;
    }

    // Busy wait for data to be received, should
    // be instant, since the ISR ran
    while ( !((*UCSRnA[chan]) & _BV(RXCn[chan])))
        ;

    if (_RXWn[chan] && _RXRn[chan] == _RXIn[chan]){
        // Ring buffer is full
        LOG("UART RX[%u] full: Dropping data!\n", chan);
        uint8_t dummy;
        while ( *UCSRnA[chan] & _BV(RXCn[chan]) ) dummy = *UDRn[chan];
        (void) dummy;

    } else {
        // Write the rx data into the ring buffer
        _RXBUFn[chan][_RXIn[chan]] = *UDRn[chan];

        // Update the ring buffer index
        _RXIn[chan] = (_RXIn[chan] + 1) % RX_BUFFER_SIZE;

        // Check for index wrap / overflow
        if (_RXIn[chan] == 0) {
            if (_RXWn[chan]) {
                LOG("!! Wrapped again?!\n");
                OS_Abort(UART_ERROR);
            } else {
                _RXWn[chan] = TRUE;
            }
        }
    }
}

ISR(USART0_RX_vect) {
    UART_ISR(0);
}

ISR(USART1_RX_vect) {
    UART_ISR(1);
}

ISR(USART2_RX_vect) {
    UART_ISR(2);
}

ISR(USART3_RX_vect) {
    UART_ISR(3);
}

