# Roomba

## Open Interface

## Movement

Our Roomba used two main types of movement, autonomous and user controlled. We used the direct drive function so we could control the speeds and direction and each well individually. This gave us more fine grained control over how the Roomba moved in comparison to the other drive function. When choosing a move to make we use percentage based speeds between 0 and 100. This allows us to adjust a single `MAX_SPEED` variable which controls how fast the Roomba moves at max speed.

### User Movement

The user was the main controller of the Roomba. The x and y translational information from a single joystick is sent from the base station to the remote station. On the remote station we convert this 16 bit value (0-1023) to left and right wheel speeds for the Roomba. We use four different movement modes. These are,

- angled,
- spin,
- straight, and
- stop

In angled mode the left and right wheels of the Roomba move at different velocities. In this mode the _driver_ can move the Roomba in arcs, circles, and at angles. In spin mode the wheels will spin at the same speed but in different directions. This spins the Roomba in a single spot. In straight mode both wheels will move at the same speed and in the same direction. The Roomba with either move straight forwards or straight backwards. Finally, in stop mode the Roomba will not move at all. The mode we use depends on the x and y positions of the joystick. The following figure shows which mode we choose. The horizontal and vertical bands are the width of a "deadband" threshold value.

![Joystick Controls Translated to Roomba Movement](https://i.imgur.com/hjPEw4n.png)

The following code chooses a move based on the `joyX` and `joyY` positions.

```c
void choose_user_move(Move *move, uint16_t joyX, uint16_t joyY) {
    // Map x and y from [0, 1023] to [100, 100]
    int16_t x = cmap_u(joyX, 0, 1023, -100, 100);
    int16_t y = cmap_u(joyY, 0, 1023, -100, 100);

    if (abs_u(x) > DEADBAND && abs_u(y) > DEADBAND) {
        // Angled drive
        x = cmap_u(x, -100, 100, -25, 25);
        int16_t left_x = 25 + x;
        int16_t right_x = 25 - x;

        if (y < 0) {
            left_x *= -1;
            right_x *= -1;
        }

        y = cmap_u(y, -100, 100, -25, 25);
        set_speeds(move,
                   cmap_u(left_x + y, -75, 75, -100, 100),
                   cmap_u(right_x + y, -75, 75, -100, 100));
    } else if (abs_u(y) > DEADBAND) {
        // Straight drive
        forward(move, y);
    } else if (abs_u(x) > DEADBAND) {
        // Spin drive
        spin_right(move, x);
    } else {
        // Stop
        stop(move);
    }
}
```

### Autonomous Movement

Even though the user was the main source of control for the Roomba, it still autonomously took over control if

- any of the bumpers were pressed or
- the invisible wall was detected.

Before a user movement action was selected, the sensors on the Roomba were checked. This was to ensure that the Roomba would never put itself in a harmful or unallowed position. The Roomba has two front bumpers which trigger when pressed in, commonly when hitting a wall. We detect this and move the Roomba in a suitable way in order to avoid the wall. If the left bumper was pressed we spin to the right. If the right bumper was pressed we spin to the left. If both bumpers were pressed we back up, as this indicates a wall directly in front of us.

In the case of the invisible wall, we simply move backwards if it is detected. We assume that the Roomba was moving forward and then detected the wall. In most situations this action is correct. However, the invisible wall can be crossed by moving across it backwards. If we were to continue developing this project, it would be better for us to move in the opposite direction we were moving if the invisible wall was detected. In our robot, all autonomous actions override user actions. For this reason we check all sensors and make an autonomous movement first. If no move was selected we then act based on user input. The function we use to choose a move is shown in the following code. Note that the `choose_user_move` function is the same function shown above.

```c
void choose_move(Move* move) {
    // Check sensors
    bool is_wall = roomba.check_virtual_wall();
    bool is_leftb = roomba.check_left_bumper();
    bool is_rightb = roomba.check_right_bumper();

    // Set base move to stop
    stop(move);

    // Do not move if dead
    if (game_on && dead) {
        return;
    }

    // Invisible wall pressed
    if (is_wall) {
        backward(move, 50);
        return;
    }

    // Both bumpers pressed
    if (is_leftb && is_rightb) {
        backward(move, 50);
        return;
    }

    // Left bumper pressed
    if (is_leftb) {
        spin_right(move, 50);
        return;
    }

    // Right bumper pressed
    if (is_rightb) {
        spin_left(move, 50);
        return;
    }

    // Move based on the users controls
    choose_user_move(move, packet.joy1X(), packet.joy1Y());
}
```

The autonomous movement in this project was fairly simple compared to that of a self driving car or more advanced robot. However, we still designed the software in such a way that autonomous actions could easily be incrementally added.
