---
layout: default
title: Self-Driving Cart
tagline: Team C9 Project Website
description: Minimal tutorial on making a simple website with GitHub Pages
---

# Weekly Update 4 (4/2 - 4/9)

**Vikram**

+ Spent a lot of time debugging the chassis. Came to the conclusion that the
  fuse bits are incorrectly programmed.
+ In contact with SeeedStudio support to get a replacement.
+ Found an alternate HBridge options.
+ Worked with David on ROS issues (described below)
+ Implemented and integrated apriltag detection in the brain app

**David**

+ Fleshed out high-level implementation details of 'brain app'
+ All components share a top-level store of all state information.
+ Within each frame, the app reads sensor data, calculates state updates, and
  communicates actuation commands to the ESP
+ Created skeleton application logic, including no-op update methods for
  localization, obstacle detection, and pathing modules
+ With Vikram, planned map representation and mapping interface

**Naveen**

+ Set up message passing interface for the microcontroller
+ The current set up matches on "\n", which tells esp that entire command has been sent
+ The message is then sent to either the LIDAR or motor task, which currently
  just prints out the message it has received
+ I tested this entire setup using an Arduino as the "phone", and was able to
  sucessfully pass an incoming message from the "phone" to both the motor and
  the LIDAR task
