# Hydruino
Hydruino: Simple Hydroponics Automation Controller.

**Simple-Hydroponics-Arduino v0.3**

Simple automation controller for hydroponic grow systems using an Arduino board.  
Licensed under the non-restrictive MIT license.

Created by NachtRaveVL, May 20th, 2022.

**UNDER ACTIVE DEVELOPMENT BUT DON'T EXPECT ANY MIRACLES**

This controller allows one to set up an entire system of sensors, pumps, relays, probes, and other things useful in automating the lighting, feeding, watering, and sensor data monitoring & collection process involved in hydroponically grown fruits, vegetables, teas, herbs, and salves. It contains a large library of crop data to select from that will automatically aim the system for the best growing parameters during the various growth phases with the hardware you have available. Crop library data can be built into onboard Flash, or stored externally, along with config and user calibration data, on a SD card or EEPROM device. Works with a large variety of common aquarium equipment and hobbyist sensors. Supports sensor data logging to MQTT (for IoT integration), .csv data files on an SD card or Network share, and can be extended to work with other JSON-based Web APIs or WiFiServer-like derivatives. Hydruino also comes with basic LCD support, or with advanced LCD and input controller support similar in operation to low-cost 3D printers - ([provided by tcMenu](https://github.com/davetcc/tcMenu)). We even made some ([custom 3D printed stuff](https://github.com/NachtRaveVL/Simple-Hydroponics-Arduino/wiki/Extra-Goodies-Supplied)) along with some other goodies.

Made primarily for Arduino microcontrollers, but should work with PlatformIO, ESP32/8266, Teensy, Pico, and others - although one might experience turbulence until the bug reports get ironed out. Unknown architectures must ensure `BUFFER_LENGTH` (or `I2C_BUFFER_LENGTH`) and `WIRE_INTERFACES_COUNT` are properly defined.

Dependencies include: Adafruit BusIO (dep of RTClib), Adafruit Unified Sensor (dep of DHT), ArduinoJson, ArxContainer, ArxSmartPtr, Callback, DallasTemperature, DHT sensor library, EasyBuzzer, I2C_EEPROM, IoAbstraction (dep of TaskManager), LiquidCrystalIO (dep of TaskManager), OneWire, RTClib, SimpleCollections (dep of TaskManager), TaskManagerIO (disableable, dep of tcMenu), tcMenu (disableable), and Time.

Datasheet links include: [DS18B20 Temperature Sensor](https://github.com/NachtRaveVL/Simple-Hydroponics-Arduino/blob/main/extra/DS18B20.pdf), [DHT12 Air Temperature and Humidity Sensor](https://github.com/NachtRaveVL/Simple-Hydroponics-Arduino/blob/main/extra/dht12.pdf), [4502c Analog pH Sensor (writeup)](https://github.com/NachtRaveVL/Simple-Hydroponics-Arduino/blob/main/extra/ph-sensor-ph-4502c.pdf), but many more are available online.

*If you value the work that we do, our small team always appreciates a subscription to our [Patreon](www.patreon.com/nachtrave).*

## About

We want to make Hydroponics more accessible by utilizing the widely available IoT and IoT-like microcontrollers (MCUs) of both today and yesterday.

With the advances in technology bringing us even more compact MCUs at even lower costs, it becomes a lot more possible to simply use one of these small devices to do what amounts to turning a bunch of relays on and off in the right order. Hydroponics is a perfect application for these devices, especially as a data logger, feed balancer, and more.

Hydruino is an MCU-based solution primarily written for Arduino and Arduino-like MCU devices. It allows one to throw together a bunch of hobbyist sensors from the hobby store, some aquarium pumps from the pet store, and other widely available low-cost hardware to build a working functional hydroponics controller systems. Be it made with PVC from the hardware store or 3D printed at home, Hydruino opens the doors for more people to get involved in reducing their carbon footprint and becoming more knowledgeable about their food.

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

// Uncomment or -D this define to disable building-in of Crops Library data (note: saves considerable size on sketch). Required for constrained devices.
//#define HYDRUINO_DISABLE_BUILT_IN_CROPS_LIBRARY   // If enabled, must use external device (such as SD Card or EEPROM) for Crops Library support.

// Uncomment or -D this define to enable debug output (treats Serial as attached to serial monitor).
//#define HYDRUINO_ENABLE_DEBUG_OUTPUT

// Uncomment or -D this define to enable debug assertions (note: adds considerable size to sketch).
//#define HYDRUINO_ENABLE_DEBUG_ASSERTIONS
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

SPI Devices Supported: SD card modules (4+MHz)

### I2C Bus

I2C (aka I²C, IIC, TwoWire, TWI) devices can be chained together on the same shared data lines (no flipping of wires), which are typically labeled `SCL` and `SDA`. Only different kinds of I2C devices can be used on the same data line together using factory default settings, otherwise manual addressing must be done. I2C runs at mid to high KHz speeds and is useful for advanced device control.

* When more than one I2C device of the same kind is to be used on the same data line, each device must be set to use a different address. This is accomplished via the A0-A2 (sometimes A0-A5) pins/pads on the physical device that must be set either open or closed (typically via a de-solderable resistor, or by shorting a pin/pad). Check your specific breakout's datasheet for details.
* Note that not all the I2C libraries used support multi-addressable I2C devices at this time. Currently, this restriction applies to RTC devices (read as: may only use one).

I2C Devices Supported: DS3231 RTC modules, AT24C* EEPROM modules, 16x2/20x4 LCD modules

### OneWire Bus

OneWire devices can be chained together on the same shared data lines (no flipping of wires). Devices can be of the same or different types, require minimal setup (and no soldering), and most can even operate in "parasite" power mode where they use the power from the data line (and an internal capacitor) to function (thus saving a `Vcc` line, only requiring `Data` and `GND`). OneWire runs only in the low kb/s speeds and is useful for digital sensors.

* Typically, sensors are limited to 20 devices along a maximum 100m of wire.
* When more than one OneWire device is on the same data data line, each device registers itself an enumeration index (0-X) along with a 64-bit unique identifier (UUID, with last byte being CRC). The device can then be referenced via this UUID (or enumeration index) by the system in the future.

### Analog IO

* Ensure AREF pin is set to the correct max input voltage to ensure analog sensors can use their full input range. The AREF pin, by default, is the same voltage as the MCU.
  * Analog sensors will be able to customize per-pin voltage ranges in software later on (as long as voltages are not negative) - it's more important to know the max input voltage in use for correctly setting AREF.
  * Not setting this correctly may affect measured analog values. Some sensors we've tested, for instance, are calibrated to operate up to 5.5v direct from factory, others operate from -5v to +5v and have to be modified to output 0-5v.
* The SAM/SAMD family of MCUs (e.g. Due, Zero, MKR, etc.) support different bit resolutions for analog/PWM pins, but also may limit how many pins are able to use these higher resolutions. See the datasheet of your MCU for details.

### Sensors

* If able to set in hardware, ensure any TDS meters and soil moisture sensors use EC (aka mS/cm) mode. EC is considered a normalized measurement, while PPM can be split into different scale categories depending on sensor chemistry in use. PPM based sensors operating on anything other than a 1v=500ppm mode will need to have their PPM scale explicitly set.
* Many different kinds of hobbyist sensors label their analog output `AO` (or `Ao`) - however, always check your specific sensor's datasheet.
* Sensor pins used for event triggering when measurements go above/below a pre-set tolerance can be safely ignored, as the software implementation of this mechanism is more versatile.
* CO2 sensors are a bit unique - they require a 24 hour powered initialization period to burn off manufacturing chemicals, not to mention require `Vcc` for the heating element (5v @ 130mA for MQ-135) thus cannot use parasitic power mode. Then to calibrate, you have to set it outside while active until its voltage stabilizes, then calibrate its stabilized voltage to the current global known CO2 level.

We also ask that our users report any broken sensors (outside of bad calibration data) for us to consider adding support to (also consider sponsoring our work on [Patreon](www.patreon.com/nachtrave)).

## Memory Callouts

* The total number of and different types of objects (sensors, pumps, relays, etc.) that the controller can support at once depends on how much free Flash storage and RAM your MCU has available. Objects range in size from 150 to 350 bytes or more depending on settings.
* For our target microcontroller range, on the low end we have ATMega2560 with 256kB of Flash and 8kB of RAM, while more recent devices like the RasPi Pico have 2MB of Flash and 264kB of RAM. The ATMega2560 may struggle with full system builds and may be limited to specific system setups (such as no debug assertions, external crop library, only minimal UI, etc.), while other newer devices with more capacity build with everything enabled without issue.
* For AVR, SAM/SAMD, and other architectures that do not have C++ STL (standard container) support, there are a series of *`_MAXSIZE` defines at the top of `HydroponicsDefines.h` that can be modified to adjust how much memory space is allocated for the various array structures the controller uses.
  * If, for example, you had a large number of crops attached to a single feed reservoir, you can modify these defines to give more space for such object storage to avoid running into any storage limitations.
* To save on the cost of code for constrained devices, focus on not enabling that which you won't need, which has the benefit of being able to utilize code stripping to remove sections of code that don't get used (e.g. WiFi not being included in the build if you don't actually use any WiFi).
  * There are also header defines that can strip out certain libraries and functionality, such as ones that disable the UI, multi-tasking subsystems, etc.
* See the Crop Writer Example to see how to externalize the Crops Library onto an SD Card or EEPROM.

## Example Usage

Below are several examples of controller usage.

### Simple Deep Water Culture (DWC) System Example

The Simple DWC Example sketch shows how a simple Hydruino system can be setup using the most minimal of work. In this sketch only that what you need is built into the final binary, making it an ideal lean choice for those who don't need anything fancy.

```Arduino
// TODO: Reinclude this example after modifications completed. -NR
```

### Vertical Nutrient Film Technique (NFT) System Example

The Vertical NFT Example sketch is the standard implementation for our 3D printed controller enclosure and for most vertical towers that will be used. It can be easily extended to include other functionality if desired.

```Arduino
// TODO: Reinclude this example after modifications completed. -NR
```

### Other Examples

The Full System Example sketch will build an empty system with all object and system features enabled. It works similarly to the Vertical NFT Example, except is meant for systems where a UI will be used to create the objects. It involves the least amount of coding and setup, but comes at the highest cost.

The Crops Writer Example sketch can be used to write the Crops Library data onto an SD Card or EEPROM so that storage constrained devices, such as the ATMega2560, can still build at least the Vertical NFT system.
