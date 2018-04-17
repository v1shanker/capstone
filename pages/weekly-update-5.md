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

+ Met with David to flesh out AT interface
+ ESP32 has a pattern recognition function for UART, may use that instead of AT which would require
  less work than repeatedly scanning for characters "A""T".
+ Decided to split esp32 operation into 3 parts.
+ The android task will be responsible for any interactions with the phone. Incoming messages will be
  distributed to the LIDAR or motor, depending on the command. There will also be an ACK bit sent back
  to the phone to ensure that the command was successfully processed by the ESP32.
+ The motor task will periodically check it's command buffer and process any commands, sending output
  if necessary to the buffer that will then be sent back to the phone
+ The LIDAR task will also check it's buffer and process any scan commands, sending output back to the buffer
  which will then be sent back to the android phone
