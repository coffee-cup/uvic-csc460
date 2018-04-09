#ifndef ROOMBA_H
#define ROOMBA_H

#include <stdint.h>

#define HIGH_BYTE(x) (x >> 8)
#define LOW_BYTE(x) (x & 0xFF)

class Roomba {
  public:
    Roomba(int serial_connector, int brc_pin);

    void init();
    void drive(int velocity, int radius);
    void dock();
    void get_data();
    bool check_power(unsigned int *power);
    bool check_power_capacity(unsigned int *power);
    void power_off();
    void control();
    void sing();

  private:
    uint16_t m_serial_num;
    uint16_t m_brc_pin;

    void start_serial(long baud);
    void write_serial(char val);
    bool read_serial(char *val);

    //This is not an exhaustive list of commands.
    enum COMMAND {
        START = 128,   // start the Roomba's serial command interface
        BAUD = 129,    // set the SCI's baudrate (default on full power cycle is 57600
        CONTROL = 130, // enable control via SCI
        SAFE = 131,    // enter safe mode
        FULL = 132,    // enter full mode
        POWER = 133,   // put the Roomba to sleep
        SPOT = 134,    // start spot cleaning cycle
        CLEAN = 135,   // start normal cleaning cycle
        MAX = 136,     // start maximum time cleaning cycle
        DRIVE = 137,   // control wheels
        MOTORS = 138,  // turn cleaning motors on or off
        LEDS = 139,    // activate LEDs
        SONG = 140,    // load a song into memory
        PLAY = 141,    // play a song that was loaded using SONG
        SENSORS = 142, // retrieve one of the sensor packets
        DOCK = 143,    // force the Roomba to seek its dock.
        STOP = 173
    };

    /// Arguments to the BAUD command
    enum BAUDRATE
    {
        300BPS = 0,
        600BPS = 1,
        1200BPS = 2,
        2400BPS = 3,
        4800BPS = 4,
        9600BPS = 5,
        14400BPS = 6,
        19200BPS = 7,
        28800BPS = 8,
        38400BPS = 9,
        57600BPS = 10,
        115200BPS = 11,
    };
};

#endif
