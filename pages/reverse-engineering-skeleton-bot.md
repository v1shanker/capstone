# Reverse Engineering the Skeleton Bot Interface

The Hercules Skeleton Bot 4WD is desinged to be used with the Arudino. A number
of libraries are available to make controlling the bot easy. However, there is
no documentation or datasheet available for the motor control board itself,
making controlling the bot extremely difficult.

## Why?

We want to be able to control the bot directly from the ESP32 instead of an
Arduino for a number of reasons. However, this required a great amount of
reverse-engineering of the not only the Motor Controller library but also the
RFBee remote controlled example library.

## Data Format

The data expected by the bot is 6 bytes over and I2C interface. The data format
is shown below.

```C
/** @brief first magic start byte */
#define START1    0x53
/** @brief second magic start byte */
#define START2    0x01
/** @brief first magic end byte */
#define END1      0x2F
/** @brief second magic end byte */
#define END2      0x45

data = {START1, START2, SPEED_LEFT, SPEED_RIGHT, END1, END2}
```

## Speed Format

There are two speed values. One for the left motors and one for the right
motors. The following rules govern the expectation of the format of each speed
value.

+ The speed value is represented as signed magnitude
  * most significant bit of **0** is **forward**
  * most significant bit of **1** is **reverse**

+ The magnitude of each bit should be between **0** and **100**.


## I2C Address

The skeleton bot's I2C slave address is **0x04**.
