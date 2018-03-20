# Introduction

In this project, we built a real-time operating system (RTOS) for the AVR ATmega2560 microcontroller. An operating system is the foundational system software upon which many applications are built - it is responsible for providing an interface which facilitates some common services for other computer programs to leverage. This interface is typically responsible for providing features such as process management such as creation and termination, scheduling including multi-tasking and synchronization, memory management, interprocess communication, etc. Traditionally, an operating system is divided into two main components: the user level, which runs generic - custom - applications; and the kernel level, responsible for the most basic level control over the system's hardware devices.

A *real-time* operating system (RTOS) is a particular style of operating system which is intended to serve real-time applications by providing a guarantee that events or data is processed by a specific moment in time. That is, an RTOS facilitates construction of a system where the correct behaviour is dependant on the timing of specific computations, along with the results of such computations.

Non-real-time operating systems are not consistent when concerning the amount of time it takes to accept and complete an application's computation (task), nor do they provide mechanism for scheduling a task at regular time intervals. These types of operating systems usually operate by means of an event-driven preemptive priority heiarchy. A real-time operating system can be constructed using a time-sharing or event-driven scheme, or a combination of the two. An event-driven system switches between tasks based on their priorities while time-sharing systems switch between tasks at timed, regular intervals.

In order to decide which tasks to run when, a real-time operating system employes a scheduler. It is the scheduler's responsibility to observe the states of all known tasks and when invoked, select one which should be run at that time. As the scheduler is part of the kernel a user level task should not require any knowledge of the scheduler; from it's perspective it has exclusive access to the hardware resources. This is precisely the goal of the Operating System, to manage a set of **logically independant** but **physically shared** hardware resources.

# Objective

The objective of this project is to construct a real-time operating system with the interface as defined in the provided [OS.h](https://webhome.csc.uvic.ca/~mcheng/460/spring.2018/os.h) header file. This RTOS will be capable of executing tasks at 3 differing levels of priority: System tasks, Periodic tasks, and Round-Robin tasks, listed in respective order of decreasing priority. Tasks will be able to communicate using the QNX-style [SEND-RECEIVE-REPLY](http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_sys_arch%2Fipc.html) interprocess communications (IPC) technique.

As explained in the introduction, a task should execute without any knowledge of that it may be suspended or resumed at any time by the kernel. To do this, the context of the task at any point in time must be able to be saved and restored. When a task is suspended, it's context is recorded such that it can be restored later without any immediately noticable change in the task's state.


