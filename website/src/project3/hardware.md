# Hardware

Most of the hardware used in this project was also used in [project 1](/project1). However, in project 1 we used the Arduino libraries to interface with the hardware. In this project no Arduino libraries were used. This section will discuss how we implemented with the hardware components using ATMega2560 registers.

## ATMega2560

## Servos

## Joysticks

We used the same joysticks as we used in project 1. They allow the user to input X, Y, and Z axis positions using their thumb.

![Joystick](https://i.imgur.com/v6BjQxS.png)

The X and Y positions are analog signals which is controlled internally by a variable resistor. In order to read the positions of the X and Y values, the internal analog to digital converter must be used. There are 15 ADC pins available on the ATMega2560. Before the values can be read from the joystick, the analog to digital inputs on the board must be initialized. This consists of setting the prescaler bits on the ADC control and status register (`ADCSRA`). We then set the ADC voltage reference to the analog supply voltage (`AVCC`). The ADC is left adjusted so we can easily read 8 bit values. Finally we enable the ADC by setting the enable bit and performing the first ADC initialization conversion. Our initialization code is shown below.

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

To read a value from analog pin 0:15, we need to deal with the ADC multiplexer selection register (`ADMUX`). We select the analog channel being read, start the conversion and wait for the conversion to complete. When it completes (the `ADSC` bit of the ADC status control register is 0), we read the low and high bits of the resulting conversion. A 10 bit value between 0 and 1023 is returned. Our analog read function is shown below.

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
    while (BIT_TEST(ADCSRA, ADSC));

    /* We setup the ADC to shift input to left, so we simply return the High register. */
    uint8_t lowADC = ADCL;
    uint8_t highADC = ADCH;
    return (lowADC >> 6) | (highADC << 2);
}
```

![ATMega2560 Analog to Digital Converter Schematic](https://i.imgur.com/sxeWFGH.png)

## Lasers
