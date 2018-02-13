# Phase 2

Phase 2 consisted of using two ATMega2560 boards to control a Roomba robot and a mounted pan-and-tilt kit and laser. The purpose of this phase is to familiarize ourselves with sending commands between boards and with the importance of timing in low level embedded systems.

## System Overview

One board was installed on the Roomba as the _remote_ station and the other was used as a _base_ station. Connected to the base station were two joysticks and an LCD screen. The positions of the joysticks were sent to remote station over WiFi. The WiFi modules were setup in such a way that they were automatically configured to the correct settings and ready to be used when powered on. A time-triggered-scheduler was used to control the execution of the various tasks performed on the remote and base stations.

The components used in the base station are

- two joysticks,
- an LCD display, and
- a WiFi module

The base station setup is shown in the following figure.

![Base Station][base]

The components used in the remote station are

- a laser,
- pan-and-tilt kit servos,
- a Roomba (not shown), and
- a Wifi module

The remote station setup is shown in the following figure.

![Remote Station][remote]

A demonstration of the our phase 2 can be seen in the below Youtube video

<iframe width="720" height="360" src="https://www.youtube.com/embed/I5R19W6xEE0" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>

[base]: https://i.imgur.com/naujmlM.png "Base Station"

[remote]: https://i.imgur.com/8K3AXsR.png "Remote Station"
