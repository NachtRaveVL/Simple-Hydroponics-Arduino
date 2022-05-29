# Hydroponics-Arduino
Arduino Controller for Simple Hydroponics.

**Hydroponics-Arduino v0.1**

TODO
Licensed under the non-restrictive MIT license.

Created by NachtRaveVL, May 20th, 2022.

TODO

Made primarily for Arduino microcontrollers, but should work with PlatformIO, ESP32/8266, Teensy, and others - although one might want to ensure `BUFFER_LENGTH` (or `I2C_BUFFER_LENGTH`) and `WIRE_INTERFACES_COUNT` is properly defined for any architecture used.

Dependencies include Scheduler if on a ARM/ARMD architecture (e.g. Due/Zero/etc.), but usage can be disabled via library setup header defines or custom build flags.

TODO

## Library Setup

### Installation

The easiest way to install this library is to utilize the Arduino IDE library manager, or through a package manager such as PlatformIO. Otherwise, simply download this library and extract its files into a `Hydroponics-Arduino` folder in your Arduino custom libraries folder, typically found in your `[My ]Documents\Arduino\libraries` folder (Windows), or `~/Documents/Arduino/libraries/` folder (Linux/OSX).

### Header Defines
 
There are several defines inside of the library's main header file that allow for more fine-tuned control of the library. You may edit and uncomment these lines directly, or supply them via custom build flags. While editing the main header file isn't ideal, it is often the easiest given the Arduino IDE's limited custom build flag support. Note that editing the library's main header file directly will affect all projects compiled on your system using those modified library files.

Alternatively, you may also refer to <https://forum.arduino.cc/index.php?topic=602603.0> on how to define custom build flags manually via modifying the platform[.local].txt file. Note that editing such directly will affect all other projects compiled on your system using those modified platform framework files.

From Hydroponics.h:
```Arduino
// Uncomment or -D this define to enable use of the software i2c library (min 4MHz+ processor).
//#define HYDRO_ENABLE_SOFTWARE_I2C             // http://playground.arduino.cc/Main/SoftwareI2CLibrary

// Uncomment or -D this define to disable usage of the Scheduler library on SAM/SAMD architecures.
//#define HYDRO_DISABLE_SCHEDULER               // https://github.com/arduino-libraries/Scheduler

// Uncomment or -D this define to enable debug output.
//#define HYDRO_ENABLE_DEBUG_OUTPUT
```

### Library Initialization

There are several initialization mode settings exposed through this library that are used for more fine-tuned control.

#### Class Instantiation

The library's class object must first be instantiated, commonly at the top of the sketch where pin setups are defined (or exposed through some other mechanism), which makes a call to the library's class constructor. The constructor allows one to set the module's SPI CS pin, ISR VSync pin, i2c Wire class instance, if on Espressif then i2c SDA pin and i2c SCL pin, and lastly i2c clock speed (all i2c parameters being ommitted when in software i2c mode). The default constructor values of the library, if left unspecified, is SPI CS pin `D10`, ISR VSync pin `DISABLED`, i2c Wire class instance `Wire` @`400k`Hz, and if on Espressif then i2c SDA pin `D21` and i2c SCL pin `D22` (ESP32[-S] defaults).

From Hydroponics.h, in class Hydroponics, when in hardware i2c mode:
```Arduino
    // Library constructor. Typically called during class instantiation, before setup().
    // ISR VSync pin only available for Lepton FLiR breakout board v2+ (GPIO3=VSYNC).
    // Boards with more than one i2c line (e.g. Due/Teensy/etc.) can supply a different
    // Wire instance, such as Wire1 (using SDA1/SCL1), Wire2 (using SDA2/SCL2), etc.
    // Supported i2c clock speeds are 100kHz, 400kHz, and 1000kHz.
    // Supported SPI clock speeds are ~2.2MHz(@80x60)/~8.8MHz(@160x120) to 20MHz.
    Hydroponics(byte spiCSPin = 10, byte isrVSyncPin = DISABLED, TwoWire& i2cWire = Wire, uint32_t i2cSpeed = 400000);

    // Convenience constructor for custom Wire instance. See main constructor.
    Hydroponics(TwoWire& i2cWire, uint32_t i2cSpeed = 400000, byte spiCSPin = 10, byte isrVSyncPin = DISABLED);
```

From Hydroponics.h, in class Hydroponics, when in software i2c mode (see examples for sample usage):
```Arduino
    // Library constructor. Typically called during class instantiation, before setup().
    // ISR VSync pin only available for Lepton FLiR breakout board v2+ (GPIO3=VSYNC).
    // Minimum supported i2c clock speed is 100kHz, which sets minimum processor speed at
    // 4MHz+ running in i2c standard mode. For up to 400kHz i2c clock speeds, minimum
    // processor speed is 16MHz+ running in i2c fast mode.
    // Supported SPI clock speeds are ~2.2MHz(@80x60)/~8.8MHz(@160x120) to 20MHz.
    Hydroponics(byte spiCSPin = 10, byte isrVSyncPin = DISABLED);
```

## Hookup Callouts

* TODO

## Memory Callouts


## Example Usage

Below are several examples of library usage.

### Simple Example

```Arduino
// TODO: Reinclude this example after modifications completed. -NR
```
