# Hydruino
Hydruino: Simple Hydroponics Automation Controller.

**Simple-Hydroponics-Arduino v0.6.5.0**

Simple automation controller for hydroponic grow systems.  
Licensed under the non-restrictive MIT license.

Created by NachtRaveVL, May 20th, 2022.

**UNDER ACTIVE DEVELOPMENT -- WORK IN PROGRESS**

This controller allows one to set up a system of reservoirs, pumps, probes, relays, and other objects useful in automating the daily lighting, feed dosing, watering, and data monitoring & collection processes involved in hydroponically grown fruits, vegetables, teas, herbs, and salves. Works with a large variety of widely-available aquarium/hobbyist equipment, including popular GPS, RTC, EEPROM, SD card, WiFi, and other modules compatible with Arduino. Contains a large library of crop data to select from that will automatically aim the system for the best growing parameters during the various growth phases for the system configured, along with fully customizable weekly feed/additive amounts and daily feeding/lighting scheduling. With the right setup Hydruino can automatically do things like: enable grow lights for the needed period each day (potentially only turning on to augment daily sunlight hours), drive water pumps and auto-dosers during feedings, spray leafy plants in the morning before lights/sunrise, heat cold water to a specific temp for tropical plants, use CO2 sensors to manage air circulation fans to maintain optimal grow tent parameters, remind when to prune plants, or even use soil moisture sensing to dynamically determine watering schedule.

* Can be used entirely off-line with RTC module and optional GPS module (or known static location) for accurate sun angle measurements and sunrise/sunset timings, or used on-line through enabled on-board WiFi/Ethernet or external ESP-AT WiFi module.
  * Uses SolarCalculator Arduino library, based upon NOAA Solar Calculator, for fine calculations of the sun's solar position (accurate until 2100).
* Configured system setup can be saved/loaded to/from EEPROM, SD card, or WiFiStorage-like external storage device.
  * Can export/import in JSON for human-readability (allowing easy text editing), or in Binary for ultra-compactness/speed.
  * Auto-save, backup-auto-save (for auto-recovery), and low-disk cleanup (TODO) functionality.
  * Import functions are optimized with minimum spanning trie for ultra-fast text parsing.
* Supports interval-based sensor data publishing & system event data logging to MQTT IoT broker or to external storage in .csv/.txt format.
  * Can be extended to work with other JSON based Web APIs or Client-like derivatives (for DB storage or server-endpoint support).
  * Can add a piezo buzzer for audible system alerts (TODO).
* Library data can be built into onboard Flash or exported onto external storage to save on compiled build size.
  * Data export may allow enough space savings for certain 256kB Flash device builds.
* Actuator and Sensor I/O pins can be natively multiplexed or expanded through 8/16-bit i2c expanders (TODO) for pin-limited environments.
* Enabled GUI works with a large variety of common Arduino-compatible LCD/OLED/TFT displays, touchscreens, matrix keypads, analog joysticks, rotary encoders, and momentary buttons (support by tcMenu).
  * Contains at-a-glance system overview screen and UI menu system for system configuration, calibration, and more (TODO).
  * System configuration menus can be pin-coded to prevent tampering, thus still allowing informational/read-only menu access.
  * GUI I/O can be setup as fully interrupt driven (5-20ms) or polling based (50-100ms).
  * Can be built in Minimal mode, saving space at the cost of having to re-compile and re-upload upon system setup changes (i.e. a R/O UI), or Full mode, which uses large amounts of Flash space to provide everything at once (i.e. a R/W UI), with only pinout changes requiring rebuild.
  * Includes remote UI access through enabled Ethernet, WiFi, Serial, and/or Simhub connection.

Made primarily for Arduino microcontrollers / build environments, but should work with PlatformIO, Espressif, Teensy, STM32, Pico, and others - although one might experience turbulence until the bug reports get ironed out.

Dependencies include: Adafruit BusIO (dep of RTClib/tcMenu), Adafruit GPS Library (ext NMEA, optional), Adafruit Unified Sensor (dep of DHT), ArduinoJson, ArxContainer, ArxSmartPtr, DallasTemperature, DHT sensor library, I2C_EEPROM, IoAbstraction (dep of TaskManagerIO), OneWire, RTClib, SimpleCollections (dep of TaskManagerIO), a SD-like library, SolarCalculator, TaskManagerIO (disableable, dep of tcMenu), tcMenu (disableable), Time, and a WiFi-like library (optional): WiFi101 (MKR1000), WiFiNINA_Generic, WiFiEspAT (ext serial AT), or Ethernet.

GUI Dependencies (building with tcMenu) include: Adafruit FT6206 Library (disableable), Adafruit GFX Library, Adafruit PCD8544 Nokia 5110 LCD library, Adafruit ST7735 and ST7789 Library, LiquidCrystalIO, tcUnicodeHelper, TFT_eSPI, U8g2, and XPT2046_Touchscreen (optional).

Datasheet links include: [DS18B20 Temperature Sensor](https://github.com/NachtRaveVL/Simple-Hydroponics-Arduino/blob/main/extra/DS18B20.pdf), [DHT12 Air Temperature and Humidity Sensor](https://github.com/NachtRaveVL/Simple-Hydroponics-Arduino/blob/main/extra/dht12.pdf), [4502c Analog pH Sensor (writeup)](https://github.com/NachtRaveVL/Simple-Hydroponics-Arduino/blob/main/extra/ph-sensor-ph-4502c.pdf), but many more are available online.

*If you value the work that we do, our small team always appreciates a subscription to our [Patreon](www.patreon.com/nachtrave).*

## About

We want to make hydroponics more accessible to DIY'ers by utilizing the widely-available low-cost IoT and IoT-like microcontrollers (MCUs) of today.

With the advances in miniaturization technology bringing us even more compact MCUs at even lower costs, it becomes a lot more possible to simply use one of these small devices to do what amounts to turning a bunch of relays on and off in the right order as the day goes by. Hydroponics is a perfect application for these devices, especially as a data logger, process monitor, and more. Professional controller systems like this can cost hundreds to even thousands of dollars, but DIY systems can wind up being a fraction of that cost.

Hydruino is a MCU-based solution primarily written for Arduino and Arduino-like MCU devices. It allows one to throw together a bunch of hobbyist sensors and relays, some aquarium pumps, maybe some fancy lights, and other widely available low-cost hardware to build a functional DIY hydroponics controller system. Be it made with PVC from the hardware store or 3D printed at home, Hydruino opens the door for more people to get involved in reducing their carbon footprint, becoming more knowledgeable about their food and where it comes from - and hey, hopefully learning some basic electronics/coding along the way.

## Controller Setup

### MCU Requirements

Minimum MCU: 256-512kB Flash, 16-24kB SRAM, 16MHz  
Recommended: 512kB-1MB+ Flash, 24-32kB+ SRAM, 32-48MHz+

* Definitely will work: GIGA, Portenta, ESP32/8266, Teensy 3.5+, STM32 (>256kB Flash), Pico/Nano RP2040 Connect, etc.

* _Can_ work /w ext. data storage/min UI: Nano 33 (any), MKR (any), Due/Zero, Teensy 3.2, STM32 (256kB Flash)

* Definitely will ___not___ work: Uno (any), Nano (classic & Every), Leonardo/Duemilanove, Micro, Pro, Esplora, Teensy 2/LC, STM8/32 (<256kB Flash), etc.

* _May_ work, but only with heavy tweaking/limited build: ATMega2560, Genuino 101

Note: Certain MCUs, such as those from STM, are sold in many different Flash/SRAM size configurations. Some configurations may not be supported, others may limit total system size (i.e. object count, library support, features, etc.). Bigger is always better until you get a better idea of your specific use case's size requirements.

### Installation

The easiest way to install this controller is to utilize the Arduino IDE library manager, or through a package manager such as PlatformIO. Otherwise, simply download this controller and extract its files into a `Simple-Hydroponics-Arduino` folder in your Arduino custom libraries folder, typically found in your `[My ]Documents\Arduino\libraries` folder (Windows), or `~/Documents/Arduino/libraries/` folder (Linux/OSX).

From there, you can make a local copy of one of the example sketches based on the kind of system setup you want to use. If you are unsure of which, we recommend the Vertical NFT Example, as it is our standard implementation built for most common system setups and only requires changing setup defines at the top of the file.

Older storage constrained MCUs (< 512kB Flash) may need further tweaking, and possibly external storage hardware (such as EEPROM or SD Card - see the Data Writer example for more details). Modern MCUs with lots of Flash storage can instead simply build the Full System Example (TODO: Still a WIP).

### Setup

#### Header Defines

There are several defines inside of the controller's main Hydruino.h header file that allow for more fine-tuned control of the controller. You may edit and uncomment these lines directly, or supply them via custom build flags. While editing the main header file isn't ideal, it is often easiest. Note that editing the controller's main header file directly will affect all projects compiled on your system using those modified controller files.

Alternatively, you may also refer to <https://forum.arduino.cc/index.php?topic=602603.0> on how to define custom build flags manually via modifying the platform[.local].txt file, or with the Arduino CLI (preferred way going forward).

For the older platform.local.txt file override approach, create platform.local.txt alongside platform.txt located in %applocaldata%\Arduino15\packages\{platform}\hardware\{arch}\{version}\ (replacing %applocaldata%\Arduino15 with ~/Library/Arduino15 for OSX, and ~/.arduino15 for Linux), with the contents: `compiler.cpp.extra_flags=-Dname` (replacing `name` with full name of below define). Note that it will affect all builds for that platform until again changed/removed. Some build systems may require directly editing platform.txt and adding onto the end of its CPP build recipe, e.g. Teensy & `recipe.cpp.o.pattern=`.

From Hydruino.h:
```Arduino
// Uncomment or -D this define to completely disable usage of any multitasking commands and libraries. Not recommended.
//#define HYDRO_DISABLE_MULTITASKING              // https://github.com/davetcc/TaskManagerIO

// Uncomment or -D this define to disable usage of tcMenu library, which will disable all GUI control. Not recommended.
//#define HYDRO_DISABLE_GUI                       // https://github.com/davetcc/tcMenu

// Uncomment or -D this define to enable usage of the platform WiFi library, which enables networking capabilities.
//#define HYDRO_ENABLE_WIFI                       // https://reference.arduino.cc/reference/en/libraries/wifi/

// Uncomment or -D this define to enable usage of the external serial AT WiFi library, which enables networking capabilities.
//#define HYDRO_ENABLE_AT_WIFI                    // https://github.com/jandrassy/WiFiEspAT

// Uncomment or -D this define to enable usage of the platform Ethernet library, which enables networking capabilities.
//#define HYDRO_ENABLE_ETHERNET                   // https://reference.arduino.cc/reference/en/libraries/ethernet/

// Uncomment or -D this define to enable usage of the Arduino MQTT library, which enables IoT data publishing capabilities.
//#define HYDRO_ENABLE_MQTT                       // https://github.com/256dpi/arduino-mqtt

// Uncomment or -D this define to enable usage of the Adafruit GPS library, which enables GPS capabilities.
//#define HYDRO_ENABLE_GPS                        // https://github.com/adafruit/Adafruit_GPS

// Uncomment or -D this define to enable usage of the XPT2046_Touchscreen library, in place of the Adafruit FT6206 library.
//#define HYDRO_ENABLE_XPT2046TS                  // https://github.com/PaulStoffregen/XPT2046_Touchscreen

// Uncomment or -D this define to enable external data storage (SD card or EEPROM) to save on sketch size. Required for constrained devices.
//#define HYDRO_DISABLE_BUILTIN_DATA              // Disables library data existing in Flash, see DataWriter example for exporting details

// Uncomment or -D this define to enable debug output (treats Serial output as attached to serial monitor).
//#define HYDRO_ENABLE_DEBUG_OUTPUT

// Uncomment or -D this define to enable verbose debug output (note: adds considerable size to compiled sketch).
//#define HYDRO_ENABLE_VERBOSE_DEBUG

// Uncomment or -D this define to enable debug assertions (note: adds significant size to compiled sketch).
//#define HYDRO_ENABLE_DEBUG_ASSERTIONS
```

#### External Libraries

* TFT_eSPI: If using this graphical display library, user library setup via its TFT_eSPI\User_Setup.h library setup file is required.

### Initialization

There are several initialization mode settings exposed through this controller that are used for more fine-tuned control.

#### Class Instantiation

The controller's class object must first be instantiated, commonly at the top of the sketch where pin setups are defined (or exposed through some other mechanism), which makes a call to the controller's class constructor. The constructor allows one to set the module's various devices and how they are connected, with defaults providing no device specified.

From Hydruino.h, in class Hydruino:
```Arduino
    // Controller constructor. Typically called during class instantiation, before setup().
    Hydruino(pintype_t piezoBuzzerPin = -1,                         // Piezo buzzer pin, else -1
             Hydro_EEPROMType eepromType = Hydro_EEPROMType_None,   // EEPROM device type/size, else None
             DeviceSetup eepromSetup = DeviceSetup(),               // EEPROM device setup (i2c only)
             Hydro_RTCType rtcType = Hydro_RTCType_None,            // RTC device type, else None
             DeviceSetup rtcSetup = DeviceSetup(),                  // RTC device setup (i2c only)
             DeviceSetup sdSetup = DeviceSetup(),                   // SD card device setup (spi only)
             DeviceSetup netSetup = DeviceSetup(),                  // Network device setup (spi/uart)
             DeviceSetup gpsSetup = DeviceSetup(),                  // GPS device setup (uart/i2c/spi)
             pintype_t *ctrlInputPins = nullptr,                    // Control input pins, else nullptr
             DeviceSetup displaySetup = DeviceSetup());             // Display device setup (i2c/spi)
```

#### Controller Initialization

Additionally, a call is expected to be provided to the controller class object's `init[From…](…)` method, commonly called inside of the sketch's `setup()` function. This allows one to set the controller's system type (Recycling or DrainToWaste), units of measurement (Metric, Imperial, or Scientific), control input mode, and display output mode. The default mode of the controller, if left unspecified, is a Recycling system set to Metric units, without any input control or output display.

From Hydruino.h, in class Hydruino:
```Arduino
    // Initializes default empty system. Typically called near top of setup().
    // See individual enums for more info.
    void init(Hydro_SystemMode systemMode = Hydro_SystemMode_Recycling,                 // What system of crop feeding is performed
              Hydro_MeasurementMode measureMode = Hydro_MeasurementMode_Default,        // What units of measurement should be used
              Hydro_DisplayOutputMode dispOutMode = Hydro_DisplayOutputMode_Disabled,   // What display output mode should be used
              Hydro_ControlInputMode ctrlInMode = Hydro_ControlInputMode_Disabled);     // What control input mode should be used

    // Initializes system from EEPROM save, returning success flag
    // Set system data address with setSystemEEPROMAddress
    bool initFromEEPROM(bool jsonFormat = false);
    // Initializes system from SD card file save, returning success flag
    // Set config file name with setSystemConfigFilename
    bool initFromSDCard(bool jsonFormat = true);
#ifdef HYDRO_USE_WIFI_STORAGE
    // Initializes system from a WiFiStorage file save, returning success flag
    // Set config file name with setSystemConfigFilename
    bool initFromWiFiStorage(bool jsonFormat = true);
#endif
    // Initializes system from custom JSON-based stream, returning success flag
    bool initFromJSONStream(Stream *streamIn);
    // Initializes system from custom binary stream, returning success flag
    bool initFromBinaryStream(Stream *streamIn);
```

The controller can also be initialized from a saved configuration, such as from an EEPROM or SD card, or other JSON or Binary stream. A saved configuration of the system can be made via the controller class object's `saveTo…(…)` methods, or called automatically on timer by setting an Autosave mode/interval.

From Hydruino.h, in class Hydruino:
```Arduino
    // Saves current system setup to EEPROM save, returning success flag
    // Set system data address with setSystemEEPROMAddress
    bool saveToEEPROM(bool jsonFormat = false);
    // Saves current system setup to SD card file save, returning success flag
    // Set config file name with setSystemConfigFilename
    bool saveToSDCard(bool jsonFormat = true);
#ifdef HYDRO_USE_WIFI_STORAGE
    // Saves current system setup to WiFiStorage file save, returning success flag
    // Set config file name with setSystemConfigFilename
    bool saveToWiFiStorage(bool jsonFormat = true);
#endif
    // Saves current system setup to custom JSON-based stream, returning success flag
    bool saveToJSONStream(Stream *streamOut, bool compact = true);
    // Saves current system setup to custom binary stream, returning success flag
    bool saveToBinaryStream(Stream *streamOut);
```

### Event Logging & Data Publishing

The controller can, after initialization, be set to produce logs and data files that can be further used by other applications. Log entries are timestamped and can keep track of when feedings are performed, when devices enable/disable, etc., while data files can be read into plotting applications or exported to a database for further processing. The passed file prefix is typically the subfolder that such files should reside under and is appended with the year, month, and date (in YYMMDD format).

Note: You can also get the same logging output sent to the Serial device by defining `HYDRO_ENABLE_DEBUG_OUTPUT`, described above in Header Defines.

Note: Files on FAT32-based SD cards are limited to 8 character file/folder names and a 3 character extension.

From Hydruino.h, in class Hydruino:
```Arduino
    // Enables data logging to the SD card. Log file names will append YYMMDD.txt to the specified prefix. Returns success flag.
    inline bool enableSysLoggingToSDCard(String logFilePrefix = "logs/hy");

    // Enables data publishing to the SD card. Log file names will append YYMMDD.csv to the specified prefix. Returns success flag.
    inline bool enableDataPublishingToSDCard(String dataFilePrefix = "data/hy");
```

## Hookup Callouts

* The recommended Vcc power supply and logic level is 5v, with most newer MCUs restricted to 3.3v.
  * There are many devices that are 3.3v only and not 5v tolerant. Check your IC's datasheet for details.
* Devices that do not have the same logic level voltage as the MCU will need a level converter (or similar), bi-directional particularly on any fast data lines, in order to operate together (unless capable of doing so without such).
  * 5v analog sensor signal data lines, for example, will need level converted in order to connect to a 3.3v MCU, as the pins will not be 5v tolerant (this includes `AREF`).
  * Alternatively, using a 10kΩ resistor can often times be enough to 'convert' 5v to 3.3v, but the correct way is to use a 1kΩ resistor and a 2kΩ resistor (or any size with a 1:2 ratio) in a [simple voltage divider circuit](https://randomnerdtutorials.com/how-to-level-shift-5v-to-3-3v/).
* OneWire sensor's logic level voltage is the same as their Vcc supply voltage. Most of the time this isn't an issue, but some (such as CO2 sensors) may require 5v Vcc supply power and thus need their data line converted in order to connect to a 3.3v MCU.

### Serial UART

Serial UART uses individual communication lines for each device, with the receive `RX` pin of one being the transmit `TX` pin of the other - thus having to "flip wires" when connecting. However, devices can always be active and never have to share their access. UART runs at low to mid kHz speeds and is useful for simple device control, albeit somewhat clumsy at times.

* When wiring up modules that use Serial UART, make sure to flip `RX`/`TX` lines.
  * 5v devices interacting with 3.3v devices that are not 5v tolerant (such as [serial ESP WiFi modules](http://www.instructables.com/id/Cheap-Arduino-WiFi-Shield-With-ESP8266/)) will require a bi-directional logic level converter/shifter to utilize.
    * Alternatively, hack a single 10kΩ resistor ([but preferably two of any 1:2 ratio](https://randomnerdtutorials.com/how-to-level-shift-5v-to-3-3v/)) between the 5v module's TX pin and 3.3v module's RX pin.

Serial UART Devices Supported: AT WiFi modules, NMEA GPS modules

### SPI Bus

SPI devices can be chained together on the same shared data lines, which are typically labeled `COPI` (or `MOSI`), `CIPO` (or `MISO`), and `SCK`, often with an additional `CS` (or `SS`). Each SPI device requires its own individual cable-select `CS` wire as only one SPI device may be active at any given time - accomplished by pulling its `CS` line of that device low (aka active-low). SPI runs at MHz speeds and is useful for large data block transfers.

* The `CS` pin may be connected to any digital output pin, but it's common to use the `CS` (or `SS`) pin for the first device. Additional devices are not restricted to what pin they can or should use, but given it's not a data pin not using a choice interrupt-capable pin allows those to be used for interrupt driven mechanisms.
* Many low-cost SPI-based SD card modules on market only read SDHC sized SD cards (2GB to 32GB) formatted in FAT32 (filenames limited to 8 characters plus 3 character file extension).
  * Some SD cards simply will not play nicely with these modules and you may have to try another SD card manufacturer. We recommend 32GB SD cards due to overall lowest cost (smaller SD cards actually becoming _more_ expensive).
* Many various graphical displays have an additional `DC` (or `RS`) pin, which is required to be connected to any digital pin in addition to its `CS` pin.
  * There is also an additional `Reset` pin for many of these, but is optional.

SPI Devices Supported: SD card modules, NMEA GPS modules, 128x32 to 296x128 OLED displays, 320x240+ LCD/TFT gfx displays, XPT touchscreens

### I2C Bus

I2C (aka I²C, IIC, TwoWire, TWI) devices can be chained together on the same shared data lines (no flipping of wires), which are typically labeled `SCL` and `SDA`. Only different kinds of I2C devices can be used on the same data line together using factory default settings, otherwise manual addressing must be done. I2C runs at mid to high kHz speeds and is useful for advanced device control.

* When more than one I2C device of the same kind is to be used on the same data line, each device must be set to use a different address. This is accomplished via the A0-A2 (sometimes A0-A5) pins/pads on the physical device that must be set either open or closed (typically via a de-solderable resistor, or by shorting a pin/pad). Check your specific breakout's datasheet for details.
* Note that not all the I2C libraries used support multi-addressable I2C devices at this time (read as: may only use one). Currently, this restriction applies to: RTC devices.

I2C Devices Supported: DS*/PCF* RTC modules, AT24C* EEPROM modules, NMEA GPS modules, 16x2/20x4 LCD displays, 128x32 to 296x128 OLED displays, FT touchscreens

### OneWire Bus

OneWire devices can be chained together on the same shared data lines (no flipping of wires). Devices can be of the same or different types, require minimal setup (and no soldering), and most can even operate in "parasite" power mode where they use the power from the data line (and an internal capacitor) to function (thus saving a `Vcc` line, only requiring `Data` and `GND`). OneWire runs only in the low kb/s speeds and is useful for digital sensors.

* Typically, sensors are limited to 20 devices along a maximum 100m of wire.
* When more than one OneWire device is on the same device line, each device registers itself an enumeration index (0 - N) along with its own 64-bit unique identifier (UUID, with last byte being CRC). The device can then be referenced via this UUID by the system in the future indefinitely, or enumeration index so long as the device doesn't change its line position.

OneWire Devices Supported: DHT* air temp/humidity sensors, DS* water temp sensors

### Analog IO

* All analog sensors will need to have the same operational voltage range. Many analog sensors are set to use 0v to 5v by default, but some can go -5v to +5v, some even up to 5.5v.
* The `AREF` (or `IOREF`) pin, by default, is the same voltage as the MCU. Analog sensors must not exceed this voltage limit.
  * 5v analog sensor signals **must** be [level converted](https://randomnerdtutorials.com/how-to-level-shift-5v-to-3-3v/) in order to connect to 3.3v MCUs.
* The SAM/SAMD family of MCUs (e.g. Due, Zero, MKR, etc.) as well as the RasPi Pico and others support different bit resolutions for analog/PWM pins. Refer to the datasheet of your MCU for details.

### Sensors

* If able to set in hardware, ensure any TDS meters and soil moisture sensors use EC (aka mS/cm) mode. EC is considered a normalized measurement while PPM can be split into different scale categories depending on sensor chemistry in use (often tied to country of origin). PPM/EC based sensors operating on anything other than a 1v=500ppm/1v=1EC mode will need to have their PPM scale explicitly set.
* Many different kinds of hobbyist sensors label their analog output `AO` (or `Ao`) - however, always check your specific sensor's datasheet.
  * Again, make sure all analog sensors are calibrated to output the same 0v - `AREF` volts in range.
* Sensor pins used for event triggering when measurements go above/below a pre-set tolerance - many of which are deceptively labeled `DO` (or `Do`), despite having nothing to do with being `D`ata lines of any kind - can be safely ignored, as the software implementation of such mechanism is more than sufficient.
  * Often these pins are used to drive other hardware-only based solutions that aren't a part of Hydruino's use case.
* CO2 sensors are a bit unique - they require a 24 hour powered initialization period to burn off manufacturing chemicals, and _require_ `Vcc` for its heating element (5v @ 130mA for MQ-135) thus cannot use OneWire parasitic power mode. To calibrate, you have to set it outside while active until its voltage stabilizes, then calibrate its stabilized voltage to the current global known CO2 level.

We also ask that our users report any broken sensors (outside of bad calibration data) for us to consider adding support to (also consider sponsoring our work on [Patreon](www.patreon.com/nachtrave)).

### WiFi

* Devices with built-in WiFi can enable such through header defines while other devices can utilize an external [serial ESP WiFi module](http://www.instructables.com/id/Cheap-Arduino-WiFi-Shield-With-ESP8266/).

## Memory Callouts

* The total number of objects and different kinds of objects (reservoirs, pumps, probes, relays, etc.) that the controller can support at once depends on how much free Flash storage and SRAM your MCU has available. Hydruino C++0x11 objects range in memory usage size from 150 to 750 bytes or more depending on settings and object type, with the compiled Flash binary ranging in size from 100kB to 500kB+ depending on platform and settings.
  * For our supported microcontroller range, on the low end we have devices with 256kB of Flash and at least 16kB of SRAM, while on the upper end we have more modern devices with 1MB+ of Flash and 32kB+ of SRAM. Devices with < 24kB of SRAM may struggle with system builds and may be limited to minimal system setups (such as no WiFi, no data publishing, no built-in library data, only minimal UI, etc.), while other newer devices with more capacity build with everything enabled.
* For AVR, SAM, and other build architectures that do not have C++0x11 STL (standard container library) support, there are a series of *`_MAXSIZE` defines at the top of `HydroDefines.h` that can be modified to adjust how much memory space is allocated for the various static array structures the controller instead uses.
* To save on the cost of code size for constrained devices, focus on not enabling that which you won't need, which has the benefit of being able to utilize code stripping to remove sections of code that don't get used.
  * There are also header defines that can strip out certain libraries and functionality, such as ones that disable the UI, multi-tasking subsystems, etc.
* To further save on code size cost, see the Data Writer Example on how to externalize library data onto an SD Card or EEPROM.
  * Note: Upgrading between versions or changing custom/program data may require you to re-build and re-deploy to such external device.

## Example Usage

Below are several examples of controller usage.

### Simple Deep Water Culture (DWC) System Example

DWC setups are great for beginners and for crops that do not flower, and has the advantage of being able to be built out of commonly available plastic containers. Aeration is important in this setup to oxygenate the non-circulating water.

The Simple DWC Example sketch shows how a simple Hydruino system can be setup using the most minimal of work. In this sketch only that which you actually use is built into the final compiled binary, making it an ideal lean choice for those who don't need anything fancy. This sketch has no UI or input control, but with a simple buzzer and some additional sensors the system can alert you to when the feed is low and more is needed, or when pH is too high, etc.

```Arduino
#include <Hydruino.h>

#define SETUP_PIEZO_BUZZER_PIN          -1              // Piezo buzzer pin, else -1
#define SETUP_GROW_LIGHTS_PIN           8               // Grow lights relay pin (digital)
#define SETUP_WATER_AERATOR_PIN         7               // Aerator relay pin (digital)
#define SETUP_FEED_RESERVOIR_SIZE       5               // Reservoir size, in default measurement units
#define SETUP_AC_POWER_RAIL_TYPE        AC110V          // Rail power type used for AC rail (AC110V, AC220V)

#define SETUP_CROP_TYPE                 Lettuce         // Type of crop planted, else Undefined
#define SETUP_CROP_SUBSTRATE            ClayPebbles     // Type of crop substrate
#define SETUP_CROP_SOW_DATE             DateTime(2022, 5, 21) // Date that crop was planted

Hydruino hydroController(SETUP_PIEZO_BUZZER_PIN);       // Controller using default setup aside from buzzer pin, if defined

void setup() {
    // Setup base interfaces
    #ifdef HYDRO_ENABLE_DEBUG_OUTPUT
        Serial.begin(115200);           // Begin USB Serial interface
        while (!Serial) { ; }           // Wait for USB Serial to connect
    #endif

    // Initializes controller with default environment, no logging, eeprom, SD, or anything else.
    hydroController.init();

    // DWC systems tend to require less feed, so we can tell the system feeding scheduler that our feeding rates should reflect such.
    hydroController.scheduler.setBaseFeedMultiplier(0.5);

    // Adds a simple relay power rail using standard AC. This will manage how many active devices can be turned on at the same time.
    auto relayPower = hydroController.addSimplePowerRail(JOIN(Hydro_RailType,SETUP_AC_POWER_RAIL_TYPE));

    // Adds a main water reservoir of SETUP_FEED_RESERVOIR_SIZE size, treated as already being filled with water.
    auto feedReservoir = hydroController.addFeedWaterReservoir(SETUP_FEED_RESERVOIR_SIZE, true);

    // Adds a water aerator at SETUP_WATER_AERATOR_PIN, and links it to the feed water reservoir and the relay power rail.
    auto aerator = hydroController.addWaterAeratorRelay(SETUP_WATER_AERATOR_PIN);
    aerator->setParentRail(relayPower);
    aerator->setParentReservoir(feedReservoir);

    // Add grow lights relay at SETUP_GROW_LIGHTS_PIN, and links it to the feed water reservoir and the relay power rail.
    auto lights = hydroController.addGrowLightsRelay(SETUP_GROW_LIGHTS_PIN);
    lights->setParentRail(relayPower);
    lights->setParentReservoir(feedReservoir);

    // Add timer fed crop set to feed on a standard 15 mins on/45 mins off timer, and links it to the feed water reservoir.
    auto crop = hydroController.addTimerFedCrop(JOIN(Hydro_CropType,SETUP_CROP_TYPE),
                                                JOIN(Hydro_SubstrateType,SETUP_CROP_SUBSTRATE),
                                                SETUP_CROP_SOW_DATE);
    crop->setFeedReservoir(feedReservoir);

    // Launches controller into main operation.
    hydroController.launch();
}

void loop()
{
    // Hydruino will manage most updates for us.
    hydroController.update();
}

```

### Main System Examples

There are two main system examples to choose from, Vertical NFT and Full System, each with its own requirements and capabilities. The Vertical NFT Example is the recommended starting point for most system builders, and is perfect for those with only basic programming knowledge. It is also the standard implementation for our 3D printed controller enclosure. It can be easily extended to include other functionality if desired, simply by copying and pasting the example code.

The Vertical NFT Example sketch has the benefit of being able to compile in a minimal UI mode that will strip out what isn't used, making it ideal for storage constrained devices (e.g. those with < 512kB Flash), but will not provide full UI functionality since it will be missing the code for all the other objects the system build code strips out, thus requiring re-compiling/re-uploading on system setup changes. The UI, in this mode, only provides edit capabilities, not create/delete, with more customization options locked out.

By contrast, the Full System Example sketch will build an empty system with all object and system features enabled. It is recommended for modern MCUs that have lots of storage space to use such as ESP32, RasPi Pico, etc. It works similarly to the Vertical NFT Example, except is meant for systems where a GUI (possibly only remotely controlled) will be primarily used to create objects with. It involves the least amount of coding, but comes at the highest cost since it has to contain everything that possibly could be used, even if it ultimately isn't. The UI, in this mode, provides edit, create, and delete capabilities, with more options available for customization.

Included below is the default system setup defines of the Vertical NFT example (of which a smaller similar version is used for the Full System example) to illustrate a variety of the controller features. This is not an exhaustive list of course, as there are many more things the controller is capable of, as documented in its main header file include, GitHub Project Wiki, and elsewhere.

```Arduino
// Pins & Class Instances
#define SETUP_PIEZO_BUZZER_PIN          -1              // Piezo buzzer pin, else -1
#define SETUP_EEPROM_DEVICE_TYPE        None            // EEPROM device type/size (AT24LC01, AT24LC02, AT24LC04, AT24LC08, AT24LC16, AT24LC32, AT24LC64, AT24LC128, AT24LC256, AT24LC512, None)
#define SETUP_EEPROM_I2C_ADDR           0b000           // EEPROM i2c address
#define SETUP_RTC_I2C_ADDR              0b000           // RTC i2c address (only 0b000 can be used atm)
#define SETUP_RTC_DEVICE_TYPE           None            // RTC device type (DS1307, DS3231, PCF8523, PCF8563, None)
#define SETUP_SD_CARD_SPI               SPI             // SD card SPI class instance
#define SETUP_SD_CARD_SPI_CS            -1              // SD card CS pin, else -1
#define SETUP_SD_CARD_SPI_SPEED         F_SPD           // SD card SPI speed, in Hz (ignored on Teensy)
#define SETUP_DISP_LCD_I2C_ADDR         0b111           // LCD i2c address
#define SETUP_DISP_GFX_I2C_ADDR         0b000           // Gfx i2c address
#define SETUP_DISP_SPI                  SPI             // Gfx/TFT SPI class instance
#define SETUP_DISP_SPI_CS               -1              // Gfx/TFT SPI CS pin, else -1
#define SETUP_CTRL_INPUT_PINS           {(pintype_t)-1} // Control input pins, else {-1}
#define SETUP_I2C_WIRE                  Wire            // I2C wire class instance
#define SETUP_I2C_SPEED                 400000U         // I2C speed, in Hz
#define SETUP_ESP_I2C_SDA               SDA             // I2C SDA pin, if on ESP
#define SETUP_ESP_I2C_SCL               SCL             // I2C SCL pin, if on ESP

// WiFi Settings                                        (note: define HYDRO_ENABLE_WIFI or HYDRO_ENABLE_AT_WIFI to enable WiFi)
// #include "secrets.h"                                 // Pro-tip: Put sensitive password information into a custom secrets.h
#define SETUP_WIFI_SSID                 "CHANGE_ME"     // WiFi SSID
#define SETUP_WIFI_PASS                 "CHANGE_ME"     // WiFi passphrase
#define SETUP_WIFI_SPI                  SPIWIFI         // WiFi SPI class instance, if using spi
#define SETUP_WIFI_SPI_CS               SPIWIFI_SS      // WiFi CS pin, if using spi
#define SETUP_WIFI_SERIAL               Serial1         // WiFi serial class instance, if using serial

// Ethernet Settings                                    (note: define HYDRO_ENABLE_ETHERNET to enable Ethernet)
#define SETUP_ETHERNET_MAC              { (uint8_t)0xDE, (uint8_t)0xAD, (uint8_t)0xBE, (uint8_t)0xEF, (uint8_t)0xFE, (uint8_t)0xED } // Ethernet MAC address
#define SETUP_ETHERNET_SPI              SPI1            // Ethernet SPI class instance
#define SETUP_ETHERNET_SPI_CS           SS1             // Ethernet CS pin

// GPS Settings                                         (note: define HYDRO_ENABLE_GPS to enable GPS)
#define SETUP_GPS_TYPE                  None            // Type of GPS (UART, I2C, SPI, None)
#define SETUP_GPS_SERIAL                Serial1         // GPS serial class instance, if using serial
#define SETUP_GPS_I2C_ADDR              0b000           // GPS i2c address, if using i2c
#define SETUP_GPS_SPI                   SPI             // GPS SPI class instance, if using spi
#define SETUP_GPS_SPI_CS                SS              // GPS CS pin, if using spi

// System Settings
#define SETUP_SYSTEM_MODE               Recycling       // System run mode (Recycling, DrainToWaste)
#define SETUP_MEASURE_MODE              Default         // System measurement mode (Default, Imperial, Metric, Scientific)
#define SETUP_DISPLAY_OUT_MODE          Disabled        // System display output mode (Disabled, LCD16x2_EN, LCD16x2_RS, LCD20x4_EN, LCD20x4_RS, SSD1305, SSD1305_x32Ada, SSD1305_x64Ada, SSD1306, SH1106, SSD1607_GD, SSD1607_WS, IL3820, IL3820_V2, ST7735, ST7789, ILI9341, PCD8544, TFT)
#define SETUP_CONTROL_IN_MODE           Disabled        // System control input mode (Disabled, RotaryEncoderOk, RotaryEncoderOkLR, UpDownButtonsOk, UpDownButtonsOkLR, UpDownESP32TouchOk, UpDownESP32TouchOkLR, AnalogJoystickOk, Matrix2x2UpDownButtonsOkL, Matrix3x4Keyboard_OptRotEncOk, Matrix3x4Keyboard_OptRotEncOkLR, Matrix4x4Keyboard_OptRotEncOk, Matrix4x4Keyboard_OptRotEncOkLR, ResistiveTouch, TouchScreen, TFTTouch, RemoteControl)
#define SETUP_SYS_UI_MODE               Minimal         // System user interface mode (Disabled, Minimal, Full)
#define SETUP_SYS_NAME                  "Hydruino"      // System name
#define SETUP_SYS_TIMEZONE              +0              // System timezone offset, in hours (int or float)
#define SETUP_SYS_LOGLEVEL              All             // System log level filter (All, Warnings, Errors, None)
#define SETUP_SYS_STATIC_LAT            DBL_UNDEF       // System static latitude (if not using GPS/UI, else DBL_UNDEF), in degrees
#define SETUP_SYS_STATIC_LONG           DBL_UNDEF       // System static longitude (if not using GPS/UI, else DBL_UNDEF), in minutes
#define SETUP_SYS_STATIC_ALT            DBL_UNDEF       // System static altitude (if not using GPS/UI, else DBL_UNDEF), in meters above sea level (msl)

// System Saves Settings                                (note: only one primary and one fallback mechanism may be enabled at a time)
#define SETUP_SAVES_CONFIG_FILE         "hydruino.cfg"  // System config file name for system saves
#define SETUP_SAVES_SD_CARD_MODE        Disabled        // If saving/loading from SD card is enable (Primary, Fallback, Disabled)
#define SETUP_SAVES_EEPROM_MODE         Disabled        // If saving/loading from EEPROM is enabled (Primary, Fallback, Disabled)
#define SETUP_SAVES_WIFISTORAGE_MODE    Disabled        // If saving/loading from WiFiStorage (OS/OTA filesystem / WiFiNINA_Generic only) is enabled (Primary, Fallback, Disabled)

// Logging & Data Publishing Settings
#define SETUP_LOG_FILE_PREFIX           "logs/hy"       // System logs file prefix (appended with YYMMDD.txt)
#define SETUP_DATA_FILE_PREFIX          "data/hy"       // System data publishing files prefix (appended with YYMMDD.csv)
#define SETUP_DATA_SD_ENABLE            false           // If system data publishing is enabled to SD card
#define SETUP_LOG_SD_ENABLE             false           // If system logging is enabled to SD card
#define SETUP_DATA_WIFISTORAGE_ENABLE   false           // If system data publishing is enabled to WiFiStorage (OS/OTA filesystem / WiFiNINA_Generic only)
#define SETUP_LOG_WIFISTORAGE_ENABLE    false           // If system logging is enabled to WiFiStorage (OS/OTA filesystem / WiFiNINA_Generic only)

// MQTT Settings                                        (note: define HYDRO_ENABLE_MQTT to enable MQTT)
#define SETUP_MQTT_BROKER_CONNECT_BY    Hostname        // Which style of address broker uses (Hostname, IPAddress)
#define SETUP_MQTT_BROKER_HOSTNAME      "hostname"      // Hostname that MQTT broker exists at
#define SETUP_MQTT_BROKER_IPADDR        { (UINT8_T)192, (UINT8_T)168, (UINT8_T)1, (UINT8_T)2 } // IP address that MQTT broker exists at
#define SETUP_MQTT_BROKER_PORT          1883            // Port number that MQTT broker exists at

// External Data Settings
#define SETUP_EXTDATA_SD_ENABLE         false           // If data should be read from an external SD card (searched first for crops lib data)
#define SETUP_EXTDATA_SD_LIB_PREFIX     "lib/"          // Library data folder/data file prefix (appended with {type}##.dat)
#define SETUP_EXTDATA_EEPROM_ENABLE     false           // If data should be read from an external EEPROM (searched first for strings data)

// External EEPROM Settings
#define SETUP_EEPROM_SYSDATA_ADDR       0x2222          // System data memory offset for EEPROM saves (from Data Writer output)
#define SETUP_EEPROM_CROPSLIB_ADDR      0x0000          // Start address for Crops Library data (from Data Writer output)
#define SETUP_EEPROM_STRINGS_ADDR       0x1111          // Start address for strings data (from Data Writer output)

// UI Settings
#define SETUP_UI_LOGIC_LEVEL            ACT_LOW         // I/O signaling logic active level (ACT_LOW, ACT_HIGH)
#define SETUP_UI_ALLOW_INTERRUPTS       true            // Allow interrupt driven I/O if able, else force polling
#define SETUP_UI_USE_UNICODE_FONTS      true            // Use tcUnicode fonts instead of default, if using graphical display
#define SETUP_UI_IS_DFROBOTSHIELD       false           // Using DFRobotShield as preset (SETUP_CTRL_INPUT_PINS may be left {-1})

// UI Display Output Settings
#define SETUP_UI_LCD_BACKLIGHT_MODE     Normal          // LCD display backlight mode (Normal, Inverted, PWM), if using LCD
#define SETUP_UI_GFX_DISP_ORIENTATION   R0              // Display orientation (R0, R1, R2, R3, HorzMirror, VertMirror), if using graphical display
#define SETUP_UI_GFX_DC_PIN             -1              // SPI display interface DC pin, if using SPI-based display
#define SETUP_UI_GFX_RESET_PIN          -1              // Optional reset pin, if using graphical display, else -1
#define SETUP_UI_GFX_ST7735_TAB         Undefined       // ST7735 tab color (Green, Red, Black, Green144, Mini160x80, Hallowing, Mini160x80_Plugin, Undefined), if using ST7735
#define SETUP_UI_TFT_SCREEN_WIDTH       320             // Custom screen width, if using TFT_eSPI
#define SETUP_UI_TFT_SCREEN_HEIGHT      240             // Custom screen height, if using TFT_eSPI

// UI Control Input Settings
#define SETUP_UI_ENC_ROTARY_SPEED       HalfCycle       // Rotary encoder cycling speed (FullCycle, HalfCycle, QuarterCycle)
#define SETUP_UI_KEY_REPEAT_SPEED       20              // Key repeat speed, in ticks
#define SETUP_UI_KEY_REPEAT_DELAY       750             // Key repeat delay, in milliseconds
#define SETUP_UI_KEY_REPEAT_INTERVAL    350             // Key repeat interval, in milliseconds
#define SETUP_UI_JS_ACCELERATION        3.0f            // Joystick acceleration (decrease divisor)
#define SETUP_UI_ESP32TOUCH_SWITCH      800             // ESP32 Touch key switch threshold, if on ESP32/using ESP32Touch
#define SETUP_UI_ESP32TOUCH_HVOLTS      V_2V7           // ESP32 Touch key high reference voltage (Keep, V_2V4, V_2V5, V_2V6, V_2V7, Max), if on ESP32/using ESP32Touch
#define SETUP_UI_ESP32TOUCH_LVOLTS      V_0V5           // ESP32 Touch key low reference voltage (Keep, V_0V5, V_0V6, V_0V7, V_0V8, Max), if on ESP32/using ESP32Touch
#define SETUP_UI_ESP32TOUCH_HVATTEN     V_1V            // ESP32 Touch key high ref voltage attenuation (Keep, V_1V5, V_1V, V_0V5, V_0V, Max), if on ESP32/using ESP32Touch

// UI Remote Control Settings
#define SETUP_UI_REMOTE1_TYPE           Disabled        // Type of first remote control (Disabled, Serial, Simhub, WiFi, Ethernet)
#define SETUP_UI_REMOTE1_UART           Serial1         // Serial setup for first remote control, if Serial/Simhub
#define SETUP_UI_REMOTE2_TYPE           Disabled        // Type of second remote control (Disabled, Serial, Simhub, WiFi, Ethernet)
#define SETUP_UI_REMOTE2_UART           Serial1         // Serial setup for second remote control, if Serial/Simhub
#define SETUP_UI_RC_NETWORKING_PORT     3333            // Remote controller networking port, if WiFi/Ethernet

// Device Pin Setup
#define SETUP_PH_METER_PIN              -1              // pH meter sensor pin (analog), else -1
#define SETUP_TDS_METER_PIN             -1              // TDS meter sensor pin (analog), else -1
#define SETUP_CO2_SENSOR_PIN            -1              // CO2 meter sensor pin (analog), else -1
#define SETUP_AC_USAGE_SENSOR_PIN       -1              // AC power usage meter sensor pin (analog), else -1
#define SETUP_DC_USAGE_SENSOR_PIN       -1              // DC power usage meter sensor pin (analog), else -1
#define SETUP_FLOW_RATE_SENSOR_PIN      -1              // Main feed pump flow rate sensor pin (analog/PWM), else -1
#define SETUP_DS18_WATER_TEMP_PIN       -1              // DS18* water temp sensor data pin (digital), else -1
#define SETUP_DHT_AIR_TEMP_HUMID_PIN    -1              // DHT* air temp sensor data pin (digital), else -1
#define SETUP_DHT_SENSOR_TYPE           None            // DHT sensor type enum (DHT11, DHT12, DHT21, DHT22, AM2301, None)
#define SETUP_VOL_FILLED_PIN            -1              // Water level filled indicator pin (digital/ISR), else -1
#define SETUP_VOL_EMPTY_PIN             -1              // Water level empty indicator pin (digital/ISR), else -1
#define SETUP_VOL_INDICATOR_TYPE        ACT_HIGH        // Water level indicator type/active level (ACT_HIGH, ACT_LOW)
#define SETUP_VOL_LEVEL_PIN             -1              // Water level sensor pin (analog)
#define SETUP_VOL_LEVEL_TYPE            Ultrasonic      // Water level device type (Ultrasonic, AnalogHeight)
#define SETUP_GROW_LIGHTS_PIN           -1              // Grow lights relay pin (digital), else -1
#define SETUP_WATER_AERATOR_PIN         -1              // Aerator relay pin (digital), else -1
#define SETUP_FEED_PUMP_PIN             -1              // Water level low indicator pin, else -1
#define SETUP_WATER_HEATER_PIN          -1              // Water heater relay pin (digital), else -1
#define SETUP_WATER_SPRAYER_PIN         -1              // Water sprayer relay pin (digital), else -1
#define SETUP_FAN_EXHAUST_PIN           -1              // Fan exhaust relay pin (digital/PWM), else -1
#define SETUP_NUTRIENT_MIX_PIN          -1              // Nutrient premix peristaltic pump relay pin (digital), else -1
#define SETUP_FRESH_WATER_PIN           -1              // Fresh water peristaltic pump relay pin (digital), else -1
#define SETUP_PH_UP_PIN                 -1              // pH up solution peristaltic pump relay pin (digital), else -1
#define SETUP_PH_DOWN_PIN               -1              // pH down solution peristaltic pump relay pin (digital), else -1
#define SETUP_CROP_SOILM_PIN            -1              // Soil moisture sensor pin, for adaptive crop

// Device Multiplexing Setup
#define SETUP_MUXING_CHANNEL_BITS       -1              // Number of channel bits for multiplexer, else -1
#define SETUP_MUXING_ADDRESS_PINS       {(pintype_t)-1} // Address channel pins, else {-1}
#define SETUP_MUXING_ENABLE_PIN         -1              // Chip enable pin for multiplexer (optional), else -1
#define SETUP_MUXING_ENABLE_TYPE        ACT_LOW         // Chip enable pin type/active level (ACT_HIGH, ACT_LOW)
#define SETUP_PH_METER_MUXCHN           -1              // pH meter sensor pin muxing channel #, else -1
#define SETUP_TDS_METER_MUXCHN          -1              // TDS meter sensor pin muxing channel #, else -1
#define SETUP_CO2_SENSOR_MUXCHN         -1              // CO2 meter sensor pin muxing channel #, else -1
#define SETUP_AC_USAGE_SENSOR_MUXCHN    -1              // AC power usage meter sensor pin muxing channel #, else -1
#define SETUP_DC_USAGE_SENSOR_MUXCHN    -1              // DC power usage meter sensor pin muxing channel #, else -1
#define SETUP_FLOW_RATE_SENSOR_MUXCHN   -1              // Main feed pump flow rate sensor pin muxing channel #, else -1
#define SETUP_VOL_FILLED_MUXCHN         -1              // Water level filled indicator pin muxing channel #, else -1
#define SETUP_VOL_EMPTY_MUXCHN          -1              // Water level empty indicator pin muxing channel #, else -1
#define SETUP_VOL_LEVEL_MUXCHN          -1              // Water level sensor pin muxing channel #, else -1
#define SETUP_GROW_LIGHTS_MUXCHN        -1              // Grow lights relay pin muxing channel #, else -1
#define SETUP_WATER_AERATOR_MUXCHN      -1              // Aerator relay pin muxing channel #, else -1
#define SETUP_FEED_PUMP_MUXCHN          -1              // Water level low indicator pin muxing channel #, else -1
#define SETUP_WATER_HEATER_MUXCHN       -1              // Water heater relay pin muxing channel #, else -1
#define SETUP_WATER_SPRAYER_MUXCHN      -1              // Water sprayer relay pin muxing channel #, else -1
#define SETUP_FAN_EXHAUST_MUXCHN        -1              // Fan exhaust relay pin muxing channel #, else -1
#define SETUP_NUTRIENT_MIX_MUXCHN       -1              // Nutrient premix peristaltic pump relay pin muxing channel #, else -1
#define SETUP_FRESH_WATER_MUXCHN        -1              // Fresh water peristaltic pump relay pin muxing channel #, else -1
#define SETUP_PH_UP_MUXCHN              -1              // pH up solution peristaltic pump relay pin muxing channel #, else -1
#define SETUP_PH_DOWN_MUXCHN            -1              // pH down solution peristaltic pump relay pin muxing channel #, else -1
#define SETUP_CROP_SOILM_MUXCHN         -1              // Soil moisture sensor pin muxing channel #, for adaptive crop

// System Setup
#define SETUP_AC_POWER_RAIL_TYPE        AC110V          // Rail power type used for actuator AC rail (AC110V, AC220V)
#define SETUP_DC_POWER_RAIL_TYPE        DC12V           // Rail power type used for actuator DC rail (DC3V3, DC5V, DC12V, DC24V, DC48V)
#define SETUP_AC_SUPPLY_POWER           0               // Maximum AC supply power wattage, else 0 if not known (-> use simple rails)
#define SETUP_DC_SUPPLY_POWER           0               // Maximum DC supply power wattage, else 0 if not known (-> use simple rails)
#define SETUP_FEED_RESERVOIR_SIZE       5               // Reservoir size, in default measurement units
#define SETUP_FEED_PUMP_FLOWRATE        20              // The base continuous flow rate of the main feed pumps, in L/min
#define SETUP_PERI_PUMP_FLOWRATE        0.070           // The base continuous flow rate of any peristaltic pumps, in L/min
#define SETUP_CROP_ON_TIME              15              // Minutes feeding pumps are to be turned on for (per feeding cycle)
#define SETUP_CROP_OFF_TIME             45              // Minutes feeding pumps are to be turned off for (per feeding cycle)
#define SETUP_CROP_TYPE                 Lettuce         // Type of crop planted, else Undefined
#define SETUP_CROP_SUBSTRATE            ClayPebbles     // Type of crop substrate, else Undefined
#define SETUP_CROP_NUMBER               1               // Number of plants in crop position (aka averaging weight)
#define SETUP_CROP_SOW_DATE             DateTime(2022, 5, 21) // Date that crop was planted at
#define SETUP_CROP_SOILM_FEED_LEVEL     0.1             // Soil moisture feeding trigger needs feeding level, for adaptive crop, in %
#define SETUP_CROP_SOILM_FED_LEVEL      0.75            // Soil moisture feeding trigger fed level, for adaptive crop, in %
```

### Data Writer Example

The Data Writer Example can be used on the same system setup to offload all exportable data, such as crop and string data, onto a connected external SD card or EEPROM storage device to help storage constrained MCUs compile system builds.

This Example doesn't actually run the Hydruino controller in full, but a code stripped version of it that easily compiles with all extra data built-in. The compiled binary in this Example will include all exportable data stored as compact JSON string text, and typically easily fits in under 256kB compilation size.

This data can further be exported into smaller binary chunks with native endianness for EEPROM storage, or into expanded pretty print JSON text files for SD card storage. You can also further customize default and/or extend library data (such as adding custom crops) that will be included in data export.

Inside of the Data Writer's `setup()` function:
```Arduino
    // Just a lone initializer is all that's needed since we won't actually be using the full controller.
    hydroController.init();

    // Right here would be the place to program in any custom crop data that you want made available for later.
    //HydroCropsLibData customCrop1(Hydro_CropType_CustomCrop1);
    //strncpy(customCrop1.cropName, "Custom name", HYDRO_NAME_MAXSIZE);
    //hydroCropsLib.setUserCropData(&customCrop1);
```

If you are planning to utilize an external EEPROM storage device and have made any custom data modifications, you will have to update your system's sketch to include the proper offsets that the output of the Data Writer sketch produces. These defines can be copied over and overwrite the existing ones in the main system examples.

In serial monitor (near end):
```
…
2022-08-07T04:24:27 [INFO] Writing String: #365 "W"
2022-08-07T04:24:27 [INFO] ... to byte offset: 11854 (0x2e4e)
2022-08-07T04:24:27 [INFO] Wrote: 2 bytes
2022-08-07T04:24:27 [INFO] Successfully wrote: 4908 bytes
2022-08-07T04:24:27 [INFO] Total EEPROM usage: 11856 bytes
2022-08-07T04:24:27 [INFO] EEPROM capacity used: 36.18% of 32768 bytes
2022-08-07T04:24:27 [INFO] Use the following EEPROM setup defines in your sketch:
#define SETUP_EEPROM_SYSDATA_ADDR       0x2222
#define SETUP_EEPROM_CROPSLIB_ADDR      0x0000
#define SETUP_EEPROM_STRINGS_ADDR       0x1111
2022-08-07T04:24:27 [INFO] Done!
```

Note: Again, you can get logging output sent to the Serial device by defining `HYDRO_ENABLE_DEBUG_OUTPUT`, described above in Header Defines.
