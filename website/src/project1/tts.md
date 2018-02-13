## Time Triggered Scheduler

A time triggered scheduler (TTS) executes tasks periodically at a very precise period. A TTS is less complicated than a real time operating system (RTOS) as tasks always run until completion and cannot be pre-empted. That is, there is no context switching between tasks. This design is much simpler than an RTOS as we do not need to worry about race-conditions. Shared memory and global variables can be used to effectively to share data between tasks.

The TTS used for this project was provided for us by [Neil's Log Book](http://nrqm.ca/mechatronics-lab-guide/lab-guide-time-triggered-scheduling/). Only small modifications were made to make use C `inttypes`. The header of the `Scheduler.h` file is

```c
#include <inttypes.h>

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

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
```

### Base Station

The base station consists of three tasks that need to be run at a pre-determined rate. These are

- updating the WiFi packet,
- sending the WiFi packet, and
- updating the LCD

We first determined how long each of these tasks take to run. These values are shown in the following table.

| Task             | Average Execution Time (ms) |
| :--------------- |   ------------------------: |
| Update packet    |                          12 |
| Send packet      |                        0.05 |
| Update LCD       |                        0.65 |

The logic analyzer capture from which these values were taken from is shown in the following figure.

![Base Station Logic Analyzer Capture][base logic]

[base logic]: https://i.imgur.com/u3OYzbf.png "Base Station Logic Analyzer Capture"

The execution times of these tasks must be taken into account when using the TTS to schedule tasks to prevent any task overlap.

The lowest sampling frequency for each of the above tasks was estimated in order to minimize CPU utilization time and at the same time ensure the system as a whole performs _smoothly_. The following table shows the delay and frequency/period values we used for each task.

| Task          | Delay (ms) | Frequency (Hz) | Period (ms) |
| :--           |       ---: |           ---: |        ---: |
| Update packet |          0 |             20 |          50 |
| Send packet   |          2 |             20 |          50 |
| Update LCD    |          4 |              2 |         500 |

In order to prevent the tasks from ever overlapping, we chose periods with the same common divisor. Updating the packet will always happen 2 ms before sending the packet. This ensure that no redundant information is sent over WiFi. The LCD display takes a significantly long time to update, which is why we only update it twice a second.

The code we used to schedule tasks for the base station is the following

```c
#define UPDATE_PACKET_DELAY 0
#define UPDATE_PACKET_PERIOD 50

#define SEND_PACKET_DELAY 2
#define SEND_PACKET_PERIOD 50

#define UPDATE_LCD_DELAY 4
#define UPDATE_LCD_PERIOD 500

// ...

Scheduler_Init();
Scheduler_StartTask(UPDATE_PACKET_DELAY, UPDATE_PACKET_PERIOD, updatePacket);
Scheduler_StartTask(SEND_PACKET_DELAY,   SEND_PACKET_PERIOD,   sendPacket);
Scheduler_StartTask(UPDATE_LCD_DELAY,    UPDATE_LCD_PERIOD,    updateLcd);
```

### Remote Station

The remote station consists of three tasks that need to be run at pre-determined rates. These are

- getting data via WiFi,
- updating the arm and laser, and
- commanding the Roomba

The task execution was determined using a logic analyzer.

| Task                 | Average Execution Time (ms) |
| :---                 |                        ---: |
| Getting data         |                        0.08 |
| Update arm and laser |                        0.25 |
| Command Roomba       |                        0.15 |

The logic analyzer capture from which these values were taken from is shown in the following figure.

![Remote Station Logic Analyzer Capture][remote logic]

[remote logic]: https://i.imgur.com/I1k5rgW.png "Remote Station Logic Analyzer Capture"

The lowest sampling frequency for each of the above tasks was estimated in order to minimize CPU utilization time and at the same time ensure the system as a whole performs _smoothly_. The following table shows the delay and frequency/period values we used for each task.

| Task                 | Delay (ms) | Frequency (Hz) | Period (ms) |
| :---                 |       ---: |           ---: |        ---: |
| Getting data         |          0 |             20 |          50 |
| Update arm and laser |          2 |             20 |          50 |
| Command Roomba       |          4 |             10 |         100 |

Getting the data must happen before we update the arm and laser. Since we get the data at the same period as updating the arm and laser, we know there will never be redundant updates.

Sending data from the base station happens at the same period as checking for data on the remote station. This ensures that the buffer on the remote stations UART will never overflow, a problem we were encountering before the periods were the identical.

The code used to schedule tasks for the remote station is the following

```c
#define GET_DATA_DELAY 0
#define GET_DATA_PERIOD 50

#define UPDATE_ARM_DELAY 2
#define UPDATE_ARM_PERIOD 50

#define COMMAND_ROOMBA_DELAY 4
#define COMMAND_ROOMBA_PERIOD 100

// ...

Scheduler_Init();
Scheduler_StartTask(UPDATE_ARM_DELAY,     UPDATE_ARM_PERIOD,     updateArm);
Scheduler_StartTask(COMMAND_ROOMBA_DELAY, COMMAND_ROOMBA_PERIOD, commandRoomba);
Scheduler_StartTask(GET_DATA_DELAY,       GET_DATA_PERIOD,       getData);
```
