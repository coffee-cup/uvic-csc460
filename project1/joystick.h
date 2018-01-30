#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

typedef struct {
  int pinX;
  int pinY;
  int pinClick;
  int boundMin;
  int boundMax;
} Joy;

Joy init_Joy(int pinX, int pinY, int pinClick, int boundMin, int boundMax);

int getX(Joy stick);
int getY(Joy stick);
int getClick(Joy stick);

#endif
