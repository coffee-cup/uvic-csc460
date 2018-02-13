## Laser and Light Sensor

[tempsa]: https://i.imgur.com/P7iovUG.jpg "Tempsa 650nm 5mW barrel laser"

[keyes]: https://i.imgur.com/MQWzkdP.jpg "Keyes Hoya Mounted 650nm 5mW barrel laser"

[LSSchematic]: https://i.imgur.com/bpMK6aL.png "Light Sensor Schematic"

[singleLS]: https://i.imgur.com/QN6xlni.png "Single Light Sensor testing"

[TIRSensorOpen]: https://i.imgur.com/OxCo7bA.jpg "Internal Reflection Sensor"

[TIRCircuit]: https://i.imgur.com/JGEYQnX.jpg "Internal Reflection circuit"

[TIRSensorLit]: https://i.imgur.com/TxO4daI.jpg "Internal Reflection circuit, ON"



We use a laser and light sensor as the basis for implementing the ability to play laser tag with the other teams. In phase one, both the laser and the light sensor are attached to the same prototyping board. We had the choice between two lasers: the Tempsa laser or the Keyes Hoya PCB mounted laser. The Tempsa laser is shipped with two loose ends that would have to be soldered to other wires before being usable. The Keyes laser is mounted on a PCB with male pinouts. Additionally the Keyes laser comes with a built-in inline resistor to help with short-circuit protection.

|  ![][tempsa]   |        ![][keyes]          |
|:--------------:|:--------------------------:|
|  Tempsa Laser  |  Keyes Hoya Mounted Laser  |


Although both options use the same 6mm copper tube laser diode they have different power consumptions. This is likely due to the inline resistor found on the Keyes laser, which draws $150mW$ ($30mA$ at $5V$) power whereas the Tempsa laser only draws $50mW$ ($10mA$ at $5V$). Both lasers can conveniently be driven directly from a digital output on the ATmega2560, although it would be unwise the use the Tempsa laser without a short-circuit protection resistor wired in series with the return. Also, oddly, the Keyes laser has three pins on it's PCB, yet only the outer two (Labelled S: Signal, -: Ground) are required to drive the laser. The third pin seems to be a $V_{ref}$ pin as it provides the same voltage as the signal pin.

The Keyes laser seems easier to work with due to the built-in short circuit protection and factory mounted PCB. With this in mind we decided to work with the Keyes Hoya laser for the remainder of the project.

For the light sensor, we are provided access to some photoresistors. They are comparable to the PDV-P8001 as described on this [datasheet](https://cdn-learn.adafruit.com/assets/assets/000/010/127/original/PDV-P8001.pdf). Importantly, this light sensor should be wired with a series resistor on the ground side to if it is to be used as a digital input. See the schematic below.

![][LSSchematic]

Without the ground side resistor the input would be floating and thus any readings would be useless. We did initial testing on a single photoresistor, then attempted to design another sensor that would be easier to hit from a distance. Previous years had used the bottom of a red solo cup to mount several photoresistors in parallel. We settled on a design which leverages reflection rather than increased sensor area.



|      ![][singleLS]       |  ![][TIRCircuit]  |
|:------------------------:|:-----------------:|
|   Testing the sensor     |     Our circuit   |

|    ![][TIRSensorOpen]    | ![][TIRSensorLit] |
|:------------------------:|:-----------------:|
|      Internal design     |      Active       |

The sensor is constructed from the center piece of 1.25cm thick plexiglass cut using a 2.5cm hole saw. The photoresistor is mounted within the center hole using some hot glue, and the clear faces of the plexiglass are covered with reflective thermal insulation tape. The cut edges of the plexiglass are partially opaque, thus when a light is shone on the sensor, a large amount of the light is reflected internally. To help with tuning the sensitivity of our light sensor we used a variable resistor (potentiometer) to control the threshold of when the sensor triggers. In the daytime lighting conditions of ECS 366, we found that a resistance of $22k\Omega$ performed well for differentiating active and inactive states of our light sensor.






