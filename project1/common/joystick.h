#ifndef JOYSTICK_H
#define JOYSTICK_H

typedef struct {
    int pinX;
    int pinY;
    int pinClick;
    int clicked;
} Joy;

Joy init_Joy(int pinX, int pinY, int pinClick);

int getX(Joy stick);
int getY(Joy stick);
int getClick(Joy stick);

#endif
