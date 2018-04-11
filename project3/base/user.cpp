#include <avr/io.h>
#include <util/delay.h>
#include <LiquidCrystal.h>
#include "Keypad.h"
#include "Joystick.h"
#include "Packet.h"

extern "C" {
    #include "kernel.h"
    #include "os.h"
    #include "common.h"
    #include "uart.h"
    #include "utils.h"
    #include "timings.h"
    void create(void);
}

#define DELTA_CHAR '\x07'
#define YBAR_CHAR  '\x06'
#define XBAR_CHAR  '\x05'
#define SWON_CHAR  '\x04'
#define SWOFF_CHAR '\x03'

#define CLEAR_TERM "\x1B[2J\x1B[H"

typedef struct {
    const uint8_t joy1X;
    const uint8_t joy1Y;
    const uint8_t joy1SW;
    const uint8_t joy2X;
    const uint8_t joy2Y;
    const uint8_t joy2SW;
} PinDefs;

PinDefs pin = {
    .joy1X  = 15,
    .joy1Y  = 14,
    .joy1SW = 1, // PORTA
    .joy2X  = 12,
    .joy2Y  = 13,
    .joy2SW = 0 // PORTA
};

Packet packet(512, 512, 0, 512, 512, 0);

Keypad pad;
Joystick joystick1(pin.joy1X, pin.joy1Y, pin.joy1SW);
Joystick joystick2(pin.joy2X, pin.joy2Y, pin.joy2SW);

void updatePacket(void);
void updateLcd(void);

int main(void) {
    Kernel_Begin();
}

void create(void) {
    // Create tasks
    Task_Create_Period(updatePacket, 0, UPDATE_PACKET_PERIOD, UPDATE_PACKET_WCET, UPDATE_PACKET_DELAY);
    Task_Create_Period(updateLcd,    0, UPDATE_LCD_PERIOD,    UPDATE_LCD_WCET,    UPDATE_LCD_DELAY);

    return;
}

void updatePacket() {
    for (;;) {
        packet.field.joy1X  = joystick1.getX();
        packet.field.joy1Y  = joystick1.getY();
        packet.field.joy1SW = joystick1.getClick() == HIGH ? 0xFF : 0x00;
        packet.field.joy2X  = joystick2.getX();
        packet.field.joy2Y  = joystick2.getY();
        packet.field.joy2SW = joystick2.getClick() == HIGH ? 0xFF : 0x00;

        Task_Next();
    }
}

void updateLcd(void) {
    char buf[16];

    pad.clear();
    for (;;) {
        sprintf(buf, "%c:%4d %c:%4d %c" , XBAR_CHAR, joystick1.rawX, YBAR_CHAR, joystick1.rawY, joystick1.rawSW ? SWON_CHAR : SWOFF_CHAR);
        pad.print(Keypad::LCD_ROW::TOP, buf);

        sprintf(buf, "%c:%4d %c:%4d %c" , XBAR_CHAR, joystick2.rawX, YBAR_CHAR, joystick2.rawY, joystick2.rawSW ? SWON_CHAR : SWOFF_CHAR);
        pad.print(Keypad::LCD_ROW::BOTTOM, buf);

        Task_Next();
    }
}
