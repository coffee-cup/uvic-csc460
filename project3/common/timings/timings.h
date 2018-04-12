#ifndef _TIMINGS_H_
#define _TIMINGS_H_

#include "common.h"

// Base Station

#define UPDATE_PACKET_PERIOD 5
#define UPDATE_PACKET_WCET 1
#define UPDATE_PACKET_DELAY 0

#define SEND_PACKET_PERIOD 100
#define SEND_PACKET_WCET 2
#define SEND_PACKET_DELAY 1

#define UPDATE_LCD_PERIOD 50
#define UPDATE_LCD_WCET 4
#define UPDATE_LCD_DELAY 2

// Remote Station

#define GET_DATA_PERIOD 1000
#define GET_DATA_WCET 2
#define GET_DATA_DELAY 10

#define UPDATE_ARM_PERIOD 50
#define UPDATE_ARM_WCET 1
#define UPDATE_ARM_DELAY 2

#define COMMAND_ROOMBA_PERIOD 500
#define COMMAND_ROOMBA_WCET 4
#define COMMAND_ROOMBA_DELAY 2

#define ARM_TICK_PERIOD 2
#define ARM_TICK_WCET 1
#define ARM_TICK_DELAY 2

#define MODE_PERIOD (60000 / MSECPERTICK) // 60 seconds
#define MODE_WCET 1
#define MODE_DELAY 0

#endif
