# Hydruino
Hydruino: Simple Hydroponics Automation Controller.

**Simple-Hydroponics-Arduino v0.2**

Simple automation controller for hydroponic grow systems using an Arduino board.  
Licensed under the non-restrictive MIT license.

Created by NachtRaveVL, May 20th, 2022.

**UNDER ACTIVE DEVELOPMENT BUT DONT EXPECT ANY MIRACLES**

This controller allows one to set up an entire system of sensors, pumps, relays, probes, and other things useful in automating the lighting, feeding, watering, and sensor data monitoring & collection process involved in hydroponically grown fruits, vegetables, teas, herbs, and salves. It contains a large library of crop types to select from that will automatically aim the system for the best growing parameters during the various growth phases with the hardware you have available. Works with a large variety of common aquarium equipment and hobby sensors. Supports sensor data logging to MQTT (for IoT integration), .csv data files on an SD card or Network share (for custom graphing), and can be extended to work with other JSON-based Web APIs. System config can be hard coded or saved to SD card, EEPROM, or Network share. Hydruino also comes with LCD and input controller support similar in operation to low-cost 3D printers ([support provided by tcMenu](https://github.com/davetcc/tcMenu)). We even made a custom 3D printed enclosure ([.stl's in extra folder](https://github.com/NachtRaveVL/Simple-Hydroponics-Arduino/tree/main/extra)) along with some other goodies.

Made primarily for Arduino microcontrollers, but should work with PlatformIO, ESP32/8266, Teensy, and others - although one might experience turbulence until the bug reports get ironed out. Unknown architectures must ensure `BUFFER_LENGTH` (or `I2C_BUFFER_LENGTH`) and `WIRE_INTERFACES_COUNT` are properly defined.

Dependencies include: Adafruit BusIO, Adafruit Unified Sensor, ArduinoJson, ArxContainer, ArxSmartPtr, Callback, DallasTemperature, DHT sensor library, EasyBuzzer, I2C_EEPROM, IoAbstraction, LiquidCrystalIO, OneWire, RTClib, SimpleCollections, TaskManagerIO (disableable), tcMenu (disableable), and Time.

TODO datasheet links /TODO

*If you value the work that we do, our small team always appreciates a subscription to our [Patreon](www.patreon.com/nachtrave).*

## Reasoning

In the most basic sense, automating crops is basically turning a bunch of relays on or off to push some dials up or down. This is not necessarily the most challenging of problems, as there exists a large variety of code projects out there that have tried their take on this same exact thing. Most barely had to hit 500+ lines of real honest code to arrive at a basic working setup.

You can pretty much do everything this controller does using the Cloud, a few IoT sensors, and some ingenuity - very little coding required. You can literally, right now, do everything this controller can, on a RasPi with similar little effort. In fact, several of these bigger hydroponic controller setups have existed for RasPi and others for quite some time now, with well established and tested code bases, full web interfaces, custom phone apps, and more.

Few have seen the value in putting to use the heaps of old, cheap, slow Arduinos that we all have laying around doing absolutely nothing. But as time has passed, the prices of MCUs have sharply risen. Add in a global chip manufacturing crisis, a pandemic, massive inflation, food shortages, etc., it then becomes a little more apparent that it's overkill to use a costly RasPi to turn a bunch of relays on and off.

For devices at scale in today's world, a cheap, readily available, reliable MCU can meet these new challenges. You don't need the shiny fast red sports car version when you're turning on and off switches. Be it Arduino, ESP32, RasPi Pico, or others, cheaper MCUs have exploded in popularity, and rightfully so. This project aims to take advantage of these while keeping costs down so that everyone can more easily access hydroponics.

## Controller Setup

### Installation

The easiest way to install this controller is to utilize the Arduino IDE library manager, or through a package manager such as PlatformIO. Otherwise, simply download this controller and extract its files into a `Simple-Hydroponics-Arduino` folder in your Arduino custom libraries folder, typically found in your `[My ]Documents\Arduino\libraries` folder (Windows), or `~/Documents/Arduino/libraries/` folder (Linux/OSX).

From there, you can make a local copy of one of the examples based on the kind of system setup you want to use. If you are unsure of which, we recommend the full system example, as it contains all the necessary code to make a fully configurable system without any code modification necessary. Other examples can also be used if a more lean install is desired.

### Header Defines

There are several defines inside of the controller's main header file that allow for more fine-tuned control of the controller. You may edit and uncomment these lines directly, or supply them via custom build flags. While editing the main header file isn't ideal, it is often easiest. Note that editing the controller's main header file directly will affect all projects compiled on your system using those modified controller files.

Alternatively, you may also refer to <https://forum.arduino.cc/index.php?topic=602603.0> on how to define custom build flags manually via modifying the platform[.local].txt file. Note that editing such directly will affect all other projects compiled on your system using those modified platform framework files, but at least you keep those changes to the same place.

From Hydroponics.h:
```Arduino
// Uncomment or -D this define to completely disable usage of any multitasking commands and libraries. Not recommended.
//#define HYDRUINO_DISABLE_MULTITASKING             // https://github.com/davetcc/TaskManagerIO

// Uncomment or -D this define to disable usage of tcMenu library, which will disable all GUI control. Not recommended.
//#define HYDRUINO_DISABLE_GUI                      // https://github.com/davetcc/tcMenu

// Uncomment or -D this define to enable debug output (treats Serial as attached to serial monitor).
//#define HYDRUINO_ENABLE_DEBUG_OUTPUT

// Uncomment or -D this define to disable debug assertions.
//#define HYDRUINO_DISABLE_DEBUG_ASSERTIONS
```

### Controller Initialization

There are several initialization mode settings exposed through this controller that are used for more fine-tuned control.

#### Class Instantiation

The controller's class object must first be instantiated, commonly at the top of the sketch where pin setups are defined (or exposed through some other mechanism), which makes a call to the controller's class constructor. The constructor allows one to set the module's XXX TODO, i2c Wire class instance, if on Espressif then i2c SDA pin and i2c SCL pin, and lastly i2c clock speed. The default constructor values of the controller, if left unspecified, is XXX TODO i2c Wire class instance `Wire` @`400k`Hz.

From Hydroponics.h, in class Hydroponics:
```Arduino
// TODO: Reinclude this example after modifications completed. -NR
```

## Hookup Callouts

* The recommended Vcc power supply and logic level is 5v.
  * There are some devices that may be 3.3v only and not 5v tolerant. Check your IC's datasheet for details.

### Serial UART

Serial UART uses individual communication lines for each device, with the receive `RX` pin of one being the transmit `TX` pin of the other - thus having to "flip wires" when connecting. However, devices can always be active and never have to share their access. UART runs at low to mid kHz speeds and is useful for simple device control, albeit somewhat clumsy at times.

* When wiring up modules that use Serial UART, make sure to flip `RX`/`TX` lines.
  * 3.3v devices that are not 5v tolerant (such as ESP8266 WiFi modules) may require a bi-directional logic level converter/shifter to access on 5v MCUs.
    * We have included a small breakout PCB ([gerbers in extra folder](https://github.com/NachtRaveVL/Simple-Hydroponics-Arduino/tree/main/extra)) to assist with hooking up such common WiFi and level shifter modules alongside one another.
    * Alternatively, hack a 1kΩ resistor between the MCU's TX pin and module's RX pin.

Serial UART Devices Supported: ESP8266 WiFi module (3.3v only)

### SPI Bus

SPI devices can be chained together on the same shared data lines (no flipping of wires), which are typically labeled `MOSI`, `MISO`, and `SCK` (often with an additional `SS`). Each SPI device requires its own individual cable-select `CS` wire as only one SPI device may be active at any given time - accomplished by pulling its `CS` line of that device low (aka active-low). SPI runs at MHz speeds and is useful for large data block transfers.

* The `CS` pin may be connected to any digital output pin, but it's common to use `SS` for the first device. Additional devices are not restricted to what pin they can or should use, but given it's a signal pin not using an interrupt-capable pin allows those to be used for interrupt driven mechanisms.

SPI Devices Supported: SD card modules (4MHz)

### I2C Bus

I2C (aka I²C, IIC, TwoWire, TWI) devices can be chained together on the same shared data lines (no flipping of wires), which are typically labeled `SCL` and `SDA`. Only different kinds of I2C devices can be used on the same data line together using factory default settings, otherwise manual addressing must be done. I2C runs at mid to high KHz speeds and is useful for advanced device control.

* When more than one I2C device of the same kind is to be used on the same data line, each device must be set to use a different address. This is accomplished via the A0-A2 (sometimes A0-A5) pins/pads on the physical device that must be set either open or closed (typically via a de-solderable resistor, or by shorting a pin/pad). Check your specific breakout's datasheet for details.
* Note that not all the I2C libraries used support multi-addressable I2C devices at this time. Currently, this restriction applies to RTC devices (read as: may only use one).

I2C Devices Supported: DS3231 RTC modules, AT24C* EEPROM modules, 16x2/20x4 LCD modules

### OneWire Bus

OneWire devices can be chained together on the same shared data lines (no flipping of wires). Devices can be of the same or different types, require minimal setup (and no soldering), and most can even operate in "parasite" power mode where they use the power from the data line (and an internal capacitor) to function (thus saving a `Vcc` line, only requiring `Data` and `GND`). OneWire runs only in the low kb/s speeds and is useful for digital sensors.

* Typically, sensors are limited to 20 devices along a maximum 100m of wire.
* When more than one OneWire device is on the same data data line, each device registers itself an enumeration index (0-X) along with an unique identifier (UUID). The device can then be referenced via this enumeration index (or UUID) by the system.

### Analog IO

* Ensure AREF pin is set to the correct max input voltage to ensure analog sensors can use their full input range. The AREF pin, by default, is the same voltage as the MCU.
  * Analog sensors will be able to customize per-pin voltage ranges in software later on - it's more important to know the max input voltage in use for correctly setting AREF.
  * Not setting this correctly may affect measured analog values. Some sensors we've tested, for instance, are calibrated to operate up to 5.5v direct from factory.
* The SAM/SAMD family of MCUs (e.g. Due, Zero, MKR, etc.) support different bit resolutions for analog/PWM pins, but also may limit how many pins are able to use these higher resolutions. See the datasheet on your MCU for details.

### Sensors

* If able to set in hardware, ensure pH and TDS meters use EC (mS/cm), aka PPM(500) or PPM(TDS). EC is considered a normalized measurement, while PPM can be split into different scale categories depending on sensor chemistry in use. PPM based sensors operating in a non-EC/PPM(500)/PPM(TDS)-based mode will need to have their PPM scale explicitly set.
* Many different kinds of hobbyist sensors label its analog output `AO` (or `Ao`), and OneWire data `DO` (or `Do`, `DQ`, `Dq`) - however, always check your specific sensor's datasheet.
  * Sensor pins used for event triggering when measurements go above/below a pre-set tolerance can be safely ignored, as the software implementation of this mechanism is more versatile.
* Some meters (such as pH, but also seen on others) may also have additional `To` (or `TO`) pin for an analog temperature output to use in sensor value refinement. These can be set up as additional analog water temperature sensors and then tied to the pH meter object later on.
* CO2 sensors are a bit unique - they require a 24 hour powered initialization period to burn off manufacturing chemicals, not to mention require `Vcc` for the heating element (5v @ 130mA for MQ-135) thus cannot use parasitic power mode. Then to calibrate, you have to set it outside while active until its voltage stabilizes, then calibrate its stabilized voltage to the current global known CO2 level.

We also ask that our users report any broken sensors (outside of bad calibration data) for us to consider adding support to (also consider sponsoring our work on [Patreon](www.patreon.com/nachtrave)).

## Memory Callouts

The total number of objects (sensors, pumps, relays, probes, etc.) that the controller can support at once depends on how much free memory your MCU has available, with larger memory scaling with cost. We're currently targeting Mega2560 at minimum at this time.

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

