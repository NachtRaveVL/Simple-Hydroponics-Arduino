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

From there, you can make a local copy of one of the examples based on the kind of system setup you want to use. If you are unsure of which, we recommend using the Vertical NFT Example for older storage constrained MCUs, or using the Full System Example for more modern MCUs.

### Header Defines

There are several defines inside of the controller's main header file that allow for more fine-tuned control of the controller. You may edit and uncomment these lines directly, or supply them via custom build flags. While editing the main header file isn't ideal, it is often easiest. Note that editing the controller's main header file directly will affect all projects compiled on your system using those modified controller files.

Alternatively, you may also refer to <https://forum.arduino.cc/index.php?topic=602603.0> on how to define custom build flags manually via modifying the platform[.local].txt file. Note that editing such directly will affect all other projects compiled on your system using those modified platform framework files, but at least you keep those changes to the same place.

From Hydroponics.h:
```Arduino
// Uncomment or -D this define to completely disable usage of any multitasking commands and libraries. Not recommended.
//#define HYDRUINO_DISABLE_MULTITASKING             // https://github.com/davetcc/TaskManagerIO

// Uncomment or -D this define to disable usage of tcMenu library, which will disable all GUI control. Not recommended.
//#define HYDRUINO_DISABLE_GUI                      // https://github.com/davetcc/tcMenu

// Uncomment or -D this define to enable external data storage (Crops, Strings, etc.) to save on sketch size. Required for constrained devices.
//#define HYDRUINO_ENABLE_EXTERNAL_DATA             // If enabled, must use external device (such as SD Card or EEPROM) for data fetches.

// Uncomment or -D this define to enable debug output (treats Serial as attached to serial monitor).
//#define HYDRUINO_ENABLE_DEBUG_OUTPUT

// Uncomment or -D this define to enable debug assertions (note: adds considerable size to sketch).
//#define HYDRUINO_ENABLE_DEBUG_ASSERTIONS
```

### Controller Initialization

There are several initialization mode settings exposed through this controller that are used for more fine-tuned control.

#### Class Instantiation

The controller's class object must first be instantiated, commonly at the top of the sketch where pin setups are defined (or exposed through some other mechanism), which makes a call to the controller's class constructor. The constructor allows one to set the module's piezo buzzer pin, EEPROM device size, SD Card CS pin, input ribbon pin1, EEPROM i2c address, RTC i2c address, LCD i2c address,, i2c Wire class instance, i2c clock speed, SD Card SPI speed (hard-wired to `25M`Hz on Teensy), and WiFi class instance. The default constructor values of the controller, if left unspecified, has no pins set, zeroed i2c addresses, i2c Wire class instance `Wire` @`400k`Hz, SD Card SPI speed @ `4M`Hz, and WiFi class instance `WiFi`.

From Hydroponics.h, in class Hydroponics:
```Arduino
    // Library constructor. Typically called during class instantiation, before setup().
    Hydroponics(byte piezoBuzzerPin = -1,                   // Piezo buzzer pin, else -1
                uint32_t eepromDeviceSize = 0,              // EEPROM bit storage size (use I2C_DEVICESIZE_* defines), else 0
                byte sdCardCSPin = -1,                      // SD card CS pin, else -1
                byte controlInputPin1 = -1,                 // First pin of input ribbon, else -1 (ribbon pins can be individually customized later)
                byte eepromI2CAddress = B000,               // EEPROM address
                byte rtcI2CAddress = B000,                  // RTC i2c address (only B000 can be used atm)
                byte lcdI2CAddress = B000,                  // LCD i2c address
                TwoWire &i2cWire = Wire,                    // I2C wire class instance
                uint32_t i2cSpeed = 400000U,                // I2C speed, in Hz
                uint32_t sdCardSpeed = 4000000U,            // SD card SPI speed, in Hz (ignored if on Teensy)
                WiFiClass &wifi = WiFi);                    // WiFi class instance
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
* For our target microcontroller range, on the low end we have ATMega2560 with 256kB of Flash and 8kB of RAM, while more recent devices like the RasPi Pico have 2MB of Flash and 264kB of RAM. The ATMega2560 may struggle with full system builds and may be limited to specific system setups (such as no debug assertions, external crops library, only minimal UI, etc.), while other newer devices with more capacity build with everything enabled without issue.
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
#include <Hydroponics.h>

#define SETUP_GROW_LIGHTS_PIN           8               // Grow lights relay pin (digital)
#define SETUP_WATER_AERATOR_PIN         7               // Aerator relay pin (digital)
#define SETUP_FEED_RESERVOIR_SIZE       5               // Reservoir size, in default measurement units
#define SETUP_AC_POWER_RAIL_TYPE        AC110V          // Rail power type used for AC rail (AC110V, AC220V)

#define SETUP_CROP1_TYPE                Lettuce         // Type of crop planted at position 1, else Undefined
#define SETUP_CROP1_SUBSTRATE           ClayPebbles     // Type of crop substrate at position 1
#define SETUP_CROP1_SOW_DATE            DateTime(2022, 5, 21) // Date that crop was planted at position 1

Hydroponics hydroController;            // Controller using default settings

void setup() {
    // Initializes controller with default environment, no logging, eeprom, SD, or anything else.
    hydroController.init();

    // Adds a simple relay power rail using standard AC. This will manage how many active devices can be turned on at the same time.
    auto relayPower = hydroController.addSimplePowerRail(JOIN(Hydroponics_RailType,SETUP_AC_POWER_RAIL_TYPE));

    // Adds a 5 unit main water reservoir, already filled with feed water.
    auto feedReservoir = hydroController.addFeedWaterReservoir(SETUP_FEED_RESERVOIR_SIZE, true);

    // Adds a water aerator at SETUP_WATER_AERATOR_PIN, and links it to the feed water reservoir and the relay power rail.
    auto aerator = hydroController.addWaterAeratorRelay(SETUP_WATER_AERATOR_PIN);
    aerator->setRail(relayPower);
    aerator->setReservoir(feedReservoir);

    // Add grow lights relay at SETUP_GROW_LIGHTS_PIN, and links it to the feed water reservoir and the relay power rail.
    auto lights = hydroController.addGrowLightsRelay(SETUP_GROW_LIGHTS_PIN);
    lights->setRail(relayPower);
    lights->setReservoir(feedReservoir);

    // Add timer fed crop set to feed on a standard 15 mins on/45 mins off timer, and links it to the feed water reservoir.
    auto crop = hydroController.addTimerFedCrop(JOIN(Hydroponics_CropType,SETUP_CROP1_TYPE),
                                                JOIN(Hydroponics_SubstrateType,SETUP_CROP1_SUBSTRATE),
                                                SETUP_CROP1_SOW_DATE);
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

The Vertical NFT Example sketch is the standard implementation for our 3D printed controller enclosure and for most vertical towers that will be used. It is recommended for storage constrained MCUs such as the ATMega2560, ESP8266, etc. It can be easily extended to include other functionality if desired, simply by copying and pasting the example code. This system code has the benefit of being able to compile out what you don't use, making it ideal for storage constrained devices, but will not provide full UI functionality since it will be missing the code for all the other objects the system build code strips out.

The Full System Example sketch on the other hand will build an empty system with all object and system features enabled. It is recommended for more modern MCUs that have plenty of storage space to use such as the ESP32, Raspberry Pi Pico, etc. It works similarly to the Vertical NFT Example, except is meant for systems where UI interaction will be used to create the objects (or also done in code to initialize, as is done in the Vertical NFT example). It involves the least amount of coding and setup, but comes at the highest cost.

Included below is the default system setup defines of the Vertical NFT example to illustrate a variety of the controller features. This is not an exhaustive list of course, as there are many more things the controller is capable of, as documented in its main header file include and elsewhere.

```Arduino
#include <Hydroponics.h>

// Pins & Class Instances
#define SETUP_PIEZO_BUZZER_PIN          11              // Piezo buzzer pin, else -1
#define SETUP_EEPROM_DEVICE_SIZE        I2C_DEVICESIZE_24LC256 // EEPROM bit storage size (use I2C_DEVICESIZE_* defines), else 0
#define SETUP_SD_CARD_CS_PIN            SS              // SD card CS pin, else -1
#define SETUP_CTRL_INPUT_PINS           {31.33,30,32}   // Control input pin ribbon, else {-1}
#define SETUP_EEPROM_I2C_ADDR           B000            // EEPROM address
#define SETUP_RTC_I2C_ADDR              B000            // RTC i2c address (only B000 can be used atm)
#define SETUP_LCD_I2C_ADDR              B000            // LCD i2c address
#define SETUP_I2C_WIRE_INST             Wire            // I2C wire class instance
#define SETUP_I2C_SPEED                 400000U         // I2C speed, in Hz
#define SETUP_ESP_I2C_SDA               SDA             // I2C SDA pin, if on ESP
#define SETUP_ESP_I2C_SCL               SCL             // I2C SCL pin, if on ESP
#define SETUP_SD_CARD_SPI_SPEED         4000000U        // SD card SPI speed, in Hz (ignored if on Teensy)
#define SETUP_WIFI_INST                 WiFi            // WiFi class instance

// System Settings
#define SETUP_SYSTEM_MODE               Recycling       // System run mode (Recycling, DrainToWaste)
#define SETUP_MEASURE_MODE              Imperial        // System measurement mode (Default, Imperial, Metric, Scientific)
#define SETUP_LCD_OUT_MODE              Disabled        // System LCD output mode (Disabled, 20x4LCD, 20x4LCD_Swapped, 16x2LCD, 16x2LCD_Swapped)
#define SETUP_CTRL_IN_MODE              Disabled        // System control input mode (Disabled, 2x2Matrix, 4xButton, 6xButton, RotaryEncoder)
#define SETUP_SYS_NAME                  "Hydruino"      // System name
#define SETUP_SYS_TIMEZONE              +0              // System timezone offset

// System Saves Settings                                (note: only one save mechanism is enabled at a time)
#define SETUP_SYS_AUTOSAVE_ENABLE       true            // If autosaving system out is enabled or not
#define SETUP_SAVES_SD_CARD_ENABLE      true            // If saving/loading from SD card is enable
#define SETUP_SD_CARD_CONFIG_FILE       "hydruino.cfg"  // System config file name for SD Card saves
#define SETUP_SAVES_EEPROM_ENABLE       false           // If saving/loading from EEPROM is enabled 
#define SETUP_EEPROM_SYSDATA_ADDR       0x2588          // System data memory offset for EEPROM saves (from Data Writer output)

// WiFi Settings
#define SETUP_ENABLE_WIFI               false           // If WiFi is enabled
#define SETUP_WIFI_SSID                 "CHANGE_ME"     // WiFi SSID
#define SETUP_WIFI_PASS                 "CHANGE_ME"     // WiFi password

// Logging & Data Publishing Settings
#define SETUP_LOG_SD_ENABLE             true            // If system logging is enabled to SD card
#define SETUP_LOG_FILE_PREFIX           "logs/hy"       // System logs file prefix (appended with YYMMDD.txt)
#define SETUP_DATA_SD_ENABLE            true            // If system data publishing is enabled to SD card
#define SETUP_DATA_FILE_PREFIX          "data/hy"       // System data publishing files prefix (appended with YYMMDD.csv)

// External Data Settings
#define SETUP_EXTDATA_SD_ENABLE         true            // If data should be read from an external SD Card
#define SETUP_EXTDATA_SD_LIB_PREFIX     "lib/"          // Library data folder/data file prefix (appended with {type}##.dat)
#define SETUP_EXTDATA_EEPROM_ENABLE     true            // If data should be read from an external EEPROM
#define SETUP_EXTDATA_EEPROM_CROP_ADDR  0               // Start address for Crops Library data (from Data Writer output)
#define SETUP_EXTDATA_EEPROM_STR_ADDR   0               // Start address for Strings data (from Data Writer output)

// Base Setup
#define SETUP_FEED_RESERVOIR_SIZE       4               // Reservoir size, in default measurement units
#define SETUP_AC_POWER_RAIL_TYPE        AC110V          // Rail power type used for AC rail (AC110V, AC220V)
#define SETUP_DC_POWER_RAIL_TYPE        DC12V           // Rail power type used for peristaltic pump rail (DC5V, DC12V)
#define SETUP_AC_SUPPLY_POWER           0               // Maximum AC supply power wattage, else 0 if not known (-> use simple rails)
#define SETUP_DC_SUPPLY_POWER           0               // Maximum DC supply power wattage, else 0 if not known (-> use simple rails)

// Device Setup
#define SETUP_PH_METER_PIN              A0              // pH meter sensor pin (analog), else -1
#define SETUP_TDS_METER_PIN             A1              // TDS meter sensor pin (analog), else -1
#define SETUP_CO2_SENSOR_PIN            -1              // CO2 meter sensor pin (analog), else -1
#define SETUP_POWER_SENSOR_PIN          -1              // Power meter sensor pin (analog), else -1
#define SETUP_FLOW_RATE_SENSOR_PIN      -1              // Main feed pump flow rate sensor pin (analog/PWM), else -1
#define SETUP_DS18_WTEMP_PIN            3               // DS18* water temp sensor data pin (digital), else -1
#define SETUP_DHT_ATEMP_PIN             4               // DHT* air temp sensor data pin (digital), else -1
#define SETUP_DHT_SENSOR_TYPE           DHT12           // DHT sensor type enum (use DHT* defines)
#define SETUP_VOL_FILLED_PIN            -1              // Water level filled indicator pin (digital/ISR), else -1
#define SETUP_VOL_EMPTY_PIN             -1              // Water level empty indicator pin (digital/ISR), else -1
#define SETUP_GROW_LIGHTS_PIN           22              // Grow lights relay pin (digital), else -1
#define SETUP_WATER_AERATOR_PIN         24              // Aerator relay pin (digital), else -1
#define SETUP_FEED_PUMP_PIN             26              // Water level low indicator pin, else -1
#define SETUP_WATER_HEATER_PIN          28              // Water heater relay pin (digital), else -1
#define SETUP_WATER_SPRAYER_PIN         -1              // Water sprayer relay pin (digital), else -1
#define SETUP_FAN_EXHAUST_PIN           -1              // Fan exhaust relay pin (digital/PWM), else -1
#define SETUP_NUTRIENT_MIX_PIN          23              // Nutrient premix peristaltic pump relay pin (digital), else -1
#define SETUP_FRESH_WATER_PIN           25              // Fresh water peristaltic pump relay pin (digital), else -1
#define SETUP_PH_UP_PIN                 27              // pH up solution peristaltic pump relay pin (digital), else -1
#define SETUP_PH_DOWN_PIN               29              // pH down solution peristaltic pump relay pin (digital), else -1

// Crop Setup
#define SETUP_CROP_ON_TIME              15              // Minutes feeding pumps are to be turned on for
#define SETUP_CROP_OFF_TIME             45              // Minutes feeding pumps are to be turned off for
#define SETUP_CROP1_TYPE                Lettuce         // Type of crop planted at position 1, else Undefined
#define SETUP_CROP1_SUBSTRATE           ClayPebbles     // Type of crop substrate at position 1
#define SETUP_CROP1_SOW_DATE            DateTime(2022, 5, 21) // Date that crop was planted at position 1
#define SETUP_CROP1_SOILM_PIN           -1              // Soil moisture sensor for crop at position 1 pin (analog), else -1
#define SETUP_CROP2_TYPE                Lettuce         // Type of crop planted at position 2, else Undefined
#define SETUP_CROP2_SUBSTRATE           ClayPebbles     // Type of crop substrate at position 2
#define SETUP_CROP2_SOW_DATE            DateTime(2022, 5, 21) // Date that crop was planted at position 2
#define SETUP_CROP2_SOILM_PIN           -1              // Soil moisture sensor for crop at position 2 pin (analog), else -1
#define SETUP_CROP3_TYPE                Lettuce         // Type of crop planted at position 3, else Undefined
#define SETUP_CROP3_SUBSTRATE           ClayPebbles     // Type of crop substrate at position 3
#define SETUP_CROP3_SOW_DATE            DateTime(2022, 5, 21) // Date that crop was planted at position 3
#define SETUP_CROP3_SOILM_PIN           -1              // Soil moisture sensor for crop at position 3 pin (analog), else -1
#define SETUP_CROP4_TYPE                Lettuce         // Type of crop planted at position 4, else Undefined
#define SETUP_CROP4_SUBSTRATE           ClayPebbles     // Type of crop substrate at position 4
#define SETUP_CROP4_SOW_DATE            DateTime(2022, 5, 21) // Date that crop was planted at position 4
#define SETUP_CROP4_SOILM_PIN           -1              // Soil moisture sensor for crop at position 4 pin (analog), else -1
#define SETUP_CROP5_TYPE                Lettuce         // Type of crop planted at position 5, else Undefined
#define SETUP_CROP5_SUBSTRATE           ClayPebbles     // Type of crop substrate at position 5
#define SETUP_CROP5_SOW_DATE            DateTime(2022, 5, 21) // Date that crop was planted at position 5
#define SETUP_CROP5_SOILM_PIN           -1              // Soil moisture sensor for crop at position 5 pin (analog), else -1
```

### Data Writer Example

The Data Writer Example sketch can be used to write the Crops Library and Strings data onto an SD Card or EEPROM so that storage constrained devices can still build at least the Vertical NFT system. This Example doesn't actually run the controller in full, but a code stripped version of it that will more easily compile down onto storage constrained devices. You can also program in any custom data you want made available later if you choose to change the default library data, such as custom crops data.

Inside of the Data Writer's `setup()` function:
```Arduino
    // Just a lone initializer is all that's needed since we won't actually be using the full controller.
    hydroController.init();

    // Right here would be the place to program in any custom crop data that you want made available for later.
    //HydroponicsCropsLibData customCrop1(Hydroponics_CropType_CustomCrop1);
    //strncpy(customCrop1.cropName, "Custom name", HYDRUINO_NAME_MAXSIZE);
    //getCropsLibraryInstance()->setCustomCropData(&customCrop1);
```

In particular, after setting up the settings defines similarly to that of the Vertical NFT or Full System sketch, running the Data Writer sketch will produce the EEPROM configuration setup defines to use for that particular run. You typically won't need to copy these over unless you are planning to utilize an external EEPROM and have made any custom data modifications, as these defines are updated by the dev team prior to release. If however you do have custom data modifications and using an external EEPROM, do copy these values over into your Main System sketch.

In serial monitor:
```
2022-07-29T14:43:33 [INFO] Use the following EEPROM configuration defines in your sketch:
#define SETUP_EXTDATA_EEPROM_CROP_ADDR  0x0
#define SETUP_EXTDATA_EEPROM_STR_ADDR   0x1b70
#define SETUP_EEPROM_SYSDATA_ADDR       0x2588
```
