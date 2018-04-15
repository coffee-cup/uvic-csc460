# Changes to the Real Time Operating System

Users of our RTOS can create Tasks to complete their computations, and Tasks have 3 levels of priority, System, Periodic, and Round Robin. System tasks preempt all other tasks, and run until completion or yield. Periodic tasks are designed to be executed on a given period, they must yield before a given worst-case execution time, and are able to preempt Round Robin tasks but not System tasks. Round Robin tasks are the lowest priority, they are executed only when no other task is executing, and run until preemption, termination, or yield.

Briefly, real time operating systems (RTOS) are a type of operating system, specifically those which are intended to serve real-time applications by providing a guarantee that events or data are processed at a specific moment in *time*. That is, an RTOS facilitates construction of a system where the correct behaviour is dependant on both the timing of specific computations, and the results of such computations. More information on real time operating systems, their design, and our implementation is available in the [project 2 report](/project2). For project 3 we used our RTOS, although some changes had to be made, detailed below.


## Shared libraries

The same operating system was required to be used on both the "base" (joysticks, sending data) and "remote" (roomba, receiving data) stations. To help encapsulate development of the various components that would need be developed for this project, we switched to developing new code in C++. This means that earlier code would have to be properly imported using the C++ C-import syntax:
```c++
#include <cpp_code.h>

extern "C" {
    #include <c_code.h>
    ...
}
```

Where the project directory for project 2 was mostly flat, we needed a way to include the operating system in multiple projects. For this we renamed the `libs` directory to `common`
and created two sub-projects in the project 3 directory. This way the individual "base" and "remote" stations could include the same code and could be independently built using the associated Makefile.

+-------------------------------|--------------------------------+
|  Project 2                    |  Project 3                     |
+===============================+================================+
|<div class ="tree-dirs">       |<div class="tree-dirs">         |
|     project2                  |     project3                   |
|     ├── libs (mv → common)    |     ├── common                 |
|     │   ├── tests             |     │   ├── os                 |
|     │   │   └─ ...            |     │   │   └─ ...             |
|     │   ├── trace             |     │   ├── Packet             |
|     │   │   └─ ...            |     │   │   └─ ...             |
|     │   └── uart              |     │   ├── Roomba             |
|     │       └─ ...            |     │   │   └─ ...             |
|     ├── common.h              |     │   ├── ...                |
|     ├── cswitch.S             |     │   ├── tests              |
|     ├── kernel.c / h          |     │   │   └─ ...             |
|     ├── Makefile              |     │   ├── trace              |
|     ├── message.c / h         |     │   │   └ ...              |
|     ├── os.c / h              |     │   ├── uart               |
|     ├── process.c / h         |     │   │   └ ...              |
|     └── user.c                |     │   └── utils              |
|                               |     │       └ ...              |
|                               |     ├── base                   |
|                               |     │   ├── Makefile           |
|                               |     │   └── user.cpp           |
|                               |     └── remote                 |
|                               |         ├── Makefile           |
|                               |         └── user.cpp           |
|</div>                         |</div>                          |
+-------------------------------|--------------------------------+


### Base Station

The base station was designed to be the simpler of the two component operating systems. It uses only two periodic tasks, one to read control input data from joysticks, and another to broadcast the input data as datagrams over UDP.

In entirety, the base station is controlled by a file of under 100 lines of code. The main functionality is shown below. [View the whole file.](https://github.com/coffee-cup/uvic-csc460/blob/master/project3/base/user.cpp)

```c++
void updatePacket(void) {
    TASK({
        // TX data only - discard any incoming data
        if (UART_Available(data_channel)) {
            UART_Flush(data_channel);
        }

        // Read joystick values and update packet
        packet.joy1X(joystick1.getX());
        packet.joy1Y(joystick1.getY());
        packet.joy1SW(joystick1.getClick() ? 0xFF : 0x00);
        packet.joy2X(joystick2.getX());
        packet.joy2Y(joystick2.getY());
        packet.joy2SW(joystick2.getClick() ? 0xFF : 0x00);
    })
}

void TXData(void) {
    TASK({
        // UART TX is ready to write
        if (UART_Writable(data_channel)) {
            UART_send_raw_bytes(data_channel, PACKET_SIZE, packet.data);
        }
    })
}

void create(void) {
    data_channel = 2;
    UART_Init(data_channel, 38400);

    // Create tasks
    Task_Create_Period(updatePacket, 0,
        UPDATE_PACKET_PERIOD,
        UPDATE_PACKET_WCET,
        UPDATE_PACKET_DELAY
    );

    Task_Create_Period(TXData,       0,
        SEND_PACKET_PERIOD,
        SEND_PACKET_WCET,
        SEND_PACKET_DELAY
    );

    return;
}
```

### Remote Station

Following the design pattern of a simple base station, the heart of the computation needed to happen on the remote station end. The remote station is responsible for performing at least the following tasks:

 - Communicating with Roomba
 - Listening for UDP packets (abstracted to listening to UART by the ESP8266 WiFi chip).
 - Decoding and verifying packets
 - Translating packet values into commands
 - Managing game state
 - Monitoring sensors
 - Controlling Servos

As such, the file containing the control code for the remote station RTOS is significantly longer. The remote station uses a combination of Periodic tasks and Round Robin tasks to complete its requirements. During setup the control code initializes 4 periodic tasks for receiving data, reading light sensor data, updating pan-and-tilt kit state, and moving the pan-and-tilt kit in accordance to requested state. A Round Robin task is created immediately as well to initialize the Roomba - this task is long running, and when complete it creates another (5th) Periodic task for issuing movement commands to the Roomba. [View the whole remote station control code file here.](https://github.com/coffee-cup/uvic-csc460/blob/master/project3/remote/user.cpp)

## Periodic task queuing fairness

One of the bugs in the operating system that went undiscovered in testing during project 2 was to do with dispatching between multiple Periodic tasks. Consider two periodic tasks, one with high frequency (low period) but low worst-case execution time and the second with high period and high worst-case execution. That is, the first task runs often but does not take long, and the second task takes a long time, but runs infrequently.

The problem encountered with this scenario was that the kernel would not dispatch from one periodic task to another when clock ticks occurred. This would cause the high frequency task to be delayed until after the long running task completed. Often this means that the high frequency task has missed it's start time, causing a timing violation. The fix is to treat all tasks fairly, regardless of priority. If multiple periodic tasks are ready to run, or are running at any point in time, then they should get equal share of the hardware.

The following logic trance screenshots illustrate this issue. The first 3 traces show tasks, the signal goes high when it begins executing and goes low when it ends its execution. The 4th trace is the system clock. In both cases the tasks have the same timings:

 * Task A::
    - Period: 5 ticks
    - Worst Case Execution Time: 3 ticks
    - Delay: 10 ticks
 * Task B::
    - Period: 5 ticks
    - Worst Case Execution Time: 3 ticks
    - Delay: 12 ticks
 * Task B::
    - Period: 10 ticks
    - Worst Case Execution Time: 9 ticks
    - Delay: 13 ticks

[working]: https://i.imgur.com/ZlD5nxu.png "Long running periodic tasks are forced to yield to other periodic tasks"
[broken]: https://i.imgur.com/vge8ZnO.png "Periodic tasks do not yield for when others should start"

The vertical lines show when Task A and B should have started for the second time.

![Periodic tasks do not yield for when others should start][broken]

![Long running periodic tasks are forced to yield to other periodic tasks][working]

One argument against this change might have been that no two periodic tasks should be running at the same time, but abiding by this restriction was difficult in practice. With this change in place however, we need to be mindful to having too many periodic tasks, as the number of periodic tasks running now affects the worst case execution time of all other periodic tasks.

## Aborting from within kernel

Some abort cases were going unnoticed since the system wide function `OS_Abort(ABORT_CODE error)` could not be successfully called from within the Kernel. As implemented, `OS_Abort` caused the system to create a `Kernel_Request` call, which causes the system to attempt to context switch into the kernel. If the caller was already the kernel, then the current process pointer would become mangled, and would cause the hardware to perform a hard reset, resulting in the system rebooting and the error disappearing. A fix for this was to have the kernel jump directly to the error handler without performing a call to `Kernel_Request`.
