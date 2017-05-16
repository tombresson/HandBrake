# USB Handbrake 

## Abstract

This goal of this project was to use a commercially available handbrake and convert it into a USB controller that could output an analog axis value, a joystick button (at a configurable threshold) or a keypress (at a configurable threshold).  The parts had to be readily available, but 3D printed parts could be utilized. The cost also had to be low, as this was a project for for a DIY piece of Sim Racing kit. Commercially available hardware already exists for a high (but not exuberant) price.

## Features

The USB Handbrake has the following features:

* Outputs a Joystick Analog (z-axis), Joystick Digital (button 1) or Keyboard key to provide universal support to any PC game
* Allows configuration of the device through the serial interface to configure:
  * Key that is pressed when in "Keyboard" mode
  * Threshold for the digital outputs
  * RGB LED Brightness
  * Deadzone for the "Pulled" and "Released" positions
* Allows for calibration of the device to set the full "Released" and "Pulled" sensor values
* Returns to the last mode after on power-up (Joystick Analog, Joystick Digital, or Keyboard)

## Hardware

### Mechanical

#### Bill of Materials

1 - Generic Generic Hydraulic Handbrake 
4 - M8 x 20mm x 1.25 (Box mounting and Rear O-ring attach point)
2 - M8 x 30mm x 1.25 (Front O-Ring attach point)
1 - M8 x 80mm x 1.25 ("Piston")
1 - 1/2" x 1/16" N52 Neodymium Magnet
1 - Spring (?)
2 - O-Rings (1-1/8 ID x 1-3/8 OD x 1/8 Diameter)
4 - Nylon Spacers
4 - M8 Washers
1 - 3D Printed Sensor Enclosure
1 - 3D Printed Bolt Guide Plate

#### Assembly


### Electronics

#### Bill of Materials

1 - Arduino platform (Teensy 3.1)
1 - Push Button
1 - RGB LED
1 - SOT-23 Breakout Board
1 - DRV5053RA (SOT-23 form factor)

#### Schematic

## Software