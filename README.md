# DIY Racing Sim Handbrake

## Overview 

This document will provide an overview of how to build a USB Sim Racing Handbrake Controller. 

The controller can act as a keyboard or joystick (analog or digital) input out of a hydraulic handbrake. The over all construct is fairly straightforward, but you may need to improvise a bit mechanically to make this project work for you.

## Components

For this project you'll first need to acquire a handbrake for the base mechanical platform. The one I've used can be found all over eBay by simply searching "hydraulic hand brake". It's pictured below.

[handbrake]: ./img/handbrake.jpg "Chinesium Hydraulic Hand Brake"

### Mechanical 

* 1 - Generic Hydraulic Handbrake 
* 4 - M8 x 20mm x 1.25 (Sensor Box mounting and Rear O-ring attach point)
* 2 - M8 x 30mm x 1.25 (Front O-Ring attach point)
* 1 - M8 x 80mm x 1.25 ("Piston")
* 1 - 1/2" x 1/16" N52 Neodymium Magnet
* 1 - Spring (?)
* 2 - O-Rings (1-1/8 ID x 1-3/8 OD x 1/8 Diameter) 
* 4 - Nylon Spacers
* 4 - M8 Washers
* 1 - 3D Printed Sensor Box Enclosure
* 1 - 3D Printed Electronics Enclosure (optional)

### Electronics

* 1 - Arduino Device (Teensy 3.x - already has HID support for USB devices)
* 1 - Push Button
* 1 - FD-5WSRGB-A RGB LED
* 1 - SOT-23 Breakout Board
* 1 - DRV5053RA (SOT-23 form factor)

## Assembly

I'm going to preface this section by two caveats. First, I'm writing this document months after I've completed this project, so there are likely to be missing steps in this description. Second, a lot of this project was trial and error, with many small modification to parts along the way. If you run into issues, don't hesitate to message me and I'll do my best to help you.

### Electronics and Software

A basic schematic can be found below.

[handbrake]: ./img/schematic.png "Handbrake Schematic"


### Building up the Handbrake 

First, remove the hydraulic cylinder and the latch hook. Save the bolts M8 bolts from the cylinder mounting, as they should be able to be used later.



