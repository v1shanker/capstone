---
layout: default
title: Self-Driving Cart
tagline: Team C9 Project Website
description: Minimal tutorial on making a simple website with GitHub Pages
---

# Weekly Update 3 (3/26 - 4/2)

**Vikram**

+ Spent a lot of time debugging the chassis. Came to the conclusion that the
  fuse bits are incorrectly programmed.
+ In contact with SeeedStudio support to get a replacement.
+ Found an alternate HBridge options.
+ Worked with David on ROS issues (described below)
+ Implemented and integrated apriltag detection in the brain app

**David**

+ Worked with Vikram to diagnose issues with Hercules Control Board
+ Used [existing toolchain to cross-compile ROS C++ Packages for Android](http://github.com/ekumenlabs/roscpp_android)
+ Found issues with toolchain's build output
+ These issues prevent integration of C++ packages with Android JNI, so 
  we are abandoning the prospect of using ROS packages. Instead we will use 
  other solutions for integrating C++ packages (like AprilTags) and roll our
  own solutions where needed. This means our capabilities down the road will
  be less sophisticated than existing ROS solutions, but it means that rather
  than running into brick walls applying poorly-documented tools, we will be
  implementing our own solutions. 
+ With Vikram, re-integrated AprilTags C++ library with Android, using
  [existing JNI wrapper](http://github.com/johnjwang/apriltag-android) 
  as a base. We will need to expand these JNI wrappers to expose more of the
  underlying functionality which we will be depending on.
+ Integrated [USB Serial Library for Android](http://github.com/mik3y/usb-serial-for-android)
  into brain app
+ With Naveen, implemented communications demo to discover attached USB device
  and test read/write communication to a program running on the ESP32.

**Naveen**

+ Ported C++ LIDAR SDK over to C code to compile on esp32
+ Set up serial connection between esp32 and LIDAR
+ Connected to LIDAR and read Serial number, managed to get basic data about angle and distance
+ Helped set up esp32 side of serial connection between Android phone

