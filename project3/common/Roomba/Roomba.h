#ifndef _ROOMBA_H_
#define _ROOMBA_H_

#include <stdint.h>

#define STRAIGHT 32768

class Roomba {
  public:
    Roomba(uint8_t serial_connector, uint8_t brc_pin);

    bool init();
    void drive(int16_t velocity, int16_t radius);
    void stop();
    void dock();

    bool check_oi_mode(uint16_t* mode);
    bool check_power(uint16_t* power);
    bool check_power_capacity(uint16_t* power);
    bool check_light_bumper(uint16_t* data);
    void power_off();

    bool check_virtual_wall();
    bool check_left_bumper();
    bool check_right_bumper();

    void set_song(uint8_t song_number, uint8_t song_length, uint8_t *song);
    void play_song(uint8_t song_number);

  private:
    uint8_t uart_channel;
    uint8_t baud_change_pin;

    //This is not an exhaustive list of open interface commands.
    enum OI_COMMAND {
        RESET = 7U,       // reset the Roomba as if the battery was removed and replaced
        START_SCI = 128U, // start the Roomba's serial command interface
        BAUD = 129U,      // set the SCI's baudrate (default on full power cycle is 115200)
        SAFE = 131U,      // enter safe mode (aka. control)
        FULL = 132U,      // enter full mode
        POWER = 133U,     // put the Roomba to sleep
        SPOT = 134U,      // start spot cleaning cycle
        CLEAN = 135U,     // start normal cleaning cycle
        MAX = 136U,       // start maximum time cleaning cycle
        DRIVE = 137U,     // control wheels
        MOTORS = 138U,    // turn cleaning motors on or off
        LEDS = 139U,      // activate LEDs
        SONG = 140U,      // load a song into memory
        PLAY = 141U,      // play a song that was loaded using SONG
        SENSORS = 142U,   // retrieve one of the sensor packets
        DOCK = 143U,      // force the Roomba to seek its dock.
        STOP_SCI = 173U,  // stop the Roomba's SCI

    };

    // Nor is this an exhaustive list of arguments to the SENSOR command
    enum OI_SENSOR_ARGS {
        BUMPERS = 7U,
        VIRTUAL_WALL = 13U,
        BATTERY_CHARGE = 25U,
        BATTERY_CAPACITY = 26U,
        OIMODE = 35U,
        LIGHT_BUMPER = 45U,
        LIGHT_BUMP_LEFT = 46U
    };

    /// Arguments to the BAUD OI_COMMAND
    enum OI_BAUD_ARGS
    {
        BPS_300 = 0,
        BPS_600 = 1,
        BPS_1200 = 2,
        BPS_2400 = 3,
        BPS_4800 = 4,
        BPS_9600 = 5,
        BPS_14400 = 6,
        BPS_19200 = 7,
        BPS_28800 = 8,
        BPS_38400 = 9,
        BPS_57600 = 10,
        BPS_115200 = 11,
    };

    void start_serial(uint32_t baud);
    void issue_cmd(uint8_t val);
    bool try_read(uint8_t *val);
    void flush_data();
    bool check_sensor(OI_SENSOR_ARGS sensor, uint8_t nbytes, uint16_t* data);

  public:
    enum OI_PLAY_ARGS {
        SONG_ONE = 0,
        SONG_TWO = 1,
        SONG_THREE = 2,
        SONG_FOUR = 3,
        SONG_FIVE = 4
    };

    enum OI_LED_MASK_ARGS {
        DEBRIS_LED = 0x1,
        SPOT_CLEAN_LED = 0x2,
        DOCK_LED = 0x4,
        CHECK_ROBOT_LED = 0x8
    };

    enum OI_MODE_TYPE {
        PASSIVE_MODE = 1,
        SAFE_MODE = 2,
        FULL_MODE = 3
    };

    void set_mode(OI_MODE_TYPE mode);
    void sing(OI_PLAY_ARGS);
    void leds(OI_LED_MASK_ARGS leds, uint8_t power_led_colour, uint8_t power_led_intensity);

};

#endif
