# Phase 1

The first phase of the project consisted of connecting the joysticks, servos, laser pointer, lcd screen, and analog light sensor to a single ATMega2560 board. The purpose of this phase was to familiarize ourselves with the hardware and the software required to run control each component.

## System Overview

For this phase we setup a two servo motors mounted inside a pan-and-tilt kit. The two motors were controlled using both axes of a joystick. The push-button of the joystick controlled a laser mounted on top of the pan-and-tilt kit servos. On a separate component, we wired up a light sensor that would detect the presence of a light source hitting it. Finally, we connected an LCD display which showed the coordinates of the joystick and the status of the light sensor.

A schematic of how our phase 1 is connected is shown in the following figure.

![Phase 1 Schematic][phase1_schematic]


[phase1_schematic]: https://i.imgur.com/rY9LenJ.png "Phase 1 Schematic"

In the schematic, blue wires represent a signal connection between a hardware component and the ATMega2560. Red wires are connections between power and a component and black wires are ground.
