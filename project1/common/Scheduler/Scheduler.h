/*
 * scheduler.h
 *
 *  Created on: 17-Feb-2011
 *      Author: nrqm
 */

#include <inttypes.h>

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

// The delays and periods for all tasks the scheduler will run

// Base Station

#define LSENSOR_DELAY  0
#define LSENSOR_PERIOD 50

#define UPDATE_CHAR_DELAY 10
#define UPDATE_CHAR_PERIOD 50

#define UPDATE_PACKET_DELAY 0
#define UPDATE_PACKET_PERIOD 50

#define SEND_PACKET_DELAY 2
#define SEND_PACKET_PERIOD 50

#define UPDATE_LCD_DELAY 4
#define UPDATE_LCD_PERIOD 500

// Remote Station

#define GET_DATA_DELAY 0
#define GET_DATA_PERIOD 50

#define UPDATE_ARM_DELAY 2
#define UPDATE_ARM_PERIOD 50

#define COMMAND_ROOMBA_DELAY 4
#define COMMAND_ROOMBA_PERIOD 100

///Up to this many tasks can be run, in addition to the idle task
#define MAXTASKS	8

///A task callback function
typedef void (*task_cb)();

/**
 * Initialise the scheduler.  This should be called once in the setup routine.
 */
void Scheduler_Init();

/**
 * Start a task.
 * The function "task" will be called roughly every "period" milliseconds starting after "delay" milliseconds.
 * The scheduler does not guarantee that the task will run as soon as it can.  Tasks are executed until completion.
 * If a task misses its scheduled execution time then it simply executes as soon as possible.  Don't pass stupid
 * values (e.g. negatives) to the parameters.
 *
 * \param id The tasks ID number.  This must be between 0 and MAXTASKS (it is used as an array index).
 * \param delay The task will start after this many milliseconds.
 * \param period The task will repeat every "period" milliseconds.
 * \param task The callback function that the scheduler is to call.
 */
void Scheduler_StartTask(int16_t delay, int16_t period, task_cb task);

/**
 * Go through the task list and run any tasks that need to be run.  The main function should simply be this
 * function called as often as possible, plus any low-priority code that you want to run sporadically.
 */
uint32_t Scheduler_Dispatch();

#endif /* SCHEDULER_H_ */
