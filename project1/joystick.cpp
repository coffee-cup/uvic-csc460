#include <Arduino.h>
#include "joystick.h"

Joy init_Joy(int pinX, int pinY, int pinClick, int boundMin, int boundMax) {
  Joy stick = (Joy){ pinX, pinY, pinClick, boundMin, boundMax, 0 };
  pinMode(pinClick, INPUT_PULLUP);
  return stick;
}

int normalize(Joy stick, int v) {
  v = map(v, 20, 1010, stick.boundMin, stick.boundMax);
  v = constrain(v, stick.boundMin, stick.boundMax);

  // deadband
  if (abs(v) <= 5) {
    v = 0;
  }
  return v;
}

int getX(Joy stick) {
  int value = analogRead(stick.pinX);
  return normalize(stick, value);
}

int getY(Joy stick) {
  int value = analogRead(stick.pinY);
  return normalize(stick, value);
}

int getClick(Joy stick) {
  return !digitalRead(stick.pinClick);
}
