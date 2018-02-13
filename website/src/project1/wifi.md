## WiFi Communication

[wifiboard]: https://i.imgur.com/1wicDVT.jpg
[wifiset]: https://i.imgur.com/cXeNw2m.png
[netset]: https://i.imgur.com/2dCzdG5.png
[wireshark]: https://i.imgur.com/ofInh8r.png

With our project we opted to pilot the usage of WiFi as a wireless communications method. Other groups have traditionally used Bluetooth to communicate with their remote devices. The potential advantages for using WiFi are to allow geographically remote control, easier payload inspection, and the ability to broadcast commands to devices.

The WiFi modules used are the DOIT DT-06 which is based on the ESP8266 WiFi card.

![DT-06 WiFi board][wifiboard]

These WiFi boards have built in software controlled access points (SoftAP) and HTTP servers, allowing them to operate without the need for external networks. They also provide the same RX/TX pins that our lab's traditonal bluetooth boards posess, along with an onboard Serial decoder that translates UART data into IP packets. This means that if our tests with WiFi control are successful, the remainder of the lab can move to using WiFi instead of bluetooth.


### Setup

Setup for the DT-06 is extremely intuitive, simply provide power to the board and the DT-06 will create an open network ending with the last 6 bytes of its MAC address. Connect and use a web browser to access the modules configuration page located by default at 192.168.4.1.

The two main setup pages are for configuring the SoftAP, the network the DT-06 should connect to, and the type of service that the DT-06 should run.

![WiFi configuration on DT-06][wifiset]

![Network configuration on DT-06][netset]

The DT-06 has several other web accessible pages - not pictured - to view status, set baud rates, restart, and apply settings. We configured our WiFi cards to broadcast data to 192.168.1.255 over UDP on port 9000, furthermore we configured them to connect to the 'ECS366G' network. This allows us to monitor and inject WiFi traffic using Wireshark without connecting directly to the DT-06 modules.

### Packet Specification

As part of the requirements of Phase 2, we needed to be able to send commands to the roomba and the pan and tilt kit mounted on top. For ease of development we decided to simply send the values of the two joysticks in a single packet and interpolate the values on the remote side.

The packet was declared using the following fields:

| Name         | Size    |  Range    |   Description                         |
|:-------------|:--------|:----------|:--------------------------------------|
| Magic        | 1 Byte  | 243       | Start of packet                       |
| Joy1X        | 2 Bytes | 0 - 1023  | X axis reading of Joystick 1          |
| Joy1Y        | 2 Bytes | 0 - 1023  | Y axis reading of Joystick 1          |
| Joy2X        | 2 Bytes | 0 - 1023  | X axis reading of Joystick 2          |
| Joy2Y        | 2 Bytes | 0 - 1023  | Y axis reading of Joystick 2          |
| Joy1SW       | 1 Byte  | 0 or 255  | Z axis (switch) reading of Joystick 1 |
| Joy2SW       | 1 Byte  | 0 or 255  | Z axis (switch) reading of Joystick 2 |

Using a magic number at the front of the packet allows us to segment the incoming data steam into packets and detect transmission errors by waiting until the magic number appears, then reading the appropriate number of following bytes - the following 10 bytes in our case. If an inappropriate number of bytes are available to read then the packet is discarded.

![Wireshark capture showing packet data][wireshark]

The above screenshot shows a Wireshark capture of the some sample packets, note that the two byte fields are transmitted in little-endian format. Using Wireshark proved a useful tool for debugging our project for phase 2 when data was communicated wirelessly.
