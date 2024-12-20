# Device-TesterHat

![alt text](Pictures/NodeV1WithHat.jpg)

## Overview

A test device to facilitate easy testing of various system function. Has four DIP switches for digital input, one pot for analog input, and four LEDs for analog(PWM)/digital output. All devices are attached to a small protoboard that is directly connected to the node controller's headers. This is a internal test device and as such will likely never been made into a finalized production device. A protoboard hat was only ever created for the Node Controller V1 PCB, however, the firmware was used with the Node Controller V2 PCB, but with no attached hardware.

## Hardware

Known Issues
* LEDs are all wired through one current limiting resistor(1k) so brightness is inconsistent

![alt text](Pictures/Schematic.png)

## Firmware

### Message IDs and Formats

* Pot - 0xB000 - 0 to 4096 integer
* Button 1 - 0xB001 - 1 bit bool
* Button 2 - 0xB002 - 1 bit bool 
* Button 3 - 0xB003 - 1 bit bool 
* Button 4 - 0xB004 - 1 bit bool  

* LED 1 - 0xC000 - 0 to 255 integer representing brightness  
* LED 2 - 0xC001 - 0 to 255 integer representing brightness  
* LED 3 - 0xC002 - 0 to 255 integer representing brightness  
* LED 4 - 0xC003 - 0 to 255 integer representing brightness
