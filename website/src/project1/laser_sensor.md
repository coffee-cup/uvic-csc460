## Laser and Light Sensor

We use a laser and light sensor as the basis for implementing the ability to play laser tag with the other teams. In phase one, both the laser and the light sensor are attached to the same prototyping board. We had the choice between two lasers: the Tempsa laser or the Keyes Hoya PCB mounted laser. We chose to work with the Keyes laser because it is conveniently mounted on a PCB with male pin outs. The Tempsa laser is shipped with two loose ends that would have to be soldered to other wires before being usable. Additionally the Keyes laser comes with a builtin inline resistor to help with short-circuit protection.

|  ![][tempsa]   |        ![][keyes]          |
|:--------------:|:--------------------------:|
|  Tempsa Laser  |  Keyes Hoya Mounted Laser  |

[tempsa]: https://drive.google.com/uc?export=download&id=1qWeOWczlumMEu5fMqrc_7EZPUgUOsRCf "Tempsa 650nm 5mW barrel laser"
[keyes]: https://drive.google.com/uc?export=download&id=1pMls5j6DNy5kO0141cP4m419n5CjOS2g "Keyes Hoya Mounted 650nm 5mW barrel laser"

Although both lasers seem to use the same 6mm copper tube laser diode they have different power consumptions. This is likely due to the inline resistor found on the Keyes laser, it draws $150mW$ ($30mA$ at $5V$) power whereas the Tempsa laser only draws $50mW$ ($10mA$ at $5V$). Both lasers can conveniently be driven directly from a digital output on the ATmega2560, although it would be unwise the use the Tempsa laser without a short-circuit protection resistor in series. The Keyes laser has three pins on it's PCB, yet only the outer two (Labelled S: Signal, -: Ground) are required to drive the laser. The thrid pin seems to be a $V_{ref}$ pin as it provides the same voltage as the signal pin. For these reasons we choose to work with the Keyes laser for this project.

