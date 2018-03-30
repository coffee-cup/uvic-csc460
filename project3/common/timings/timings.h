#ifndef _TIMINGS_H_
#define _TIMINGS_H_

// Base Station

#define UPDATE_PACKET_PERIOD 5
#define UPDATE_PACKET_WCET 1
#define UPDATE_PACKET_DELAY 0

#define SEND_PACKET_PERIOD 5
#define SEND_PACKET_WCET 1
#define SEND_PACKET_DELAY 1

#define UPDATE_LCD_PERIOD 50
#define UPDATE_LCD_WCET 4
#define UPDATE_LCD_DELAY 2

// Remote Station

#define GET_DATA_PERIOD 50
#define GET_DATA_WCET 1
#define GET_DATA_DELAY 0

#define UPDATE_ARM_PERIOD 50
#define UPDATE_ARM_WCET 1
#define UPDATE_ARM_DELAY 2

#define COMMAND_ROOMBA_PERIOD 100
#define COMMAND_ROOMBA_WCET 2
#define COMMAND_ROOMBA_DELAY 4

#endif
