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

void Scheduler_Init();

void Scheduler_StartTask(int16_t delay, int16_t period, task_cb task);

uint32_t Scheduler_Dispatch();

#endif /* SCHEDULER_H_ */
```

The scheduler is used by first calling the `Scheduler_Init()` function. Next, tasks which need to be run periodically are added to the scheduler with the `Scheduler_StartTask()` function. The parameters provided are

- `delay`: Initial delay in ms that the task will be run after
- `period`: How often in ms that the task will be repeat after
- `task`: Function callback that represents the task and will be run every period

Each iteration of the main loop the `Scheduler_Dispatch()` function is called. This function will figure out which task needs to be run based on the current running time. The amount of idle time remaining between the function call and the next task that needs to be run is returned as a `uint32_t`. This `idle_period` can be used to do some low priority task that does not take a lot of time.

### Base Station

The base station consists of three tasks that need to be run at a pre-determined rate. These are

- updating the WiFi packet,
- sending the WiFi packet, and
- updating the LCD

We first determined how long each of these tasks take to run. These values are shown in the following table.

| Task             | Average Execution Time (ms) |
| :--------------- |   ------------------------: |
| Update packet    |                        0.65 |
| Send packet      |                        0.05 |
| Update LCD       |                          12 |

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
