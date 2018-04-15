# Hardware Interfaces

Following the requirements of project 2 that are implicitly translated to this project, no Arduino code or libraries should be used. This meant that we needed to implement some of the hardware interfaces ourselves. Arduino provides functions such as `analogRead` and classes such as `Serial` and `Servo` that would need to be re-implemented. The `analogRead` function is used to invoke the onboard analog to digital converter, typically used for determining position of the X and Y axis of the joysticks, but also for the light sensors. The `Serial` class is for using one of the four onboard universal asynchronous receiver transmitters (UART) chips. UART is used for sending data to peripheral devices, in this project UART is typically used for sending data over WiFi, issuing commands to the Roomba, and for logging text to a connected debug console. Finally, the `Servo` class is used to generate pulse width modulated signals specifically for the purpose of controlling the position of servo motors. Important details of how each of these Arduino functionalities was replaced is described in this section.

## Analog to Digital Converter (ADC)

There are 15 ADC pins available on the ATMega2560. Before the values can be read from the joysticks or light sensors, the analog to digital inputs on the board must be initialized. This consists of setting the prescaler bits on the ADC control and status register (`ADCSRA`). We then set the ADC voltage reference to the analog supply voltage (`AVCC`), doing so will help consistency of the readings since we assume the peripheral device is powered by the same power supply as the ATMega2560 is using. The ADC output is left adjusted so we can easily read the least significant 8 bit values. Finally we enable the ADC by setting the enable bit and performing the first ADC initialization conversion.

![ATMega2560 Analog to Digital Converter Schematic](https://i.imgur.com/sxeWFGH.png)

Our initialization code is shown below.

```c
void analog_init() {
    // Configure analog inputs using ADC
    BIT_SET(ADCSRA, ADPS2);
    BIT_SET(ADCSRA, ADPS1);
    BIT_SET(ADCSRA, ADPS0);

    // Set ADC reference to AVCC
    BIT_SET(ADMUX, REFS0);

    // Left adjust ADC result to allow easy 8 bit reading
    BIT_SET(ADMUX, ADLAR);

    // Enable ADC
    BIT_SET(ADCSRA, ADEN);

    // Start a conversion to warmup the ADC
    BIT_SET(ADCSRA, ADSC);
}
```

To read a value from analog pin 0:15, we need to configure the ADC multiplexer selection register (`ADMUX`). We select the analog channel being read, start the conversion and wait for the conversion to complete. When it completes (the `ADSC` bit of the ADC status control register is 0), we read the low and high bits of the resulting conversion. A 10 bit value between 0 and 1023 is returned. Our analog read function is shown below.

```c
uint16_t analog_read(uint8_t channel) {
    /* Set the three LSB of the Mux value. */
    /* Caution modifying this line, we want MUX4 to be set to zero, always */
    ADMUX = (ADMUX & 0xF0 ) | (0x07 & channel);

    /* We set the MUX5 value based on the fourth bit of the channel, see page 292 of the
     * ATmega2560 data sheet for detailed information */
    ADCSRB = (ADCSRB & 0xF7) | (channel & (1 << MUX5));

    /* We now set the Start Conversion bit to trigger a fresh sample. */
    BIT_SET(ADCSRA, ADSC);

    /* We wait on the ADC to complete the operation, when it completes, the hardware
       will set the ADSC bit to 0. */
    while (BIT_TEST(ADCSRA, ADSC))
        ;

    /* We setup the ADC to shift output to left, so we have to recombine the output. */
    return (ADCL >> 6) | (ADCH << 2);
}
```
![ADC Output Register content for ADLAR = 1](https://i.imgur.com/t6btclN.png)


## Universal Asynchronous Receiver Transmitter (UART)

The ATMega2560 has 4 USART chips available. USART stands for Universal *Synchronous* or Asynchronous Receiver Transmitter. Fortunately USART supports UART, as this is what we would need to use in our project. Three basic functions need to be implemented to use the UART chips:

1. Initialization
2. Reading
3. Writing

Our first implementation only supported blocking read. That is to say, calling `UART_Receive` would block execution until data became available, a potentially dangerous operation in a system where timing is critically important. If the caller expected $n$ bytes to be available but less than $n$ bytes arrived in the receiver, then the program would hang until more bytes arrived. Furthermore, since the ATMega2560's receive buffer is a mere 2 bytes, the application would frequently drop data if the receive function was not being polled frequently enough. To alleviate this issue, the `UART_Receive` function was depreciated and replaced by a combination of Receive Interrupts, ring buffers, and a new function `UART_AsyncReceive`.

A ring buffer is implemented as an array where indexes which exceed the length of the array are wrapped back to the beginning. Conceptually a ring buffer in several different states can be visualized as shown here, images from [Grijjy](https://blog.grijjy.com/2017/01/12/).

[RingBuffer_1]: https://i.imgur.com/Itv6c12.png "Ring buffer example"
[RingBuffer_2]: https://i.imgur.com/PgoRQYA.png "Ring buffer example"
[RingBuffer_3]: https://i.imgur.com/gV3mqZZ.png "Ring buffer example"

| ![][RingBuffer_1]    | ![][RingBuffer_2]    | ![][RingBuffer_3]            |
|:--------------------:|:--------------------:|:----------------------------:|
| 1. Ring buffer state | 2. A read occurs     | 3. A subsequent write occurs |

This approach to receiving data allows the application to query how many bytes are available and allows for non-blocking reading. However, it is important to be wary of race conditions in implementations such as this, so when reading or writing to the ring buffers interrupts should be disabled.

### UART Initialization

In order to generalize the new UART implementation to the 4 available chips, each function will first take a channel argument. The channel argument is used to index into various arrays of pointers to the memory locations for the associated UART device. For example, the `<avr/io.h>` header file defines memory addresses for the baudrate registers, control and status registers, and data registers. In our implementation, these are all wrapped into arrays where the index corresponds to the channel number.

```c
volatile uint16_t* UBRRn[4]  = {&UBRR0,  &UBRR1,  &UBRR2,  &UBRR3 }; // Baud rate registers
volatile uint8_t*  UCSRnA[4] = {&UCSR0A, &UCSR1A, &UCSR2A, &UCSR3A}; // Control/Status reg.
volatile uint8_t*  UCSRnB[4] = {&UCSR0B, &UCSR1B, &UCSR2B, &UCSR3B}; // Control/Status reg.
volatile uint8_t*  UDRn[4]   = {&UDR0,   &UDR1,   &UDR2,   &UDR3  }; // Data registers
```

Furthermore, the `<avr/io.h>` header also defines indexes for the bits in each of those registers. These will be also be extensively used in the code for our UART implementation.

```c
uint8_t RXCIEn[4] = {RXCIE0, RXCIE1, RXCIE2, RXCIE3}; // Receive Interrupt Enable Flag Bits
uint8_t RXENn[4]  = {RXEN0,  RXEN1,  RXEN2,  RXEN3 }; // Receive Enable Flag Bits
uint8_t RXCn[4]   = {RXC0,   RXC1,   RXC2,   RXC3  }; // Receive Complete Flag Bits
uint8_t TXENn[4]  = {TXEN0,  TXEN1,  TXEN2,  TXEN3 }; // Transmit Enable Flag Bits
uint8_t UDREn[4]  = {UDRE0,  UDRE1,  UDRE2,  UDRE3 }; // Data Register Empty Flag Bits
```

Lastly, since our UART implementation uses ring buffers for temporarily storing received data, the following 2 dimensional array and was defined, accompanied by last read and write indexes for each of the 4 main dimensions:

```c
static volatile uint8_t  _RXBUFn[4][RX_BUFFER_SIZE];  // Receive ring buffers
static volatile uint16_t _RXIn[4] = {0, 0, 0, 0};     // Ring buffer last write index
static volatile uint16_t _RXRn[4] = {0, 0, 0, 0};     // Ring buffer last read index
static volatile bool     _RXWn[4] = {FALSE, FALSE, FALSE, FALSE}; // Ring buffer wrapped flag
```

With these global variables defined, initializing a UART device boils down to simply updating the associated status registers.

```c
void UART_Init(uint8_t chan, uint32_t baud_rate) {
    ... // Validation

    *UBRRn[chan] = (F_CPU / 16 / baud_rate) - 1;
    *UCSRnB[chan] = 0
                  | 1 << (TXENn[chan])   // Enable Transmit
                  | 1 << (RXENn[chan])   // Enable Receive
                  | 1 << (RXCIEn[chan]); // Enable Interrupt on Receive
}
```

### UART Reading / Receiving

As our implementation uses the interrupt on receive functionality of the ATMega2560's UART chips, we pre-declared an Interrupt Service Routine for each channel. `UART_ISR(uint8_t channel)` is the generalized interrupt handler for all channels. These ISRs run only if the `RXCIE` bit is set in the associated UART control registers, set during initialization.

```c
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
```

Then, the program must simply update the ring buffers appropriately in the `UART_ISR` function, which reads in one byte per execution from the UART Data Register.

```c
void UART_ISR(uint8_t chan) {
    ... // Validation

    // Busy wait for data to be received, should
    // be instant, since the ISR ran
    while ( !((*UCSRnA[chan]) & _BV(RXCn[chan])))
        ;

    // Check ring buffer size
    if (_RXWn[chan] && _RXRn[chan] == _RXIn[chan]){
        // Ring buffer is full, drop incoming data

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
            _RXWn[chan] = TRUE;
        }
    }
}
```

### UART Writing / Transmitting

Writing data over UART is not buffered and *is* blocking. We did not encounter any issues with this approach. It does however require that the peripheral devices consume data immediately so that our application does not hang while waiting for data to be transmitted.

The transmission function (`UART_Transmit`) writes one byte per execution to the UART Data Register. This can be done in succession to allow the applcation to print multiple bytes to a UART channel.

```c
void UART_Transmit(uint8_t chan, uint8_t byte) {
    ... // Validation

    // Critical section, save program status register
    uint16_t old_sreg = SREG;
    cli(); // Disable Interrupts

    // Busy wait for empty transmit buffer
    while (!((*UCSRnA[chan]) & (1 << UDREn[chan])))
        ;

    // Put data into buffer, sends the data
    *UDRn[chan] = byte;

    // Restore status register
    SREG = old_sreg;
}
```

## Pulse Width Modulation (PWM)

The last major Ardunio functionality that needed to be implemented was the ability to generate PWM signals. A pulse width modulated signal is a periodic square wave signal with configurable duty cycle. The ATMega2560 has more than sufficient PWM support, 12 16-bit resolution PWM channels are available, but as our implementation needed only to control two servos, our PWM support was limited to two channels.

The ATMega2560 supports various PWM modes, but for our purposes we used PWM mode 15 (Fast PWM). Fast PWM mode uses a combination of timers and comparison values to generate PWM signals.

![Fast PWM](https://i.imgur.com/4exBOb2.png)

The diagram above depicts two PWM signals being generated by a single counter/timer. The counter/timer is configured as a counter with automatic reset to 0 when the value reaches the maximum. When this reset occurs, the PWM output signals are set high across all output channels. When the counter value is equal (or greater) to a value associated with the PWM channel the corresponding channel output signal is set to low again.

By changing the prescaler on the counter the period of the PWM signal can be adjusted, and by changing the comparison value the duty cycle of the output signal can be adjusted. On the ATMega2560, we used timer/counter 3 as 4 was being used for the operating system clock. The comparison values are held in registers called Output Compare Registers. PWM is performed through 3 functions in our code. `PWM_Init` starts the counter, `PWM_Attach` enables wave generation on a specified pin and returns the address of the associated output compare register, and finally `PWM_Write` updates the value in the output compare register.

```c
void PWM_Init() {
    // Set Wave Generation mode to Fast PWM (mode 15)
    // Counter counts up and resets to BOTTOM (value 0) at TOP
    MASK_SET(TCCR3A, _BV(WGM30) | _BV(WGM31));
    MASK_SET(TCCR3B, _BV(WGM32) | _BV(WGM33));

    // Set prescaler to 64
    MASK_SET(TCCR3B, _BV(CS31) | _BV(CS30));

    // Set TOP value for a 20 ms period based on a 1/64 prescaler
    OCR3A = 5000;
}

void PWM_Write(volatile uint16_t* OCR3n, uint16_t micro_seconds) {
    // Careful about the values written. This PWM implementation is
    // for controlling Servo motors, so don't allow values outside their
    // Operational range.
    // Min = 5000 / 2000 * 50 = 125
    // Max = 5000 / 2000 * 250 = 625
    uint16_t value = cmap_u(micro_seconds, 500, 2500, 125, 625);
    *OCR3n = value;
}

```
