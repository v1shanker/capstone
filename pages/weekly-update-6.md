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
+ With David, set up control of motors with new replacement h-bridge boards
  using lab power supply and PWM signal from function generator.
+ With David and Naveen, designed interface of occupancy grid map representation,
  including AprilTag locations.
+ Debugged existing battery issues.
+ Designed the implemented the driver to control the chassis to control the
  chassis programatically.
+ Helped David with various android-based programming constructs.

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

+ Set up message passing for sending motor commands to the PWM driver 
+ Set up other side of demo where motor start and start commands would be registered by ESP
+ Managed to retrieve scan data from LIDAR and process it on ESP - ran in to a lot of
  subtle bugs when trying to get this to work. 
+ Discussed with David and Vikram how we're going to represent maps on Android app
