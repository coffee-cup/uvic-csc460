#ifndef ARM_H
#define ARM_H

/* #include <Arduino.h> */
#include <Servo.h>

#define S_MIN 1
#define S_MAX 179
#define S_MAX_SPEED 70 // deg / sec

typedef struct {
  int pinX;
  int pinY;
  int posX;
  int posY;
  int speedX;
  int speedY;
  unsigned long lastRunX;
  unsigned long lastRunY;
  Servo servoX;
  Servo servoY;
} Arm;

Arm init_Arm(int pinX, int pinY);
void setSpeedX(Arm *arm, int s);
void setSpeedY(Arm *arm, int s);
void tick(Arm *arm);

#endif
