#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <LiquidCrystal.h>

uint64_t ticks = 0;

uint8_t cs = 0;
uint8_t sec = 0;
uint8_t min = 0;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

ISR(TIMER4_COMPA_vect)
{
    ticks = ticks + 1;
    cs = (ticks) % 100;
    sec = (ticks / 100) % 60;
    min = ticks / 6000;
}

void initIO() {
    DDRA = 0xFF;
    DDRB = 0xFF;
    DDRC = 0xFF;
    DDRD = 0xFF;

    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    PORTD = 0x00;
}

void setupTimer()
{
    //Clear timer config.
    TCCR4A = 0;
    TCCR4B = 0;
    //Set to CTC (mode 4)
    TCCR4B |= (1 << WGM42);

    //Set prescaller to 256
    TCCR4B |= (1 << CS42);

    //Set TOP value (0.01 seconds)
    OCR4A = 625;

    //Enable interupt A for timer 3.
    TIMSK4 |= (1 << OCIE4A);

    //Set timer to 0 (optional here).
    TCNT4 = 0;

    asm volatile("sei"::);
}

int16_t main()
{
    initIO();
    setupTimer();

    char buffer[16];

    for(;;){
        _delay_ms(/* prime */53);
        sprintf(buffer, "%02u:%02u:%02u", min, sec, cs);
        lcd.setCursor(0, 0);
        lcd.print(buffer);
    }
}
