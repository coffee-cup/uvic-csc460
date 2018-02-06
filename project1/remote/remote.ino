
#include <stdint.h>
#include <Servo.h>
#include "../common/arm.h"
#include "../common/Scheduler.h"
#include "../common/packet.h"

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
    .idle   = 50
};

Arm arm;
Packet p;

void getData();
void updateArm();

void setup() {
    arm = init_Arm(pin.servoX, pin.servoY);

    pinMode(pin.laser, OUTPUT);

    Scheduler_Init();
    Scheduler_StartTask(0, 50, updateArm);
    Scheduler_StartTask(15, 10, getData);

    Serial.begin(38400);
    Serial2.begin(9600);
}

// idle task
void idle(uint32_t idle_period)
{
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
	if (idle_period)
	{
		idle(idle_period);
	}
}

void getData() {
    if (5 <= Serial2.available()){
        uint8_t buffer[5];

        if (Serial2.readBytes(buffer, 5)){
            p = create_packet(buffer);

            Serial.print("-- SpeedX: ");
            Serial.print(p.speedX, DEC);

            Serial.print(" SpeedY: ");
            Serial.print(p.speedY, DEC);
            Serial.print("\n\r");
        }
    }
}

int16_t filterSpeed(int16_t v) {
    v = map(v, 20, 1010, -1 * S_MAX_SPEED, S_MAX_SPEED);
    v = constrain(v, -1 * S_MAX_SPEED, S_MAX_SPEED);

    // deadband
    if (abs(v) <= 5) {
        v = 0;
    }
    return v;
}

void updateArm() {

    setSpeedX(&arm, filterSpeed(p.speedX));
    setSpeedY(&arm, filterSpeed(p.speedY));

    digitalWrite(pin.laser, p.laserOn ? HIGH : LOW);

    tick(&arm);
}
