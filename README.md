# Hydruino
Hydruino: Simple Hydroponics Automation Controller.

**Simple-Hydroponics-Arduino v0.7.2.0**

Simple automation controller for hydroponic grow systems.  
Licensed under the non-restrictive MIT license.

Created by NachtRaveVL, May 20th, 2022.

This project is part of a four-library controller family: **Simple-Hydroponics-Arduino (Hydruino)**, **Simple-SolarTracker-Arduino (Helioduino)**, **Simple-Homestead-Arduino (Terraduino)**, and **Simple-AstroTracker-Arduino (Astruino)**.

This controller manages reservoirs, pumps, probes, relays, lighting, dosing, watering, and data collection for hydroponic grow systems. It supports common Arduino-compatible RTC, GPS, EEPROM, SD, networking, sensor, and display hardware while keeping networking completely optional. The built-in crop library provides growth-stage timing and environmental targets, and timed crops can water a configured number of times per day, number of times per week, or after an elapsed interval.

The Keep-It-Simple controller system:

* Can be used entirely offline with RTC module and optional GPS module (or known static location) for accurate time keeping, or used online through enabled on-board WiFi/Ethernet or external ESP-AT WiFi module.
  * Uses [SolarCalculator](https://github.com/jpb10/SolarCalculator), inspired by the NOAA Solar Calculator, for fine offline calculations of the sun's solar position (including sunrise, sunset, & transit times), accurate until 2100.
* Exportable system configuration to EEPROM, SD card, or WiFiStorage external storage device.
  * Saved in pretty-print JSON for human-readability & easy text editing, or in raw binary for compactness & speed.
  * Auto-save, backup auto-save (for auto-recovery), and low external storage space cleanup (TODO) functionality.
  * Import string decode functions are pre-optimized with minimum spanning trie for ultra-fast text parsing & reduced loading times.
* Supports interval-based sensor data publishing and system event logging to MQTT IoT broker (for further IoT-integrated processing) or to external storage in .csv/.txt format (/w date in filename, segmented daily).
  * Can be extended to work with other JSON-based Web APIs or Client-like derivatives (for DB storage or server-endpoint support).
  * Can add a piezo buzzer for audible system warning/failure alerting (TODO), or a LCD/OLED/TFT display for current readings & recent logging messages (TODO).
* Enabled GUI works with a large variety of common Arduino-compatible LCD/OLED/TFT displays, touchscreens, matrix keypads, analog joysticks, rotary encoders, and momentary buttons (support by [tcMenu](https://github.com/davetcc/tcMenuLib)).
  * Contains at-a-glance system overview screen and interactive menu system for system configuration, sensor calibration, and more (TODO).
  * Critical system config menus can be pin-coded to prevent setup tampering.
  * Includes remote GUI menu access through enabled WiFi, Ethernet, Bluetooth, Serial, and/or Simhub connection via tcMenu's excellent [embedCONTROL](https://github.com/davetcc/tcMenu/releases) desktop application, available for Linux/OSX/Windows.
  * GUI I/O pins can be setup as fully interrupt driven (5-25ms latency), partially interrupt driven (only keys & buttons polled), or polling based (75-100ms+ latency), and can be automatically selected depending on pins used.
  * System examples can be compiled in:
    * Disabled UI mode, which removes all GUI code entirely, freeing a large amount of Flash size for constrained (<=256kB Flash) devices.
    * Minimal UI mode, which saves on compiled sketch size through optimized code stripping at the cost of having to modify/re-upload a new sketch to change most system settings (or to change system object structure).
    * Full UI mode, which uses large amounts of Flash space available on modern MCUs to provide everything all at once, with only major system (or static linked component) changes requiring a sketch modify/re-upload.
* Actuator & Sensor pins can be multiplexed or expanded along with any control input pins through 8/16-bit i2c expanders for pin-limited controllers.
* Library data can be built into onboard Flash or exported onto external storage to additionally save on compiled sketch size.

Designed primarily for Arduino and Arduino-compatible build environments. PlatformIO, Espressif, Teensy, STM32, Pico, and other supported toolchains may also be used.

Datasheet links include: [DS18B20 Temperature Sensor](https://github.com/NachtRaveVL/Simple-Hydroponics-Arduino/blob/main/extra/DS18B20.pdf), [DHT12 Air Temperature and Humidity Sensor](https://github.com/NachtRaveVL/Simple-Hydroponics-Arduino/blob/main/extra/dht12.pdf), [4502c Analog pH Sensor (writeup)](https://github.com/NachtRaveVL/Simple-Hydroponics-Arduino/blob/main/extra/ph-sensor-ph-4502c.pdf), but many more are available online.

*If this work is useful, project support is always appreciated through [Patreon](www.patreon.com/nachtrave).*

## About

The goal is to make hydroponics more accessible to DIY builders by using widely available, low-cost microcontrollers (MCUs).

Modern low-cost MCUs provide enough processing power, memory, and I/O to monitor reservoirs and sensors, manage pumps and dosing equipment, schedule watering and lighting, and record system data. Hydroponics is a strong fit for these devices as a local controller, data logger, and process monitor. Commercial controller systems can cost hundreds or thousands of dollars, while DIY systems can be built for substantially less.

Hydruino is written primarily for Arduino and Arduino-compatible MCUs. It combines sensors, relays, pumps, lights, reservoirs, and other widely available low-cost hardware into a functional DIY hydroponics automation system. The physical implementation remains open to the builder.

## Controller Setup

### MCU Requirements

There is no single minimum MCU for every Hydruino build because enabled UI, networking, logging, sensor counts, reservoir counts, and automation complexity can change the program and memory requirements considerably.

As a practical starting point:

Minimum planning target: 256–512kB Flash, 16–24kB SRAM, 16MHz+

Recommended: 512kB–1MB+ Flash, 24–32kB+ SRAM, 32–48MHz+

Modern 32-bit boards such as Pico RP2040/RP2350, ESP32, Teensy 3.5+, STM32, GIGA, and Portenta-class devices are the natural starting point when automation, logging, UI, and networking are expected to run together.

Hydruino systems may need to service numerous sensors, pumps, dosing equipment, reservoirs, and timed processes concurrently. Sensor polling, actuator response, control-loop activity, display load, logging, and communication traffic can therefore matter more than Flash size alone when selecting the MCU.

### Installation

Installation through the Arduino IDE Library Manager or a package manager such as PlatformIO is the simplest option. Manual installation consists of extracting the library into a `Simple-Hydroponics-Arduino` directory under the Arduino custom libraries directory, typically `[My ]Documents\Arduino\libraries` on Windows or `~/Documents/Arduino/libraries/` on Linux/macOS.

The Simple DWC Example is the recommended starting point because it is the smallest practical system. The Vertical NFT Example is the larger integrated reference.

Storage-constrained MCUs (< 512kB Flash, particularly <= 256kB) may require smaller feature sets, adjusted max-size defines, or external EEPROM/SD storage; see the Data Writer Example. Modern MCUs with larger Flash and SRAM can enable more of the controller at once.

### Crop Data and Watering

Hydruino includes 77 built-in crop profiles. Growth cycle and phase durations are stored in weeks, daily light values are hours per day, `tdsRange` stores EC in mS/cm, water and air temperatures are Celsius, and CO2 targets are PPM. These profiles are useful starting points and should still be tuned for the cultivar, lighting, substrate, and system being used.

Timer-fed crops keep watering duration separate from watering cadence. Cadence can be set with `setFeedingsPerDay()`, `setFeedingsPerWeek()`, or `setFeedInterval()`. Existing JSON on/off timing data is still loaded as an equivalent elapsed cycle.

### Host Tests

Host-side tests can be run with CMake:

```sh
cmake -S tests -B build-host
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

### Setup

#### Header Defines

Several defines inside the controller's main `Hydruino[UI].h` header file provide fine-grained control over optional features and build behavior. These may be edited directly or supplied through custom build flags. Editing the main header is often the simplest approach, but affects every project compiled against that modified library.

Custom build flags can also be supplied through the Arduino CLI or the older `platform.local.txt` override approach. See <https://forum.arduino.cc/index.php?topic=602603.0> for additional details.

For the older `platform.local.txt` override, create `platform.local.txt` alongside `platform.txt` in `%applocaldata%\Arduino15\packages\{platform}\hardware\{arch}\{version}\` (replace `%applocaldata%\Arduino15` with `~/Library/Arduino15` on macOS or `~/.arduino15` on Linux) and add `compiler.cpp.extra_flags=-Dname`, replacing `name` with the required define. This affects all builds for that platform until changed or removed. Some build systems, including Teensy, may instead require editing `platform.txt` and appending the define to the C++ build recipe.

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

// Uncomment or -D this define to enable external data storage (SD card or EEPROM) to save on sketch size. Required for constrained devices.
//#define HYDRO_DISABLE_BUILTIN_DATA              // Disables library data existing in Flash, see DataWriter example for exporting details

// Uncomment or -D this define to enable debug output (treats Serial output as attached to serial monitor, waiting on start for connection).
//#define HYDRO_ENABLE_DEBUG_OUTPUT

// Uncomment or -D this define to enable verbose debug output (note: adds considerable size to compiled sketch).
//#define HYDRO_ENABLE_VERBOSE_DEBUG

// Uncomment or -D this define to enable debug assertions (note: adds significant size to compiled sketch).
//#define HYDRO_ENABLE_DEBUG_ASSERTIONS
```

From shared/HydruinoUI.h:
```Arduino
// Uncomment or -D this define to enable usage of the XPT2046_Touchscreen library, in place of the Adafruit FT6206 library.
//#define HYDRO_UI_ENABLE_XPT2046TS               // https://github.com/PaulStoffregen/XPT2046_Touchscreen

// Uncomment or -D this define to enable usage of the StChromaArt LDTC framebuffer capable canvas in place of default U8g2Drawable canvas (STM32/mbed only, note: requires advanced setup)
//#define HYDRO_UI_ENABLE_STCHROMA_LDTC

// Uncomment or -D this define to enable usage of the StChromaArt BSP touch screen interrogator in place of the default AdaLibTouchInterrogator (STM32/mbed only, note: requires advanced setup, see tcMenu_Extra_BspUserSettings.h)
//#define HYDRO_UI_ENABLE_BSP_TOUCH

// Uncomment or -D this define to enable usage of the debug menu 
//#define HYDRO_UI_ENABLE_DEBUG_MENU
```

#### External Libraries

Hydruino uses the following controller-side libraries depending on the enabled hardware and features:

* **ArduinoJson** for JSON configuration data.
* **ArxContainer** and **ArxSmartPtr** for container and shared-pointer support on Arduino targets.
* **DallasTemperature** and **OneWire** for DS18-series temperature sensors.
* **DHT sensor library** and **Adafruit Unified Sensor** for DHT environmental sensors.
* **I2C_EEPROM** for external I2C EEPROM storage.
* **RTClib** and **Time** for RTC and system time handling.
* **SolarCalculator** for offline solar position, sunrise, sunset, and transit calculations.
* **TaskManagerIO**, **IoAbstraction**, and **SimpleCollections** for multitasking and I/O support when multitasking is enabled.
* **Adafruit GPS** when GPS support is enabled.
* **MQTT** when MQTT publishing is enabled.
* **SD** plus the platform SPI/Wire support for local storage and buses.
* **WiFi101**, **WiFiNINA_Generic**, **WiFiEspAT**, or **Ethernet** when the matching optional network path is enabled.

Networking is optional. An offline Hydruino system does not need a WiFi, Ethernet, or MQTT library.

#### External UI Libraries

The optional tcMenu UI layer can use the following display and input libraries as required by the selected hardware:

* **tcMenu** for the menu, remote-control, and display abstraction layer.
* **Adafruit GFX**, **Adafruit ILI9341**, and **Adafruit ST7735 and ST7789 Library** for supported color displays.
* **Adafruit FT6206**, **Adafruit TouchScreen**, and optional **XPT2046_Touchscreen** for touch input.
* **LiquidCrystalIO** for character LCD displays.
* **U8g2** for monochrome OLED and LCD displays.
* **TFT_eSPI** for supported advanced TFT configurations.
* **tcUnicodeHelper** for Unicode-capable tcMenu display paths.

* **U8g2** custom display setups use the selected U8g2 device class and are statically linked to that display configuration.
* **TFT_eSPI** uses its `TFT_eSPI\User_Setup.h` configuration and therefore requires a rebuild when that hardware setup changes.
* **BSP LCD / BSP Touch** support can use the included ChromaArt/BSP adapter layer on supported STM32/mbed targets. This is an advanced hardware-specific path.
* **ST7789 custom TFT / TFT_eSPI** setups use statically configured screen dimensions and require a rebuild when those values change.

### Initialization

There are several initialization mode settings exposed through this controller that are used for more fine-tuned control.

#### Class Instantiation

Instantiate the controller object before `setup()`, typically near the sketch's pin and device configuration. The constructor accepts optional hardware-device setup values; defaults select no external devices.

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

Call the controller object's `init[From…](…)` method from `setup()` to initialize a new system or load a saved configuration. For a new system, `init()` selects the system mode, measurement mode, control-input mode, and display-output mode. Defaults select a Recycling system using the default measurement units with control input and display output disabled.

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

After initialization, the controller can write timestamped system logs and sensor data for external analysis. Log entries record controller events, while data files can be imported into plotting tools or databases. File prefixes are typically used as subfolders and are appended with the date in `YYMMDD` format.

Serial logging output can also be enabled with `HYDRO_ENABLE_DEBUG_OUTPUT`, described above under Header Defines.

FAT32-based SD cards use 8.3 filenames, limiting file/folder names to eight characters plus a three-character extension.

From Hydruino.h, in class Hydruino:
```Arduino
    // Enables data logging to the SD card. Log file names will append YYMMDD.txt to the specified prefix. Returns success flag.
    inline bool enableSysLoggingToSDCard(String logFilePrefix = "logs/hy");

    // Enables data publishing to the SD card. Log file names will append YYMMDD.csv to the specified prefix. Returns success flag.
    inline bool enableDataPublishingToSDCard(String dataFilePrefix = "data/hy");
```

## Hookup Callouts

Many of the various electronic components and systems this controller is designed to work with may have specific setup procedures and/or wiring requirements. While advanced users may find this section a refresher at best, the below callouts are highlighted in order to help prevent device damage and ensure proper controller operation.

### General

* The recommended Vcc power supply and logic level is 5v, with most newer MCUs restricted to 3.3v.
  * There are many devices that are 3.3v only and not 5v tolerant. Check your IC's datasheet for details.
* 5v device output pins that interface with any 3.3v device input pins that are not 5v tolerant (such as a 5v AVR interfacing with a 3.3v-only [serial ESP-AT WiFi module](http://www.instructables.com/id/Cheap-Arduino-WiFi-Shield-With-ESP8266/), or a 3.3v MCU interfacing with a 5v analog sensor), will require a bi-directional logic level converter/shifter to use, especially for any high-speed digital data transfer lines.
  * Alternatively, using a 10kΩ resistor can often times be enough to 'convert' 5v to 3.3v, but the correct way is to utilize a 1kΩ resistor and a 2kΩ resistor (or any size with a 1:2 ratio) in a [simple voltage divider circuit](https://randomnerdtutorials.com/how-to-level-shift-5v-to-3-3v/).
* OneWire sensor's logic level voltage is typically the same as their Vcc supply voltage. Most of the time this isn't an issue, but some (such as CO2 sensors) may require 5v Vcc supply power and thus need their data line converted in order to connect to a 3.3v MCU.

### Serial UART

Serial UART uses individual communication lines for each device, with the receive `RX` pin of one being the transmit `TX` pin of the other - thus having to "flip wires" when connecting. However, devices can always be active and never have to share their access. UART runs at low to mid kHz speeds and is useful for simple device control, albeit somewhat clumsy at times.

* When wiring up modules that use Serial UART, make sure to flip `RX`/`TX` pins.
* Always ensure that any data output pins and data input pins have compatible voltages.

Serial UART Devices Supported: Bluetooth-AT modules, ESP-AT WiFi modules, NMEA-AT GPS modules

### SPI Bus

SPI devices can be chained together on the same shared data lines, which are typically labeled `COPI` (or `MOSI`), `CIPO` (or `MISO`), and `SCK`, often with an additional `CS` (or `SS`). Each SPI device requires its own individual cable-select `CS` wire as only one SPI device may be active at any given time - accomplished by pulling its `CS` line of that device low (aka active-low). SPI runs at MHz speeds and is useful for large data block transfers.

* The `CS` pin may be connected to any digital output pin, but it's common to use the `CS` (or `SS`) pin for the first device. Additional devices are not restricted to what pin they can or should use, but given it's not a data pin not using a choice interrupt-capable pin allows those to be used for interrupt driven mechanisms.
* Many low-cost SPI-based SD card modules on market only read SDHC sized SD cards (2GB to 32GB) formatted in FAT32 (filenames limited to 8 characters plus 3 character file extension).
  * Some SD cards simply will not play nicely with these modules and you may have to try another SD card manufacturer. We recommend 32GB SD cards due to overall lowest cost (smaller SD cards actually becoming _more_ expensive).
* Many various graphical displays may have an additional `DC` (or `RS`) pin, which is required to be connected to any open digital pin in addition to its `CS` pin.
  * There is often an additional `Reset` (or `RST`) pin that needs either wired to an open digital pin for MCU control, otherwise typically will need hard-tied to a HIGH signal (such as that from `Vcc`) in order for the display to function/turn-on.
  * There is also often an additional `LED` (or `BL`) pin that controls the backlight that can be either optionally wired to an open digital or analog pin for MCU control, otherwise can be hard-tied typically to a HIGH signal (such as that from `Vcc`) in order to stay always-on, or simply left disconnected for device default.
* Always ensure that any data output pins and data input pins have compatible voltages.

SPI Devices Supported: SD card modules, NMEA GPS modules, 128x128+ LCD/OLED/TFT graphical displays, XPT2046 touchscreens

### I2C Bus

I2C (aka I²C, IIC, TwoWire, TWI) devices can be chained together on the same shared data lines (no flipping of wires), which are typically labeled `SCL` and `SDA`. Only different kinds of I2C devices can be used on the same data line together using factory default settings, otherwise manual addressing must be done. I2C runs at mid to high kHz speeds and is useful for advanced device control.

* When more than one I2C device of the same kind is to be used on the same data line, each device must be set to use a different address. This is accomplished via the A0-A2 (sometimes A0-A5) pins/pads on the physical device that must be set either open or closed (typically via a de-solderable resistor, or by shorting a pin/pad). Check your specific breakout's datasheet for details.
* Note that not all the I2C libraries used support multi-addressable I2C devices at this time (read as: may only use one). Currently, this restriction applies to: RTC devices.
* Always ensure that any data output pins and data input pins have compatible voltages.

I2C Devices Supported: DS*/PCF* RTC modules, AT24C* EEPROM modules, NMEA GPS modules, 16x2/20x4 LCD displays, 128x32/128x64 OLED displays, FT6206 touchscreens, 8/16-bit pin expanders

### OneWire Bus

OneWire devices can be chained together on the same shared data lines (no flipping of wires). Devices can be of the same or different types, require minimal setup (and often no soldering), and most can even operate in "parasite" power mode where they use the power from the data line (and an internal capacitor) to function (thus saving a `Vcc` line, only requiring `Data` and return `GND`). OneWire runs only in the low kb/s speeds and is useful for light-weight digital sensors.

* Typically, sensors are limited to 20 devices along a maximum 100m of wire.
* When more than one OneWire device is on the same device line, each device registers itself an enumeration index (0 - N) along with its own 64-bit unique identifier (UUID, with last byte being CRC). The device can then be referenced via this UUID by the system in the future indefinitely, or enumeration index so long as the device doesn't change its line position.
* Note that DHT* 1W devices may not play nicely together on the same wire line as other 1W devices - dependency issue.
* Always ensure that any data output pins and data input pins have compatible voltages.

OneWire Devices Supported: DHT* 1W air temp/humidity sensors, DS* 1W water temp sensors

### Analog IO

* All analog sensors will need to have the same operational voltage range as the controller supports. Many analog sensors are set to use 0v to 5v by default, but some can go -5v to +5v, some even up to 5.5v.
  * Note: Altering default factory calibration settings may require addition tools for setting up a new calibration, such as special calibration fluids/procedures/etc. Refer to the datasheet of your device for details.
* The `AREF` (or `IOREF`) pin, which controls the upper-bound of this range, by default if left not-connected (NC) is the same voltage as the MCU. Analog sensors must not exceed this voltage limit.
  * 5v analog sensor output signals connecting to 3.3v MCUs that are not 5v tolerant **must** either be: [level converted](https://randomnerdtutorials.com/how-to-level-shift-5v-to-3-3v/) in order to connect, or configured to output 0v to `AREF` (or `IOREF`) in voltage calibration output range (if able to calibrate - see note above).
  * Warning: Too high of applied voltage to any pin incapable of receiving such high a voltage risks permanent damage to that device. _Always_ ensure that the applied voltage level coming out a device is supported when going back into another. Some breakouts/IC's have 5v tolerance built-in, some do not. Refer to the datasheet of your MCU/device for details.
  * Note: Typically a 3.3v output signal will _not_ need level converted up to 5v for a 5v digital input to operate (read as: 3.3v is plenty enough to trigger HIGH on 5v device inputs).
* The SAM/SAMD family of MCUs (e.g. Due, Zero, MKR, Nano 33, etc.) as well as many more modern MCUs support different bit resolutions for analog/PWM pins (tied to overridable `DAC_RESOLUTION` & `ADC_RESOLUTION` defines), with some (e.g. Pico, ESP32, etc.) supporting any pin being digital or analog w/o restriction. Refer to the datasheet of your MCU for details.

### Sensors

* If able to set in hardware, ensure any electro-conductivity-based sensors (such as TDS meters and soil moisture sensors) use EC (aka mS/cm, millisiemens per centimeter) mode. EC is considered a normalized standard measurement while PPM can be split into different scale categories depending on sensor chemistry in use, which is often tied to country of origin. PPM/EC based sensors operating on anything other than a 1v=500ppm or 1v=1EC will need to have their scaling explicitly set.
* Many different kinds of hobbyist sensors label their analog output `AO` (or `Ao`) - however, always check your specific sensor's datasheet, as some may have non-standard pin designations.
  * Again, make sure all analog sensors are calibrated to output the same 0v - `AREF` (or `IOREF`) volts in range.
* Sensor pins used for event triggering when measurements go above/below a pre-set tolerance - many of which are deceptively labeled `DO` (or `Do`), despite having nothing to do with being `D`ata lines of any kind - can be safely ignored, as the software implementation of such mechanism is more than sufficient.
  * Often these connections are used to drive other hardware-only based solutions that aren't a part of Hydruino's use case, but can still be connected up using a BinarySensor that triggers upon specific conditions, possibly using an ISR-capable pin if desired.
  * BinarySensor state changes use a configurable stable-time filter before a new level is accepted. The default is 100ms. Use `setStateStableTime()` to adjust it, or set `stateStableTimeMs` to 0 to disable the filter.
* CO2 sensors are a bit unique - they require a 24 hour powered initialization period to burn off manufacturing chemicals, and _require_ `Vcc` for its heating element (5v @ 130mA for MQ-135) thus cannot use OneWire parasitic power mode. To calibrate, you have to set it outside while active until its voltage stabilizes, then calibrate its stabilized voltage to the current global known CO2 level.
* Avoid using volatile organic cleaners nearby humidity sensors - cleaning alcohols (like those commonly used in electronics) can permanently damage these devices.

### Networking & Wireless

* Networking of any kind is 100% optional. Base controller operation works offline using an RTC and optional GPS or known static location.
  * WiFi or Ethernet can be enabled when remote control, MQTT, network time, or network storage is wanted.
* Devices with built-in WiFi or Ethernet can enable such through header/build defines while other devices can utilize an external [serial ESP WiFi module](http://www.instructables.com/id/Cheap-Arduino-WiFi-Shield-With-ESP8266/) on any open Serial line.
  * Warning: While WiFi password is encrypted into system settings data, it should not be considered secure.
* Serial Bluetooth-AT modules can be used on any open Serial port to provide remote device control (only).
* MQTT requires remotely accessible broker daemon in order to publish sensor data (setup separately).
* UDP time server requires remotely accessible time & date API service in order to sync time (TODO).
  * RTC not required / used in reserve when UDP service enabled.
* Note: Geo-location APIs require external 3rd party monthly subscription fees, thus isn't included as a feature.

## Memory Callouts

* The total number of objects and different kinds of objects (reservoirs, pumps, probes, relays, etc.) that the controller can support at once depends on how much free Flash storage and SRAM your MCU has available. Hydruino C++0x11 objects range in memory usage size from 150 to 750 bytes or more depending on settings and object type, with the compiled Flash binary ranging in size from 200kB to 750kB+ depending on platform and settings.
  * For our supported microcontroller range, on the low end we have devices with 256kB of Flash and at least 16kB of SRAM, while on the upper end we have more modern devices with 1MB+ of Flash and 32kB+ of SRAM. Devices with < 24kB of SRAM may struggle with system builds and may be limited to minimal system setups (such as no WiFi, no data publishing, no built-in library data, only minimal-to-no GUI, etc.), while other newer devices with more capacity build with everything enabled.
* For AVR, SAM, and other build architectures that do not have C++0x11 STL (standard container library) support, there are a series of *`_MAXSIZE` defines nearer to the top of `Hydro[UI]Defines.h` that can be modified to adjust how much memory space is allocated for the various static array structures the controller instead uses.
* To save on the cost of code size for constrained devices, focus on not enabling that which you won't need, which has the benefit of being able to utilize code stripping to remove sections of code that don't get used.
  * There are also header defines that can strip out certain libraries and functionality, such as ones that disable the GUI, multi-tasking subsystems, etc.
* To further save on code size cost, see the Data Writer Example on how to externalize library data onto an SD Card or EEPROM.
  * Note: Upgrading between versions or changing custom/program data may require you to re-build and re-deploy to such external device.

## Example Usage

Below are several examples of controller usage.

### Simple Deep Water Culture (DWC) System Example

DWC setups are great for beginners and for crops that do not flower, and has the advantage of being able to be built out of commonly available plastic containers. Aeration is important in this setup to oxygenate the non-circulating water.

The Simple DWC Example shows how a small Hydruino system can be set up with a feed reservoir, aeration, grow lighting, and a timer-fed crop. Only the objects used by the sketch are built into the final binary, making it a lean starting point. The sketch has no UI or input control, but additional sensors and a buzzer can be added for local status and alerting.

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

    // Add timer fed crop set to run for 15 minutes once per hour, and links it to the feed water reservoir.
    auto crop = hydroController.addTimerFedCrop(JOIN(Hydro_CropType,SETUP_CROP_TYPE),
                                                JOIN(Hydro_SubstrateType,SETUP_CROP_SUBSTRATE),
                                                SETUP_CROP_SOW_DATE);
    crop->setFeedInterval(TimeSpan(60 * SECS_PER_MIN));
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

The supplied examples cover the main Hydruino system roles. The Simple DWC Example above is the recommended starting point; the remaining examples provide focused references for additional controller features.

* **SimpleDWC** - Basic DWC controller lifecycle with a feed reservoir, aeration, lighting, and timed crop feeding.
* **VerticalNFT** - Larger integrated NFT reference with sensors, dosing, reservoirs, rails, and configurable UI/storage paths.
* **FullSystem** - Full-feature empty-system reference intended for UI-driven configuration on larger MCUs.
* **DataWriter** - Crop and string data export for external EEPROM or SD storage.

### Data Writer Example

The Data Writer Example can offload exportable crop and string data to SD card or EEPROM storage, which can reduce Flash usage on storage-constrained MCUs.

It does not run the Hydruino controller in full. Instead, it builds the exportable library data into a small writer sketch and emits either binary EEPROM data or human-readable JSON files.

Hydruino can operate with the built-in data kept in Flash. External storage is optional and is mainly useful when program space matters or custom crop data is desired.

Serial logging output can also be enabled with `HYDRO_ENABLE_DEBUG_OUTPUT`, described above under Header Defines.
