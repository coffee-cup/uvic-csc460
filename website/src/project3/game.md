# Capture the Tower Game

The end goal of project 3 was to build a Roomba that could play a _Capture the Tower Game_. The game involves two teams of two positioned on opposite sides of the room. Each time has a _castle_ to defend. If your castle is shot with a laser for more than 5 seconds then your team loses. Each Roomba robot also has a light sensor mounted inside a large cup (to increase the target-able area). If the light sensor is shot then the robot is instantly dead and can no longer move or shoot its laser. Each Roomba also is equipped with a shield, a piece of styrofoam or cardboard, that can be used to block the laser. In the middle of the game floor is a _river_. The river is implemented as two virtual walls. These walls send out infrared signals so are invisible to the human eye, but detectable by the Roombas. Crossing the walls will result in the Roomba being eliminated. A box was placed on both sides of the river which could be used as cover from the enemy team.

The castle is simply a light sensor and some LEDs. The light sensor is placed inside of a red solo cup to make the detectable area larger. The LEDs indicate if the castle is currently being hit (one or two lights on) or if the castle is dead (lights flashing). A rough diagram of the game layout is shown in the following figure.

![Capture the Tower Game Layout](https://i.imgur.com/fT2Jrv5.png)

## Rules

The game rules, from the course website, are as follows

> A group wins if it can "destroy" (shoot down) the other group's castle, or "kills" (shoot down) the other group's robots. Since each robot is equipped with a shield, so it can defend itself or its castle. A robot's movement is required to switch from "cruise" mode to "stand still" mode every minute. That is, it is allowed to move freely within its playing field for one minute, and then it must remain stationary, except that it is allowed to spin, for another minute, and so on. The laser has only enough "energy" to turn on for 30 seconds in total; that is, it cannot be turned on all the time. When a robot runs out of energy, then it can only use its shield but it can't shoot.
>
> Each round is run for about 5 minutes. At the end of each round, if both castles remain, then the group with the most surviving robot wins.

## Implementation

A few modifications to our robot were required in order to prepare it for the game. This section will describe what these modifications are and how we made them.

### Game Mode

As some of the game requirements were very timing specific, we wanted to be able to start playing the game at a precise time. We implemented a "game mode", in which the Roomba would follow all the rules of the game, such as switching movement modes, 30 seconds of laser, and dying if hit by a laser. We switched to game mode when one of our joystick buttons was pressed. This was accompanied with a custom sound loaded onto the Roomba so we would know exactly when we started the game. We also changed the LED onboard the Roomba to green for further clarification of whether or not we were playing the game.

When our Roomba was first turned on, it would not start in game mode. We are able to freely move the Roomba around and shoot the laser without restrictions. It is only in game mode when the rules are followed.

### Movement Modes

While playing the game the Roomba must switching between "cruise" mode and "stand still" mode. We implemented this feature with a periodic task. When the game was started we created a simple periodic task that would simple switch between the two modes every 60 seconds. This task would also play a custom sound to indicate that the mode had changed. In our project we called cruise mode "free mode" and stand still mode "stay mode". The body of the mode change periodic task is,

```c
void modeChange(void) TASK ({
    if (dead || game_on) {
        return;
    }
    mode = (mode == FREE_MODE)
        ? STAY_MODE
        : FREE_MODE;

    // Play a sound to indicate mode switch
    roomba.play_song(MODE_SONG);
})
```

This mode state was used to determine allowed user movements. Autonomous movements were allowed in both modes but in stay mode the user was only able to spin. When choosing a user move while playing the game, we simply looked at which mode we were in, if it was stay mode we restricted the movement to spinning.

### Laser Energy

The laser only had enough energy for 30 seconds of shooting. We implemented this feature with a simple counter,

```c
// 30 seconds === 30000 ms
int32_t numLaserTicks = 30000 / MSECPERTICK;
```

In the periodic task that updated the laser state, if the game was being played and the laser was being shot, we decremented the number of laser ticks remaining by the period of the period of the task. If the number of ticks remaining was less than zero, we did not turn the laser on.

```c
/**
 * A simple periodic task to update the pan and tilt kit's position
 * and laser state
 */
void TickArm(void) {
    TASK({
        arm.tick();
        if (packet.joy1SW() && numLaserTicks > 0) {
            BIT_SET(PORTC, 0);
            if (game_on) {
                numLaserTicks -= ARM_TICK_PERIOD;
            }
        } else {
            BIT_CLR(PORTC, 0);
        }
    })
}
```

### Target

Each Roomba was required to have a target, which when hit with a laser would "kill" the Roomba. When dead, the Roomba could not move or shoot the laser. We implemented the target with a photoresistor placed inside of a large cup. The cup increased the targets area as when hit, the inside of the cup would light up and be detectable by the photoresistor even if it was not hit directly.

We connected the photoresistor to an analog pin and read it using the analog to digital converter. This gave us a value between 0 and 1013 which represented how much light was being detected.

One challenge we faced was varying ambient light. The position of the sun outside, the status of the rooms lights, and the position of shadows in the room were constantly changing. This made detecting when a laser was shooting us difficult. To overcome this we used an exponentially weighted moving average for the ambient light.

\begin{equation*}
  avg_t = \alpha \cdot x + (1 - \alpha) \cdot avg_{t - 1}
\end{equation*}

This moving average would constantly calibrate to the rooms ambient light. To detect if we were shot we performed a simple threshold check of the current value read by the photoresistor to the moving average.

```c
if (game_on && val > light_average + LIGHT_THRESHOLD) {
    die();
}
```

The values that we used for $\alpha$ and the threshold are

- $\alpha = 0.5$
- $\text{threshold} = 30$

When we detected a laser shooting us we would play a sound and change the onboard LED to red. No movement was allowed to be made and the laser would not shoot.

## Results

The full setup of our Roomba can be seen in the following image,

![Roomba Full Setup](https://i.imgur.com/IE97pyn.jpg)

We made a controller using cardboard, tape, and zip ties. The Arduino on the controller was powered using a portable battery. This was super convient as it allowed us to walk around and control the Roomba wirelessly. The controller we made can be seen in the following image.

![Game Controller](https://i.imgur.com/RVRYPa9.jpg)

Even though we did not win the overall competition, we were very happy with our final robot. It was responsive, had good movement controls, and successfully used our project 2 RTOS. We even configured a game joystick to control our Roomba by sending UDP packets from a NodeJS server running on our computers to the remote station. A video of our Roomba moving with the joystick is shown in the following video.

<iframe width="720" height="360" src="https://www.youtube.com/embed/Mu4KXoZp-OU?mute=1" frameborder="0" allow="encrypted-media" allowfullscreen></iframe>
