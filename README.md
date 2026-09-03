# Hydruino
Hydruino: Simple Hydroponics Automation Controller.

**Simple-Hydroponics-Arduino v0.7.2.0**

Simple automation controller for hydroponic grow systems.  
Licensed under the non-restrictive MIT license.

Created by NachtRaveVL, May 20th, 2022.

This project is part of a four-library controller family: **Simple-Hydroponics-Arduino (Hydruino)**, **Simple-SolarTracker-Arduino (Helioduino)**, **Simple-Homestead-Arduino (Terraduino)**, and **Simple-AstroTracker-Arduino (Astruino)**.

This controller manages reservoirs, pumps, probes, relays, lighting, dosing, watering, and data collection for hydroponic grow systems. It supports common Arduino-compatible RTC, GPS, EEPROM, SD, networking, sensor, and display hardware while keeping networking completely optional. The built-in crop library provides growth-stage timing and environmental targets, and timed crops can water a configured number of times per day, number of times per week, or after an elapsed interval.

The Keep-It-Simple controller system:

* Can be used entirely offline with an RTC module for timekeeping and either a known static location or optional GPS module for location, or used online through enabled on-board WiFi/Ethernet or an external ESP-AT WiFi module.
  * Uses [SolarCalculator](https://github.com/jpb10/SolarCalculator), inspired by the NOAA Solar Calculator, for fine offline calculations of the sun's solar position (including sunrise, sunset, & transit times), accurate until 2100.
* Exportable system configuration to EEPROM, SD card, or WiFiStorage external storage device.
  * Saved in pretty-print JSON for human-readability & easy text editing, or in raw binary for compactness & speed.
  * Auto-save, backup auto-save (for auto-recovery), and low external storage space cleanup functionality.
  * Import string decode functions are pre-optimized with minimum spanning trie for ultra-fast text parsing & reduced loading times.
* Supports interval-based sensor data publishing and system event logging to MQTT IoT broker (for further IoT-integrated processing) or to external storage in .csv/.txt format (/w date in filename, segmented daily).
  * Can be extended to work with other JSON-based Web APIs or Client-like derivatives (for DB storage or server-endpoint support).
* Enabled GUI works with a large variety of common Arduino-compatible LCD/OLED/TFT displays, touchscreens, matrix keypads, analog joysticks, rotary encoders, and momentary buttons (support by [tcMenu](https://github.com/davetcc/tcMenuLib)).
  * Remote tcMenu connections can use enabled Serial, WiFi, Ethernet, or Simhub transports with compatible tcMenu clients such as [embedCONTROL](https://github.com/davetcc/tcMenu/releases).
  * GUI input handling supports interrupt-assisted or polling operation depending on the selected control type and the capabilities of the pins used.
  * System examples can be compiled in:
    * Disabled UI mode, which removes all GUI code entirely, freeing a large amount of Flash size for constrained (<=256kB Flash) devices.
    * Minimal UI mode, which saves on compiled sketch size through optimized code stripping at the cost of having to modify/re-upload a new sketch to change most system settings (or to change system object structure).
    * Full UI mode, which uses large amounts of Flash space available on modern MCUs to provide everything all at once, with only major system (or static linked component) changes requiring a sketch modify/re-upload.
* Actuator & Sensor pins can be multiplexed or expanded along with any control input pins through 8/16-bit i2c expanders for pin-limited controllers.
* Library data can be built into onboard Flash or exported onto external storage to additionally save on compiled sketch size.

Designed primarily for Arduino and Arduino-compatible build environments. PlatformIO can also be used with supported Arduino cores for Espressif, Teensy, STM32, RP2040/RP2350, and other compatible targets.

Datasheet links include: [DS18B20 Temperature Sensor](https://github.com/NachtRaveVL/Simple-Hydroponics-Arduino/blob/main/extra/DS18B20.pdf), [DHT12 Air Temperature and Humidity Sensor](https://github.com/NachtRaveVL/Simple-Hydroponics-Arduino/blob/main/extra/dht12.pdf), [4502c Analog pH Sensor (writeup)](https://github.com/NachtRaveVL/Simple-Hydroponics-Arduino/blob/main/extra/ph-sensor-ph-4502c.pdf), but many more are available online.

*If this work is useful, project support is always appreciated through [Patreon](https://www.patreon.com/nachtrave).*

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

Some embedded SD/FAT library configurations use 8.3 filenames, limiting file/folder names to eight characters plus a three-character extension. Long-filename support depends on the storage library and build configuration.

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

* Use the supply and logic voltages required by the selected MCU and peripherals. Arduino-compatible hardware commonly uses either 5V or 3.3V logic, and many 3.3V devices are not 5V tolerant. Check the MCU, module, and IC datasheets before connecting signals.
* When two devices use incompatible logic levels, use a level-shifting method appropriate to the signal. A unidirectional resistor divider can be suitable for some slow 5V-to-3.3V signals, while bidirectional or high-speed buses generally need a proper level shifter. A single series resistor is not a general-purpose voltage converter.
* Never apply a signal outside the receiving pin's absolute-maximum and input-threshold specifications. Do not assume that a 3.3V HIGH will always satisfy a 5V input, or that a breakout board provides level shifting unless its documentation says so.

### Serial UART

Serial UART uses individual communication lines for each device, with the receive `RX` pin of one being the transmit `TX` pin of the other - thus having to "flip wires" when connecting. However, devices can always be active and never have to share their access. UART commonly operates from low kilobit/s rates into the hundreds of kilobits/s and is useful for simple point-to-point device control.

* When wiring up modules that use Serial UART, make sure to flip `RX`/`TX` pins.
* Always ensure that any data output pins and data input pins have compatible voltages.

Serial UART Devices Supported: Bluetooth-AT modules, ESP-AT WiFi modules, NMEA-AT GPS modules

### SPI Bus

SPI devices can be chained together on the same shared data lines, which are typically labeled `COPI` (or `MOSI`), `CIPO` (or `MISO`), and `SCK`, often with an additional `CS` (or `SS`). Each SPI device requires its own individual chip-select `CS` wire as only one SPI device may be active at any given time - accomplished by pulling its `CS` line of that device low (aka active-low). SPI runs at MHz speeds and is useful for large data block transfers.

* The `CS` pin may be connected to any digital output pin, but it's common to use the `CS` (or `SS`) pin for the first device. Additional devices are not restricted to what pin they can or should use, but given it's not a data pin not using a choice interrupt-capable pin allows those to be used for interrupt driven mechanisms.
* SD-card compatibility depends on the card, breakout hardware, voltage translation, and the storage library used by the target. FAT32 is widely supported on embedded Arduino setups, while maximum card size and long-filename support vary by library and configuration.
  * Validate the exact card and module combination on the target hardware before relying on it for configuration, logging, or autosave.
* Many various graphical displays may have an additional `DC` (or `RS`) pin, which is required to be connected to any open digital pin in addition to its `CS` pin.
  * There is often an additional `Reset` (or `RST`) pin that needs either wired to an open digital pin for MCU control, otherwise typically will need hard-tied to a HIGH signal (such as that from `Vcc`) in order for the display to function/turn-on.
  * There is also often an additional `LED` (or `BL`) pin that controls the backlight that can be either optionally wired to an open digital or analog pin for MCU control, otherwise can be hard-tied typically to a HIGH signal (such as that from `Vcc`) in order to stay always-on, or simply left disconnected for device default.
* Always ensure that any data output pins and data input pins have compatible voltages.

SPI Devices Supported: SD card modules, NMEA GPS modules, 128x128+ LCD/OLED/TFT graphical displays, XPT2046 touchscreens

### I2C Bus

I2C (aka I²C, IIC, TwoWire, TWI) devices share the same `SCL` and `SDA` lines. Every active device on a bus must have a non-conflicting address; whether two modules can coexist depends on their configurable address ranges, not simply on whether they are the same device type. I2C commonly runs at 100kHz or 400kHz, with other rates supported by some hardware.

* When addresses conflict, use the device's address-select pins/pads when available, place the devices on separate buses, or use an appropriate I2C multiplexer. Check the specific breakout and library documentation for supported addressing.
* Note that not all the I2C libraries used support multi-addressable I2C devices at this time (read as: may only use one). Currently, this restriction applies to: RTC devices.
* Always ensure that any data output pins and data input pins have compatible voltages.

I2C Devices Supported: DS*/PCF* RTC modules, AT24C* EEPROM modules, NMEA GPS modules, 16x2/20x4 LCD displays, 128x32/128x64 OLED displays, FT6206 touchscreens, 8/16-bit pin expanders

### OneWire Bus

Dallas/Maxim OneWire devices can share a single data line and are identified by a 64-bit ROM address. Some devices support parasite power, but externally powered operation is often more robust for larger or electrically noisy installations.

* Practical bus length and device count depend on cable capacitance, topology, pull-up strength, power mode, timing, and the devices in use. Validate the real wiring rather than relying on a fixed universal distance or device-count limit.
* When more than one OneWire device is present, the controller can identify devices by their 64-bit ROM address and may also use an enumeration position where supported. The ROM address is the stable identity; enumeration order can change if the bus population or topology changes.
* Always ensure that any data output pins and data input pins have compatible voltages.

OneWire Devices Supported: DS18-series temperature sensors. DHT sensors use their own single-data-wire protocol and are not Dallas/Maxim OneWire devices.

### Analog IO

* Analog sensors must stay within the electrical input range of the MCU pin. A sensor that can output more voltage than the ADC input allows needs attenuation, level conversion, or a different interface before it is connected.
  * Altering a sensor's factory calibration or output range may require calibration references, fluids, or procedures specific to that device. Refer to its datasheet.
* ADC reference behavior is board-specific. `AREF` can select or accept an analog reference on boards that support it when configured appropriately; `IOREF` normally indicates the board's logic voltage and is not interchangeable with `AREF`. Check the board documentation before using either pin.
  * Applying voltage beyond a pin's rated range can permanently damage the MCU. Verify both the normal operating range and absolute-maximum rating.
* ADC-capable pins, PWM-capable pins, and supported ADC/DAC resolutions vary substantially by MCU and board. The `ADC_RESOLUTION` and `DAC_RESOLUTION` defines describe the configured conversion resolution; they do not imply that every GPIO supports analog input or output. Refer to the target board's pinout and datasheet.

### Sensors

* When a conductivity/TDS sensor can report EC directly, prefer EC (mS/cm) so the measurement is not tied to an ambiguous PPM conversion factor. If the device reports TDS/PPM instead, configure the conversion scale used by that sensor explicitly.
* Many different kinds of hobbyist sensors label their analog output `AO` (or `Ao`) - however, always check your specific sensor's datasheet, as some may have non-standard pin designations.
  * Ensure the sensor output remains within the configured ADC input range and reference used by the target board.
* Many sensor modules expose a digital threshold output labeled `DO` (or `Do`). It is optional when software thresholds on the measured value are sufficient, but it can be useful when the hardware threshold itself should be monitored.
  * Connect that output through a `HydroBinarySensor` when it should participate in the controller, optionally using an ISR-capable pin where appropriate.
  * `HydroBinarySensor` state changes use a configurable stable-time filter before a new level is accepted. The default is 100ms. Use `setStateStableTime()` to adjust it, or set `stateStableTimeMs` to 0 to disable the filter.
* Gas and CO2 sensors vary widely. Heated metal-oxide devices such as the MQ-135 require substantial warm-up/burn-in and are not inherently selective or calibrated CO2 instruments. Follow the specific sensor manufacturer's power, warm-up, and calibration procedure.
* Avoid using volatile organic cleaners nearby humidity sensors - cleaning alcohols (like those commonly used in electronics) can permanently damage these devices.

### Networking & Wireless

* Networking of any kind is 100% optional. Base controller operation works offline using an RTC for timekeeping and either an optional GPS receiver or known static location for location data.
  * WiFi or Ethernet can be enabled when remote control, MQTT, or network storage is wanted.
* Devices with built-in WiFi or Ethernet can enable such through header/build defines while other devices can utilize an external [serial ESP WiFi module](http://www.instructables.com/id/Cheap-Arduino-WiFi-Shield-With-ESP8266/) on any open Serial line.
  * Warning: While WiFi password is encrypted into system settings data, it should not be considered secure.
* Serial Bluetooth-AT modules can be used on any open Serial port to provide remote device control (only).
* MQTT requires remotely accessible broker daemon in order to publish sensor data (setup separately).
* Note: Geo-location APIs require external 3rd party monthly subscription fees, thus isn't included as a feature.

## Memory Callouts

* The total number of objects and different kinds of objects (reservoirs, pumps, probes, relays, etc.) that the controller can support at once depends on how much free Flash storage and SRAM your MCU has available. Exact object and firmware sizes depend on the target architecture, enabled features, compiler, and system configuration.
  * For our supported microcontroller range, on the low end we have devices with 256kB of Flash and at least 16kB of SRAM, while on the upper end we have more modern devices with 1MB+ of Flash and 32kB+ of SRAM. Devices with < 24kB of SRAM may struggle with system builds and may be limited to minimal system setups (such as no WiFi, no data publishing, no built-in library data, only minimal-to-no GUI, etc.), while other newer devices with more capacity build with everything enabled.
* For AVR, SAM, and other build architectures that do not have C++11 STL (standard container library) support, there are a series of *`_MAXSIZE` defines nearer to the top of `Hydro[UI]Defines.h` that can be modified to adjust how much memory space is allocated for the various static array structures the controller instead uses.
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
