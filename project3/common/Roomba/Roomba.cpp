#include "Roomba.h"
#include <avr/io.h>

extern "C" {
    #include "uart.h"
    #include "util/delay.h"
    #include "../os/common.h"
};

//serial_connector determines which UART the Roomba is connected to (0, 1, etc)
//brc_pin determines where the baud rate change pin is connected.
Roomba::Roomba(uint8_t serial_connector, uint8_t brc_pin)
{
    uart_channel = serial_connector;
    baud_change_pin = brc_pin;

    // Port A assumed?!
    BIT_SET(DDRA, baud_change_pin);
}

bool Roomba::init() {

    BIT_SET(PORTA, baud_change_pin);
    _delay_ms(2000);
    // Set baud to 19200 by togling the brc pin low 3 times.
    for (uint8_t i = 6; i > 0; i -= 1) {
        _delay_ms(300);
        BIT_FLIP(PORTA, baud_change_pin);
    }

    start_serial(19200);
    _delay_ms(500);

    // Enable serial open interface and wait
    issue_cmd(OI_COMMAND::START_SCI);
    _delay_ms(200);

    // Switch to faster baud
    issue_cmd(OI_COMMAND::BAUD);
    issue_cmd(OI_BAUD_ARGS::BPS_57600);
    start_serial(57600);

    // >> Roomba Docs: You must wait 100 ms before
    //    sending commands at the new baud rate
    _delay_ms(100);

    // If baud is correct we should be in safe mode
    uint16_t mode = 0;
    uint16_t power = 0;
    uint16_t power_cap = -1;
    bool success = check_oi_mode(&mode);

    // First check couldn't get data and should be in mode 1
    if (!success || mode != 1) {
        LOG("Failed to communicate with Roomba, is it on? (Mode: %x)\n");
        success = false;
    }

    if (success) {
        // Check for battery power
        success = check_power(&power) && check_power_capacity(&power_cap);
        if (success) {
            LOG("Power: %u / %u\nDone Startup (in passive mode)\n", power, power_cap);
        } else {
            LOG("Couldn't get Roomba power\nStartup Failed!\n");
        }
    }

    return success;
}

//Checks the remaining power level.
bool Roomba::check_power(uint16_t *power) {
    return check_sensor(OI_SENSOR_ARGS::BATTERY_CHARGE, 2, power);
}

//Checks the total power capacity.
bool Roomba::check_power_capacity(uint16_t *power) {
    return check_sensor(OI_SENSOR_ARGS::BATTERY_CAPACITY, 2, power);
}

//Checks the current open interface mode
bool Roomba::check_oi_mode(uint16_t* mode){
    return check_sensor(OI_SENSOR_ARGS::OIMODE, 1, mode);
}

bool Roomba::check_light_bumper(uint16_t *data) {
    return check_sensor(OI_SENSOR_ARGS::LIGHT_BUMPER, 1, data);
}

bool Roomba::check_virtual_wall() {
    uint16_t data = 0;
    check_sensor(OI_SENSOR_ARGS::VIRTUAL_WALL, 1, &data);
    return data;
}

bool Roomba::check_left_bumper() {
    uint16_t data = 0;
    check_sensor(OI_SENSOR_ARGS::BUMPERS, 1, &data);
    return MASK_TEST_ANY(data, 0x02);
}

bool Roomba::check_right_bumper() {
    uint16_t data = 0;
    check_sensor(OI_SENSOR_ARGS::BUMPERS, 1, &data);
    return MASK_TEST_ANY(data, 0x01);
}

//Generic check for up to 2 bytes from a sensor request
bool Roomba::check_sensor(OI_SENSOR_ARGS sensor, uint8_t nbytes, uint16_t *data) {

    uint8_t databyte_high = 0;
    uint8_t databyte_low  = 0;
    bool success = false;

    flush_data();

    issue_cmd(OI_COMMAND::SENSORS);
    issue_cmd(sensor);

    _delay_ms(50);

    success = try_read(&databyte_high);

    // Got one byte
    if (success) {

        // Caller requested more than 1, max is 2 bytes so just get another
        if (nbytes > 1) {
            success = success && try_read(&databyte_low);
            if (success) {
                // Successfully got 2 bytes
                *data = ((databyte_high & 0xFF) << 8) | (databyte_low & 0xFF);
            }
        } else {
            // Only needed one byte
            *data = databyte_high & 0xFF;
        }
    }

    return success;
}

void Roomba::drive(int16_t velocity, int16_t radius) {
    issue_cmd(OI_COMMAND::DRIVE);
    issue_cmd(HIGH_BYTE(velocity));
    issue_cmd(LOW_BYTE(velocity));
    issue_cmd(HIGH_BYTE(radius));
    issue_cmd(LOW_BYTE(radius));
}

void Roomba::stop() {
    drive(0, 0);
}

void Roomba::dock() {
    issue_cmd(OI_COMMAND::DOCK);
}

void Roomba::sing(OI_PLAY_ARGS songnum) {
    issue_cmd(OI_COMMAND::PLAY);
    issue_cmd(songnum);
}

void Roomba::leds(OI_LED_MASK_ARGS leds, uint8_t power_led_colour, uint8_t power_led_intensity) {
    issue_cmd(OI_COMMAND::LEDS);
    issue_cmd(leds);
    issue_cmd(power_led_colour);
    issue_cmd(power_led_intensity);
}

void Roomba::set_mode(OI_MODE_TYPE mode) {
    switch(mode) {

        case OI_MODE_TYPE::PASSIVE_MODE:
            // Note: Use start command to return to passive mode
            issue_cmd(OI_COMMAND::START_SCI);
            break;

        case OI_MODE_TYPE::SAFE_MODE:
            issue_cmd(OI_COMMAND::SAFE);
            break;

        case OI_MODE_TYPE::FULL_MODE:
            issue_cmd(OI_COMMAND::FULL);
            break;

        default:
            LOG("Unknown command in Roomba::set_mode\n");
            break;
    }
    _delay_ms(20);
}

void Roomba::power_off() {
    LOG("Roomba is shutting down\n");
    issue_cmd(OI_COMMAND::POWER);
    issue_cmd(OI_COMMAND::STOP_SCI);
}

void Roomba::flush_data() {
    uint8_t val;
    while (try_read(&val))
        ;
}

void Roomba::start_serial(uint32_t baud) {
    UART_Init(uart_channel, baud);
}

bool Roomba::try_read(uint8_t *val) {
    return UART_Async_Receive(uart_channel, val);
}

void Roomba::issue_cmd(uint8_t val) {
    UART_Transmit(uart_channel, val);
}
