#include <stdint.h>

#include <Servo.h> // ARDMK doesn't recognize that Arm requires this lib
#include "Arm.h"
#include "Scheduler.h"
#include "Packet.h"
#include "Roomba.h"

#define CLEAR_TERM "\x1B[2J\x1B[H"

#define LOGIC_ARM 31
#define LOGIC_DATA 45
#define LOGIC_ROOMBA 51

typedef struct {
    const uint8_t servoX;
    const uint8_t servoY;
    const uint8_t laser;
    const uint8_t idle;
} PinDefs;

PinDefs pin = {
    .servoX = 11,
    .servoY = 12,
    .laser  = 40,
    .idle   = 34
};

Arm arm;
Packet packet(512, 512, 0, 512, 512, 0);

Roomba r(3, 30);
char last_command = 's';

void getData();
void updateArm();
void commandRoomba();
void printPacket();

void setup() {
    Serial.begin(38400);
    Serial2.begin(9600);
    Serial3.begin(9600);

    arm.attach(pin.servoX, pin.servoY);

    r.init();
    delay(1000);
    /* r.control(); */

    pinMode(LOGIC_ARM, OUTPUT);
    pinMode(LOGIC_DATA, OUTPUT);
    pinMode(LOGIC_ROOMBA, OUTPUT);

    pinMode(pin.laser, OUTPUT);
    pinMode(pin.idle, OUTPUT);

    Scheduler_Init();
    Scheduler_StartTask(UPDATE_ARM_DELAY,     UPDATE_ARM_PERIOD,     updateArm);
    Scheduler_StartTask(COMMAND_ROOMBA_DELAY, COMMAND_ROOMBA_PERIOD, commandRoomba);
    Scheduler_StartTask(GET_DATA_DELAY,       GET_DATA_PERIOD,       getData);
}

// idle task
void idle(uint32_t idle_period) {
    // this function can perform some low-priority task while the scheduler has nothing to run.
    // It should return before the idle period (measured in ms) has expired.  For example, it
    // could sleep or respond to I/O.

    // example idle function that just pulses a pin.
    digitalWrite(pin.idle, HIGH);
    delay(idle_period);
    digitalWrite(pin.idle, LOW);
}

void loop() {
    uint32_t idle_period = Scheduler_Dispatch();
    if (idle_period) {
        idle(idle_period);
    }
}

void getData() {
    digitalWrite(LOGIC_DATA, HIGH);

    int bytesAvailable = Serial2.available();

    if (bytesAvailable > 0) {
        uint8_t r = Serial2.read();

        // If the first byte isn't magic, don't attempt to read in a packet
        if (r == PACKET_MAGIC && bytesAvailable >= PACKET_SIZE + 1) {
            uint8_t packet_buf[PACKET_SIZE];

            // Read in a packet, and check its size
            if (Serial2.readBytes(packet_buf, PACKET_SIZE) == PACKET_SIZE) {
                packet = Packet(packet_buf);

                // Read and discard anything else available
                for (int i=0; i < bytesAvailable - 1 - PACKET_SIZE; i += 1) {
                    Serial.read();
                }
            }
        }
    }
    digitalWrite(LOGIC_DATA, LOW);
}

void commandRoomba() {
    digitalWrite(LOGIC_ROOMBA, HIGH);

    // Map down to reduce jitter
    int8_t x = map(packet.field.joy2X, 0, 1023, -5, 5);
    int8_t y = map(packet.field.joy2Y, 0, 1023, -5, 5);

    char command = 's'; // Stop

    // Threshold for movement
    if (abs(x) > 2 || abs(y) > 2) {

        // which is biggest?
        if (abs(x) > abs(y)) {
            // x is: right or left?
            command = x > 0 ? 'r' : 'l';
        } else {
            // y is: forward or back?
            command = y > 0 ? 'f' : 'b';
        }
    }

    // overwrite any command if we should be docking
    command = packet.field.joy2SW == HIGH ? 'd' : command;

    /* unsigned int power = 0; */
    /* bool success = r.check_power(&power); */
    /* Serial.print("Power: "); */
    /* Serial.print(power); */
    /* Serial.println(command); */

    if (command == last_command) {
        digitalWrite(LOGIC_ROOMBA, LOW);
        return;
    }
    last_command = command;

    switch(command) {
        case 'f':
            r.drive(150, 32768);
            break;
        case 'b':
            r.drive(-150, 32768);
            break;
        case 'r':
            r.drive(50, -1);
            break;
        case 'l':
            r.drive(50, 1);
            break;
        case 's':
            r.drive(0,0);
            break;
        case 'd':
            /* r.dock(); */
            r.control();
            r.sing();
            break;
        case 'p':
            r.power_off();
            break;
        default:
            break;
    }

    /* Serial.println(command); */
    /* Serial3.write(command); */

    digitalWrite(LOGIC_ROOMBA, LOW);
}

void updateArm() {
    digitalWrite(LOGIC_ARM, HIGH);

    arm.setSpeedX(Arm::filterSpeed(packet.field.joy1X));
    arm.setSpeedY(Arm::filterSpeed(packet.field.joy1Y));

    digitalWrite(pin.laser, packet.field.joy1SW == 0xFF ? HIGH : LOW);

    arm.tick();

    digitalWrite(LOGIC_ARM, LOW);
}

void printPacket() {
    Serial.print(CLEAR_TERM);
    Serial.print("packet.field.joy1X  : "); Serial.println(packet.field.joy1X);
    Serial.print("packet.field.joy1Y  : "); Serial.println(packet.field.joy1Y);
    Serial.print("packet.field.joy1SW : "); Serial.println(packet.field.joy1SW);
    Serial.print("packet.field.joy2X  : "); Serial.println(packet.field.joy2X);
    Serial.print("packet.field.joy2Y  : "); Serial.println(packet.field.joy2Y);
    Serial.print("packet.field.joy2SW : "); Serial.println(packet.field.joy2SW);

    Serial.print("packet.data         : ");
    for (int i = 0; i < 10; i ++) {
        Serial.print(packet.data[i], HEX);
        Serial.print(":");
    }
    Serial.println(" ");
}
