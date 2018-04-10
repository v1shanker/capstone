---
layout: default
title: Self-Driving Cart
tagline: Team C9 Project Website
description: Minimal tutorial on making a simple website with GitHub Pages
---

# Weekly Update 4 (April 2nd - April 9th)

**Vikram**

+ Spent a lot of time debugging the chassis. Came to the conclusion that the
  fuse bits are incorrectly programmed.
+ In contact with SeeedStudio support to get a replacement.
+ Found an alternate HBridge options.
+ Worked with David on ROS issues (described below)
+ Implemented and integrated apriltag detection in the brain app

**David**

+ With Naveen, fleshed out the 
+ Working on getting the brain logic to run in the background
+ Did research into Android framework support for long-running jobs in the background
+ Android offers four means of running jobs in the background: AsyncTasks, Threading, Services,
  and IntentServices. Each is suited for different use cases.
+ Since AsyncTasks are not well-suited to long-running jobs, and Threading requires manual 
  lifecycle management, our choice is primarily between a Service and an IntentService.
+ A Service is not intended to run for a long time in the background, so Android will kill it unless
  it displays some kind of UI like a notification.
+ An IntentService is the intended way for jobs to run in the background, but it requires an event to
  fire periodically from some other service.
+ For our purposes, a Service is better suited - a notification is acceptable, but there is no
  obvious external event to trigger the IntentService.
+ Currently working on managing the service lifecycle and laying framework for the main loop.
+ The loop will consist of reading sensor data, updating movement state information, repathing if
  necessary, and issuing motor commands.

**Naveen**

+ Ported C++ LIDAR SDK over to C code to compile on esp32
+ Set up serial connection between esp32 and LIDAR
+ Connected to LIDAR and read Serial number, managed to get basic data about angle and distance
+ Helped set up esp32 side of serial connection between Android phone

