# BLV_NeoPixelIntegration
This project contains a very messy code to readout the serial communication between a Duet3D Printer Board and the PanelDue. This Information is displayed on 3 NeoPixel rings.

NeoPixel related and Code comes from the following source:
https://learn.adafruit.com/multi-tasking-the-arduino-part-1
https://learn.adafruit.com/multi-tasking-the-arduino-part-2
https://learn.adafruit.com/multi-tasking-the-arduino-part-3
It was modified that it fits to our needs.

Some informations regarding the transmitted data via serial communication comes from:
https://github.com/dc42/PanelDueFirmware

TODOS

- Serial Port Sniffing and Message Handling -> State Maschine and hopefully get rid of the wrong readings.
- Progress Circles pulsating and get rid of that one red dot (has something todo with the counting)
- When Printing is finished, switch back to temperatur display
- global dimming value
- opt: Integration of more designs
- opt: use one or two switches to change designs or shown values


Used Hardware:
- Arduino Mega incl. additional 5V Powersupply
- 3 NeoPixel Rings with 16 Pixels each
- Duet3D Wifi Board
- Duet PanelDue 5"Display

Wiring:
Digital Pin 6 -> Input of left NeoPixel
Digital Pin 7 -> Input of center NeoPixel
Digital Pin 8 -> Input of right NeoPixel
Diital Pin 17 (RX2) -> UTXD0 Pin Duet3D Board (be careful, don't connect the RX Pin of the Board)

Connect all Neopixel 5V Power to the 5V Power Supply. Do the same with the Ground.
Connect the second Arduino Ground Pin with the PanelDue Ground.

Thats it for now!
