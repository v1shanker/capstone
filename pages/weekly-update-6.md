---
layout: default
title: Self-Driving Cart
tagline: Team C9 Project Website
description: Minimal tutorial on making a simple website with GitHub Pages
---

# Weekly Update 6 (4/16 - 4/23)

**Vikram**

+ Completed the AprilTag recognition acitivies.
+ Added interface for distance and phase calculations.
+ With DAVID, set up control of motors with new replacement h-bridge boards
  using lab power supply and PWM signal from function generator
+ Debugged existing battery issues.
+ Designed the implemented the driver to control the chassis to control the
  chassis programatically.

**David**

+ Integrated AprilTag recognition and serial communications into brain app 
  update loop
+ Set up simple demo to report AprilTags recognized by brain app and sending
  simple control commands to the ESP
+ Designed interface of occupancy grid map representation, including
  AprilTag locations
+ With Vikram, set up control of motors with new replacement h-bridge boards
  using lab power supply and PWM signal from function generator
+ With Vikram, helped debug issues with existing battery

**Naveen**

+ Set up message passing interface for the microcontroller
+ The current set up matches on "\n", which tells esp that entire command has been sent
+ The message is then sent to either the LIDAR or motor task, which currently
  just prints out the message it has received
+ I tested this entire setup using an Arduino as the "phone", and was able to
  sucessfully pass an incoming message from the "phone" to both the motor and
  the LIDAR task
