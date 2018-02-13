## Servo

[pwm]: https://i.imgur.com/QWYOqPr.png "Pulse width modulation"
[arm]: https://i.imgur.com/3WnlKf7.jpg "Servo pan and tilt kit"

We use a pair of servo motors to aim our laser. The two servos are mounted in a pan and tilt kit that allows us to point the laser anywhere within $\pm 90°$ left and right and $0°$ to roughly $170°$ vertically from the front of the robot. The two servos used are the SG-90 $9g$ micro servo. The associated datasheet can be found [at this link](https://www.heinpragt.com/techniek/robotica/images/SG90Servo.pdf).

![Servo pan and tilt kit][arm]

The position of the joystick is used to control the servos. The reading from the joystick is used to calculate the relative speed to move the joystick, i.e. when the joystick is centered the servos do not move. The X axis reading of the joystick maps to the arm's yaw (bottom servo), and likewise the Y axis maps to the arm's pitch (top servo).

The servos are controlled using a digital output from the microcontroller, contrary to the  common assumption that servo speed is controlled using voltage level. The waveform that is output over the digital pin is a square wave with a variant width. The technique of varying the duration of the waveform's 'high' cycle is called Pulse Width Modulation (PWM). PWM is used in many control applications, and is also required in order to control the SG-90 servos.

![PWM example waveform][pwm]

In PWM control, the position of the servo is determined by the length of the duty cycle. Our servos support a duty cycle from $544\mu s$ to $2400\mu s$. To simplify, assume a duty cycle range from $1ms$ to $2ms$ like in the figure above; then for example, to set a servo with a range from $0°$ to $180°$ to the $90°$ position the duty cycle should be $1.5ms$, and to set the same servo to the $45°$ position the duty cycle should be $1.25ms$. This calculation is currently performed by the *Servo* library provided by Arduino.
