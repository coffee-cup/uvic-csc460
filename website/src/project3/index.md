# Introduction

Accumulating what we've learned this term, this project's goal is to apply our real time operating system to a physical task. The real time operating system has been previously developed during project 2 - see the report [here](/project2). As stated in the project specifications, the task would be to compete in a "Defend Your Castle Game".

> #### Defend Your Castle Game
> The final project theme for this term is to ultimately create a [Defend Your Castle game](https://en.wikipedia.org/wiki/Defend_Your_Castle), defending and shooting, among a group of **semi-autonomous mobile robots**.
>
> Each robot is equipped with a laser mounted on a pan-and-tilt mechanism, a light-sensor and a shield (some material that can block the laser). A group of two robots is played against another group. Each group occupies its own half of the playing field. Inside the playing field, each group has a castle - essentially a light-sensor - to defend. There is a "river" separating the two playing fields (created by two virtual walls), so each robot can only shoot the laser across the field but may not cross to the other side. If a robot crosses the "river", it is killed immediately!
>
> A group wins if it can "destroy" (shoot down) the other group's castle, or "kills" (shoot down) the other group's robots. Since each robot is equipped with a shield, it can defend itself or its castle. A robot's movement is required to switch from "cruise" mode to "stand still" mode every minute. That is, it is allowed to move freely within its playing field for one minute, and then it must remain stationary, except that it is allowed to spin, for another minute, and so on. The laser has only enough "energy" to turn on for 30 seconds in total; that is, it cannot be turned on all the time. When a robot runs out of energy, then it can only use its shield but it can't shoot.


Our approach to implementing this project would be similar to how the ability to wirelessly control the robots in [project 1](/project1) was accomplished. The differences are that our own real-time operating system will replace the time triggered scheduler, and furthermore any Arduino libraries and code are forbidden.
