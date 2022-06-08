# Hydruino
Hydruino: Simple automation controller for hydroponic grow systems.

**Simple-Hydroponics-Arduino v0.1**

TODO
Licensed under the non-restrictive MIT license.

Created by NachtRaveVL, May 20th, 2022.

TODO

Made primarily for Arduino microcontrollers, but should work with PlatformIO, ESP32/8266, Teensy, and others - although one might experience turbulence until the bug reports get ironed out. Unknown architectures must ensure `BUFFER_LENGTH` (or `I2C_BUFFER_LENGTH`) and `WIRE_INTERFACES_COUNT` are properly defined.

Dependencies include: Adafruit BusIO, Adafruit Unified Sensor, CoopTask (alternate to Scheduler, disableable), DallasTemperature, DHT sensor library, EasyBuzzer, I2C_EEPROM, IoAbstraction LiquidCrystalIO, OneWire, RTClib, Scheduler (SAM/SAMD only, disableable), SimpleCollections, TaskManagerIO, tcMenu, and Time.

TODO

## Controller Setup

### Installation

The easiest way to install this controller is to utilize the Arduino IDE library manager, or through a package manager such as PlatformIO. Otherwise, simply download this controller and extract its files into a `Simple-Hydroponics-Arduino` folder in your Arduino custom libraries folder, typically found in your `[My ]Documents\Arduino\libraries` folder (Windows), or `~/Documents/Arduino/libraries/` folder (Linux/OSX).

### Header Defines

There are several defines inside of the controller's main header file that allow for more fine-tuned control of the controller. You may edit and uncomment these lines directly, or supply them via custom build flags. While editing the main header file isn't ideal, it is often the easiest given the Arduino IDE's limited custom build flag support. Note that editing the controller's main header file directly will affect all projects compiled on your system using those modified controller files.

Alternatively, you may also refer to <https://forum.arduino.cc/index.php?topic=602603.0> on how to define custom build flags manually via modifying the platform[.local].txt file. Note that editing such directly will affect all other projects compiled on your system using those modified platform framework files.

From Hydroponics.h:
```Arduino
// TODO: Reinclude this example after modifications completed. -NR
```

### Controller Initialization

There are several initialization mode settings exposed through this controller that are used for more fine-tuned control.

#### Class Instantiation

The controller's class object must first be instantiated, commonly at the top of the sketch where pin setups are defined (or exposed through some other mechanism), which makes a call to the controller's class constructor. The constructor allows one to set the module's XXX TODO, i2c Wire class instance, if on Espressif then i2c SDA pin and i2c SCL pin, and lastly i2c clock speed. The default constructor values of the controller, if left unspecified, is XXX TODO i2c Wire class instance `Wire` @`400k`Hz, and if on Espressif then i2c SDA pin `D21` and i2c SCL pin `D22` (ESP32[-S] defaults).

From Hydroponics.h, in class Hydroponics:
```Arduino
// TODO: Reinclude this example after modifications completed. -NR
```

## Hookup Callouts

* TODO

## Memory Callouts

* TODO

## Example Usage

Below are several examples of controller usage.

### Simple Deep Water Culture (DWC) System Example

```Arduino
// TODO: Reinclude this example after modifications completed. -NR
```

### Vertical Nutrient Film Technique (NFT) System Example

```Arduino
// TODO: Reinclude this example after modifications completed. -NR
```

