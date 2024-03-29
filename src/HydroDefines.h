/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Defines
*/

#ifndef HydroDefines_H
#define HydroDefines_H

#ifndef JOIN                                                // Define double joiner
#define JOIN_(X,Y)                      X##_##Y
#define JOIN(X,Y)                       JOIN_(X,Y)
#endif
#ifndef JOIN3                                               // Define triple joiner
#define JOIN3_(X,Y,Z)                   X##_##Y##_##Z
#define JOIN3(X,Y,Z)                    JOIN3_(X,Y,Z)
#endif
#ifndef STR                                                 // Define stringifier
#define STR_(X)                         #X
#define STR(X)                          STR_(X)
#endif

#define ACT_HIGH                        false               // Active high (convenience)
#define ACT_ABOVE                       false               // Active above (convenience)
#define ACT_LOW                         true                // Active low (convenience)
#define ACT_BELOW                       true                // Active below (convenience)
#define PULL_DOWN                       false               // Pull down (convenience)
#define PULL_UP                         true                // Pull up (convenience)
#define RAW                             false               // Raw mode (convenience)
#define JSON                            true                // JSON mode (convenience)

#ifndef FLT_EPSILON
#define FLT_EPSILON                     0.00001f            // Single-precision floating point error tolerance
#endif
#ifndef DBL_EPSILON
#define DBL_EPSILON                     0.0000000000001     // Double-precision floating point error tolerance
#endif
#ifndef FLT_UNDEF
#define FLT_UNDEF                       __FLT_MAX__         // Single-precision floating point value to stand in for "undefined"
#endif
#ifndef DBL_UNDEF
#define DBL_UNDEF                       __DBL_MAX__         // Double-precision floating point value to stand in for "undefined"
#endif
#ifndef MIN_PER_DAY
#define MIN_PER_DAY                     1440                // Minutes per day
#endif
#ifndef TWO_PI
#define TWO_PI                          6.283185307179586476925286766559 // Two pi
#endif

// Platform standardizations
#if (defined(ESP32) || defined(ESP8266)) && !defined(ESP_PLATFORM) // ESP_PLATFORM for any esp
#define ESP_PLATFORM
#endif
#if defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_STM32)    // Missing min/max
#define min(a,b)                        ((a)<(b)?(a):(b))
#define max(a,b)                        ((a)>(b)?(a):(b))
#endif
#if defined(CORE_TEENSY)                                    // Missing abs
#define abs(x)                          ((x)>0?(x):-(x))
#endif
#ifndef RANDOM_MAX                                          // Missing RANDOM_MAX
#if defined(RAND_MAX)
#define RANDOM_MAX                      RAND_MAX
#else
#define RANDOM_MAX                      __INT_MAX__
#endif
#endif

// MCU capabilities
#ifndef ADC_RESOLUTION                                      // Resolving ADC resolution, or define manually by build define (see BOARD for example)
#if defined(IOA_ANALOGIN_RES)                               // From IOAbstraction
#define ADC_RESOLUTION                  IOA_ANALOGIN_RES
#else
#define ADC_RESOLUTION                  10                  // Default per AVR
#endif
#endif
#ifndef DAC_RESOLUTION                                      // Resolving DAC resolution, or define manually by build define (see BOARD for example)
#if defined(IOA_ANALOGOUT_RES)                              // From IOAbstraction
#define DAC_RESOLUTION                  IOA_ANALOGOUT_RES
#else
#define DAC_RESOLUTION                  8                   // Default per AVR
#endif
#endif
#ifndef F_SPD                                               // Resolving F_SPD=F_CPU|F_BUS alias (for default SPI device speeds), or define manually by build define (see BOARD for example)
#if defined(F_CPU)
#define F_SPD                           F_CPU
#elif defined(F_BUS)                                        // Teensy/etc support
#define F_SPD                           F_BUS
#else                                                       // Fast/good enough (32MHz)
#define F_SPD                           32000000U
#endif
#endif
#ifndef V_MCU                                               // Resolving MCU voltage, or define manually by build define (see BOARD for example)
#if (defined(__AVR__) || defined(__STM8__)) && !defined(ARDUINO_AVR_FIO) && !(defined(ARDUINO_AVR_PRO) && F_CPU == 8000000L)
#define V_MCU                           5.0                 // 5v MCU (assumed/tolerant)
#else
#define V_MCU                           3.3                 // 3v3 MCU (assumed)
#endif
#endif
#ifndef BOARD                                               // Resolving board name alias, or define manually by build define via creating platform.local.txt in %applocaldata%\Arduino15\packages\{platform}\hardware\{arch}\{version}\ containing (/w quotes): compiler.cpp.extra_flags="-DBOARD={build.board}"
#if defined(CORE_TEENSY)                                    // For Teensy, define manually by build define via editing platform.txt directly in %applocaldata%\Arduino15\packages\teensy\hardware\avr\{version}\ and adding (/w space & quotes):  "-DBOARD={build.board}" to end of recipe.cpp.o.pattern=
#define BOARD                           "TEENSY"
#elif defined(ARDUINO_BOARD)
#define BOARD                           ARDUINO_BOARD
#elif defined(BOARD_NAME)
#define BOARD                           BOARD_NAME
#elif defined(USB_PRODUCT)
#define BOARD                           USB_PRODUCT
#else
#define BOARD                           "OTHER"
#endif
#endif
#ifndef HAS_INPUT_PULLDOWN                                  // Resolve for INPUT_PULLDOWN availability
#if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ARM) || defined(ARDUINO_ARCH_MBED) || defined(ARDUINO_ARCH_RP2040) || defined(ESP32) || defined(ARDUINO_ARCH_STM32) || defined(CORE_TEENSY)
#define HAS_INPUT_PULLDOWN              true                // INPUT_PULLDOWN likely available (INPUT able to be PULLDOWN/floating/PULLUP)
#else
#define HAS_INPUT_PULLDOWN              false               // INPUT_PULLDOWN likely unavailable (INPUT restricted to just floating/PULLUP)
#endif
#endif
#ifndef HAS_LARGE_SRAM                                      // Resolve for large SRAM size availability
#if defined(ARDUINO_SAM_DUE) || defined(ARDUINO_ARCH_ARM) || defined(ARDUINO_ARCH_MBED)  || defined(ARDUINO_ARCH_RP2040) || defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_STM32) || defined(CORE_TEENSY)
#define HAS_LARGE_SRAM                  true                // Large SRAM size likely available (memory saving features disabled)
#else
#define HAS_LARGE_SRAM                  false               // Large SRAM size likely unavailable (memory saving features enabled)
#endif
#endif

// Standardized gfx WxH
#ifdef HYDRO_USE_GUI
#include <User_Setup.h>
#endif
#ifndef TFT_GFX_WIDTH
#ifdef TFT_WIDTH
#define TFT_GFX_WIDTH                   TFT_WIDTH           // Custom defined
#else
#define TFT_GFX_WIDTH                   240                 // Default 240x
#endif
#endif
#ifndef TFT_GFX_HEIGHT
#ifdef TFT_HEIGHT
#define TFT_GFX_HEIGHT                  TFT_HEIGHT          // Custom defined
#else
#define TFT_GFX_HEIGHT                  320                 // Default x320
#endif
#endif

typedef typeof(millis())                millis_t;           // Millisecond time type
typedef int8_t                          hposi_t;            // Position indexing type alias
typedef uint32_t                        hkey_t;             // Key type alias, for hashing
typedef int8_t                          hid_t;              // Id type alias, for RTTI
typedef uint16_t                        hframe_t;           // Polling frame type, for sync
#define millis_none                     ((millis_t)0)       // No millis defined/none placeholder
#define hposi_none                      ((hposi_t)-1)       // No position defined/none placeholder
#define hkey_none                       ((hkey_t)-1)        // No key defined/none placeholder
#define hid_none                        ((hid_t)-1)         // No id defined/none placeholder
#define hframe_none                     ((hframe_t)0)       // No frame defined/none placeholder
#define hpin_none                       ((pintype_t)-1)     // No pin defined/none placeholder
#define hpin_virtual                    ((pintype_t)100)    // Virtual pin section start placeholder
#define hpinchnl_none                   ((int8_t)-127)      // No pin channel defined/none placeholder
typedef typeof(INPUT)                   ard_pinmode_t;      // Arduino pin mode type alias
typedef typeof(LOW)                     ard_pinstatus_t;    // Arduino pin status type alias

// The following sizes apply to all architectures
#define HYDRO_PREFIX_MAXSIZE            16                  // Prefix names maximum size (for logging/publishing)
#define HYDRO_NAME_MAXSIZE              24                  // Naming character maximum size (system name, crop name, etc.)
#define HYDRO_POS_MAXSIZE               32                  // Position indicies maximum size (max # of objs of same type)
#define HYDRO_URL_MAXSIZE               64                  // URL string maximum size (max url length)
#define HYDRO_JSON_DOC_SYSSIZE          256                 // JSON document chunk data bytes for reading in main system data (serialization buffer size)
#define HYDRO_JSON_DOC_DEFSIZE          192                 // Default JSON document chunk data bytes (serialization buffer size)
#define HYDRO_STRING_BUFFER_SIZE        32                  // Size in bytes of string serialization buffers
#define HYDRO_WIFISTREAM_BUFFER_SIZE    128                 // Size in bytes of WiFi serialization buffers
// The following sizes only apply to architectures that do not have STL support (AVR/SAM)
#define HYDRO_DEFAULT_MAXSIZE           8                   // Default maximum array/map size
#define HYDRO_ACTUATOR_SIGNAL_SLOTS     4                   // Maximum number of slots for actuator's activation signal
#define HYDRO_SENSOR_SIGNAL_SLOTS       6                   // Maximum number of slots for sensor's measurement signal
#define HYDRO_TRIGGER_SIGNAL_SLOTS      4                   // Maximum number of slots for trigger's state signal
#define HYDRO_BALANCER_SIGNAL_SLOTS     2                   // Maximum number of slots for balancer's state signal
#define HYDRO_LOG_SIGNAL_SLOTS          2                   // Maximum number of slots for system log signal
#define HYDRO_PUBLISH_SIGNAL_SLOTS      2                   // Maximum number of slots for data publish signal
#define HYDRO_RESERVOIR_SIGNAL_SLOTS    2                   // Maximum number of slots for filled/empty signal
#define HYDRO_FEEDING_SIGNAL_SLOTS      2                   // Maximum number of slots for crop feed signal
#define HYDRO_RAIL_SIGNAL_SLOTS         8                   // Maximum number of slots for rail capacity signal
#define HYDRO_SYS_OBJECTS_MAXSIZE       16                  // Maximum array size for system objects (max # of objects in system)
#define HYDRO_CROPS_CROPSLIB_MAXSIZE    8                   // Maximum array size for crops library objects (max # of different kinds of crops)
#define HYDRO_CAL_CALIBS_MAXSIZE        8                   // Maximum array size for calibration store objects (max # of different custom calibrations)
#define HYDRO_OBJ_LINKS_MAXSIZE         8                   // Maximum array size for object linkage list, per obj (max # of linked objects)
#define HYDRO_BAL_ACTUATORS_MAXSIZE     8                   // Maximum array size for balancer actuators list (max # of actuators used)
#define HYDRO_SCH_REQACTS_MAXSIZE       4                   // Maximum array size for scheduler required actuators list (max # of actuators active per process stage)
#define HYDRO_SCH_PROCS_MAXSIZE         2                   // Maximum array size for scheduler feeding/lighting process lists (max # of feed reservoirs)
#define HYDRO_SYS_ONEWIRES_MAXSIZE      2                   // Maximum array size for pin OneWire list (max # of OneWire comm pins)
#define HYDRO_SYS_PINLOCKS_MAXSIZE      2                   // Maximum array size for pin locks list (max # of locks)
#define HYDRO_SYS_PINMUXERS_MAXSIZE     2                   // Maximum array size for pin muxers list (max # of muxers)
#define HYDRO_SYS_PINEXPANDERS_MAXSIZE  2                   // Maximum array size for pin expanders list (max # of expanders)

#define HYDRO_CONTROL_LOOP_INTERVAL     100                 // Run interval of main control loop, in milliseconds
#define HYDRO_DATA_LOOP_INTERVAL        2000                // Default run interval of data loop, in milliseconds (customizable later)
#define HYDRO_MISC_LOOP_INTERVAL        250                 // Run interval of misc loop, in milliseconds

#define HYDRO_ACT_PUMPCALC_UPDATEMS     250                 // Minimum time millis needing to pass before a pump reports/writes changed volume to reservoir (reduces error accumulation)
#define HYDRO_ACT_PUMPCALC_MINFLOWRATE  0.05f               // What percentage of continuous flow rate an instantaneous flow rate sensor must achieve before it is used in pump/volume calculations (reduces near-zero error jitters)

#define HYDRO_CROPS_LINKS_BASESIZE      1                   // Base array size for crop's linkage list
#define HYDRO_CROPS_GROWWEEKS_MAX       16                  // Maximum grow weeks to support scheduling up to
#define HYDRO_CROPS_GROWWEEKS_MIN       8                   // Minimum grow weeks to support scheduling up to
#define HYDRO_CROPS_FRACTION_DEHYD      0.01f               // What % of soil moisture is considered dehydrated such that feeding state is forced in needsFeeding() checks, used in addition to adaptive crop's feeding trigger

#define HYDRO_DOSETIME_FRACTION_MIN     0.5f                // What percentage of base dosing time autodosers can scale down to, if estimated dosing time could exceed setpoint
#define HYDRO_DOSETIME_FRACTION_MAX     1.5f                // What percentage of base dosing time autodosers can scale up to, if estimated dosing time remaining could fall short of setpoint

#define HYDRO_FLUIDRES_LINKS_BASESIZE   1                   // Base array size for fluid reservoir's linkage list
#define HYDRO_FEEDRES_LINKS_BASESIZE    4                   // Base array size for feed reservoir's linkage list
#define HYDRO_FEEDRES_FRACTION_EMPTY    0.2f                // What fraction of a feed reservoir's volume is to be considered 'empty' during7*- pumping/feedings (to account for pumps, heaters, etc. - only used for feed reservoirs with volume tracking but no filled/empty triggers)
#define HYDRO_FEEDRES_FRACTION_FILLED   0.9f                // What fraction of a feed reservoir's volume to top-off to/considered 'filled' during pumping/feedings (rest will be used for balancing - only used for feed reservoirs with volume tracking but no filled/empty triggers)

#define HYDRO_MUXERS_SHARED_ADDR_BUS    false               // Pin muxer channel selects should disable all pin muxers due to using same address bus (true), or not (false)

#define HYDRO_NIGHT_START_HR            20                  // Hour of the day night starts (for night feeding multiplier, used if not able to calculate from location & time)
#define HYDRO_NIGHT_FINISH_HR           6                   // Hour of the day night finishes (for night feeding multiplier, used if not able to calculate from location & time)

#define HYDRO_POS_SEARCH_FROMBEG        -1                  // Search from beginning to end, 0 up to MAXSIZE-1
#define HYDRO_POS_SEARCH_FROMEND        HYDRO_POS_MAXSIZE   // Search from end to beginning, MAXSIZE-1 down to 0
#define HYDRO_POS_EXPORT_BEGFROM        1                   // Whenever exported/user-facing position indexing starts at 1 or 0 (aka display offset)

#define HYDRO_RANGE_PH_HALF             1.0f                // How far to go, in either direction, to form a range when pH is expressed as a single number, in pH (note: this also controls auto-balancer ranges)
#define HYDRO_RANGE_EC_HALF             0.5f                // How far to go, in either direction, to form a range when TDS/EC is expressed as a single number, in EC (note: this also controls auto-balancer ranges)
#define HYDRO_RANGE_TEMP_HALF           5.0f                // How far to go, in either direction, to form a range when Temp is expressed as a single number, in C (note: this also controls auto-balancer ranges)
#define HYDRO_RANGE_CO2_HALF            100.0f              // How far to go, in either direction, to form a range when CO2 is expressed as a single number, in PPM (note: this also controls auto-balancer ranges)

#define HYDRO_RAILS_LINKS_BASESIZE      4                   // Base array size for rail's linkage list
#define HYDRO_RAILS_FRACTION_SATURATED  0.8f                // What fraction of maximum power is allowed to be used in canActivate() checks (aka maximum saturation point), used in addition to regulated rail's limitTrigger

#define HYDRO_SCH_FEED_FRACTION         0.8f                // What percentage of crops need to have their feeding signal turned on/off for scheduler to act on such as a whole
#define HYDRO_SCH_BALANCE_MINTIME       30                  // Minimum time, in seconds, that all balancers must register as balanced for until balancing is marked as completed
#define HYDRO_SCH_AERATORS_FEEDRUN      true                // If aerators should be continued to be ran during feeding, after pre-feeding aeration is finished

#define HYDRO_SENSOR_ANALOGREAD_SAMPLES 5                   // Number of samples to take for any analogRead call inside of a sensor's takeMeasurement call, or 0 to disable sampling (note: bitRes.maxValue * # of samples must fit inside a uint32_t)
#define HYDRO_SENSOR_ANALOGREAD_DELAY   0                   // Delay time between samples, or 0 to disable delay, in milliseconds

#define HYDRO_SYS_AUTOSAVE_INTERVAL     120                 // Default autosave interval, in minutes
#define HYDRO_SYS_I2CEEPROM_BASEADDR    0x50                // Base address of I2C EEPROM (bitwise or'ed with passed address)
#define HYDRO_SYS_ATWIFI_SERIALBAUD     115200              // Data baud rate for serial AT WiFi, in bps (older modules may need 9600)
#define HYDRO_SYS_ATWIFI_SERIALMODE     SERIAL_8N1          // Data transfer mode for serial AT WiFi (see SERIAL_* defines)
#define HYDRO_SYS_NMEAGPS_SERIALBAUD    9600                // Data baud rate for serial NMEA GPS, in bps (older modules may need 4800)
#define HYDRO_SYS_URLHTTP_PORT          80                  // Which default port to access when accessing HTTP resources
#define HYDRO_SYS_LEAVE_FILES_OPEN      !defined(__AVR__)   // If high access files should be left open to improve performance (true), or closed after use to reduce memory consumption (false)
#define HYDRO_SYS_FREERAM_LOWBYTES      1024                // How many bytes of free memory left spawns a handle low mem call to all objects
#define HYDRO_SYS_FREESPACE_INTERVAL    240                 // How many minutes should pass before checking attached file systems have enough disk space (performs cleanup if not)
#define HYDRO_SYS_FREESPACE_LOWSPACE    256                 // How many kilobytes of disk space remaining will force cleanup of oldest log/data files first
#define HYDRO_SYS_FREESPACE_DAYSBACK    180                 // How many days back log/data files are allowed to be stored up to (any beyond this are deleted during cleanup)
#define HYDRO_SYS_SUNRISESET_CALCITERS  3                   // # of iterations that sunrise/sunset calculations should run (higher # = more accurate but also more costly)
#define HYDRO_SYS_LATLONG_DISTSQRDTOL   0.25                // Squared difference in lat/long coords that needs to occur for it to be considered significant enough for system update
#define HYDRO_SYS_ALTITUDE_DISTTOL      0.5                 // Difference in altitude coords that needs to occur for it to be considered significant enough for system update
#define HYDRO_SYS_DELAYFINE_SPINMILLIS  20                  // How many milliseconds away from stop time fine delays can use yield() up to before using a blocking spin-lock (used for fine timing)
#define HYDRO_SYS_YIELD_AFTERMILLIS     20                  // How many milliseconds must pass by before system run loops call a yield() mid-loop, in order to allow finely-timed tasks a chance to run
#define HYDRO_SYS_DEBUGOUT_FLUSH_YIELD  false               // If debug output statements should flush and yield afterwards to force send through to serial monitor (mainly used for debugging)
#define HYDRO_SYS_MEM_LOGGING_ENABLE    false               // If system will periodically log memory remaining messages (mainly used for debugging)
#define HYDRO_SYS_DRY_RUN_ENABLE        false               // Disables pins from actually enabling in order to simply simulate (mainly used for debugging)

#if defined(__APPLE__) || defined(__APPLE) || defined(__unix__) || defined(__unix)
#define HYDRO_BLDPATH_SEPARATOR         '/'                 // Path separator for nix-based build machines
#else
#define HYDRO_BLDPATH_SEPARATOR         '\\'                // Path separator for win-based build machines
#endif
#define HYDRO_FSPATH_SEPARATOR          '/'                 // Path separator for filesystem paths (SD card/WiFiStorage)
#define HYDRO_URLPATH_SEPARATOR         '/'                 // Path separator for URL paths

#if HYDRO_SYS_LEAVE_FILES_OPEN                              // How subsequent getters should be called when file left open
#define HYDRO_LOFS_BEGIN false
#else
#define HYDRO_LOFS_BEGIN true
#endif


// EEPROM Device Type Enumeration
enum Hydro_EEPROMType : signed short {
    Hydro_EEPROMType_AT24LC01 = I2C_DEVICESIZE_24LC01 >> 7,     // AT24LC01 (1k bits, 128 bytes), 7-bit address space
    Hydro_EEPROMType_AT24LC02 = I2C_DEVICESIZE_24LC02 >> 7,     // AT24LC02 (2k bits, 256 bytes), 8-bit address space
    Hydro_EEPROMType_AT24LC04 = I2C_DEVICESIZE_24LC04 >> 7,     // AT24LC04 (4k bits, 512 bytes), 9-bit address space
    Hydro_EEPROMType_AT24LC08 = I2C_DEVICESIZE_24LC08 >> 7,     // AT24LC08 (8k bits, 1024 bytes), 10-bit address space
    Hydro_EEPROMType_AT24LC16 = I2C_DEVICESIZE_24LC16 >> 7,     // AT24LC16 (16k bits, 2048 bytes), 11-bit address space
    Hydro_EEPROMType_AT24LC32 = I2C_DEVICESIZE_24LC32 >> 7,     // AT24LC32 (32k bits, 4096 bytes), 12-bit address space
    Hydro_EEPROMType_AT24LC64 = I2C_DEVICESIZE_24LC64 >> 7,     // AT24LC64 (64k bits, 8192 bytes), 13-bit address space
    Hydro_EEPROMType_AT24LC128 = I2C_DEVICESIZE_24LC128 >> 7,   // AT24LC128 (128k bits, 16384 bytes), 14-bit address space
    Hydro_EEPROMType_AT24LC256 = I2C_DEVICESIZE_24LC256 >> 7,   // AT24LC256 (256k bits, 32768 bytes), 15-bit address space
    Hydro_EEPROMType_AT24LC512 = I2C_DEVICESIZE_24LC512 >> 7,   // AT24LC512 (512k bits, 65536 bytes), 16-bit address space
    Hydro_EEPROMType_None = -1,                                 // No EEPROM

    Hydro_EEPROMType_Bits_1k = Hydro_EEPROMType_AT24LC01,       // 1k bits (alias of AT24LC01)
    Hydro_EEPROMType_Bits_2k = Hydro_EEPROMType_AT24LC02,       // 2k bits (alias of AT24LC02)
    Hydro_EEPROMType_Bits_4k = Hydro_EEPROMType_AT24LC04,       // 4k bits (alias of AT24LC04)
    Hydro_EEPROMType_Bits_8k = Hydro_EEPROMType_AT24LC08,       // 8k bits (alias of AT24LC08)
    Hydro_EEPROMType_Bits_16k = Hydro_EEPROMType_AT24LC16,      // 16k bits (alias of AT24LC16)
    Hydro_EEPROMType_Bits_32k = Hydro_EEPROMType_AT24LC32,      // 32k bits (alias of AT24LC32)
    Hydro_EEPROMType_Bits_64k = Hydro_EEPROMType_AT24LC64,      // 64k bits (alias of AT24LC64)
    Hydro_EEPROMType_Bits_128k = Hydro_EEPROMType_AT24LC128,    // 128k bits (alias of AT24LC128)
    Hydro_EEPROMType_Bits_256k = Hydro_EEPROMType_AT24LC256,    // 256k bits (alias of AT24LC256)
    Hydro_EEPROMType_Bits_512k = Hydro_EEPROMType_AT24LC512,    // 512k bits (alias of AT24LC512)

    Hydro_EEPROMType_Bytes_128 = Hydro_EEPROMType_AT24LC01,     // 128 bytes (alias of AT24LC01)
    Hydro_EEPROMType_Bytes_256 = Hydro_EEPROMType_AT24LC02,     // 256 bytes (alias of AT24LC02)
    Hydro_EEPROMType_Bytes_512 = Hydro_EEPROMType_AT24LC04,     // 512 bytes (alias of AT24LC04)
    Hydro_EEPROMType_Bytes_1024 = Hydro_EEPROMType_AT24LC08,    // 1024 bytes (alias of AT24LC08)
    Hydro_EEPROMType_Bytes_2048 = Hydro_EEPROMType_AT24LC16,    // 2048 bytes (alias of AT24LC16)
    Hydro_EEPROMType_Bytes_4096 = Hydro_EEPROMType_AT24LC32,    // 4096 bytes (alias of AT24LC32)
    Hydro_EEPROMType_Bytes_8192 = Hydro_EEPROMType_AT24LC64,    // 8192 bytes (alias of AT24LC64)
    Hydro_EEPROMType_Bytes_16384 = Hydro_EEPROMType_AT24LC128,  // 16384 bytes (alias of AT24LC128)
    Hydro_EEPROMType_Bytes_32768 = Hydro_EEPROMType_AT24LC256,  // 32768 bytes (alias of AT24LC256)
    Hydro_EEPROMType_Bytes_65536 = Hydro_EEPROMType_AT24LC512   // 65536 bytes (alias of AT24LC512)
};

// RTC Device Type Enumeration
enum Hydro_RTCType : signed char {
    Hydro_RTCType_DS1307 = 13,                              // DS1307 (no battFail)
    Hydro_RTCType_DS3231 = 32,                              // DS3231
    Hydro_RTCType_PCF8523 = 85,                             // PCF8523
    Hydro_RTCType_PCF8563 = 86,                             // PCF8563
    Hydro_RTCType_None = -1                                 // No RTC
};

// DHT Device Type Enumeration
enum Hydro_DHTType : signed char {
    Hydro_DHTType_DHT11 = DHT11,                            // DHT11 (using DHT library)
    Hydro_DHTType_DHT12 = DHT12,                            // DHT12 (using DHT library)
    Hydro_DHTType_DHT21 = DHT21,                            // DHT21 (using DHT library)
    Hydro_DHTType_DHT22 = DHT22,                            // DHT22 (using DHT library)
    //Hydro_DHTType_BME280 = 28,                              // BME280 (using BME280 library, TODO)
    Hydro_DHTType_None = -1,                                // No DHT

    Hydro_DHTType_AM2301 = AM2301                           // AM2301 (alias of DHT21)
};


// Crop Type
// Common crop types. Controls what pH, EC, etc. that a plant prefers.
enum Hydro_CropType : signed char {
    Hydro_CropType_AloeVera,                                // Aloe Vera crop
    Hydro_CropType_Anise,                                   // Anise crop
    Hydro_CropType_Artichoke,                               // Artichoke crop
    Hydro_CropType_Arugula,                                 // Arugula crop
    Hydro_CropType_Asparagus,                               // Asparagus crop
    Hydro_CropType_Basil,                                   // Basil crop
    Hydro_CropType_Bean,                                    // Bean (common) crop
    Hydro_CropType_BeanBroad,                               // Bean (broad) crop
    Hydro_CropType_Beetroot,                                // Beetroot crop
    Hydro_CropType_BlackCurrant,                            // Black Currant crop
    Hydro_CropType_Blueberry,                               // Blueberry crop
    Hydro_CropType_BokChoi,                                 // Bok-choi crop
    Hydro_CropType_Broccoli,                                // Broccoli crop
    Hydro_CropType_BrusselsSprout,                          // Brussels Sprout crop
    Hydro_CropType_Cabbage,                                 // Cabbage crop
    Hydro_CropType_Cannabis,                                // Cannabis (generic) crop
    Hydro_CropType_Capsicum,                                // Capsicum crop
    Hydro_CropType_Carrots,                                 // Carrots crop
    Hydro_CropType_Catnip,                                  // Catnip crop
    Hydro_CropType_Cauliflower,                             // Cauliflower crop
    Hydro_CropType_Celery,                                  // Celery crop
    Hydro_CropType_Chamomile,                               // Chamomile crop
    Hydro_CropType_Chicory,                                 // Chiccory crop
    Hydro_CropType_Chives,                                  // Chives crop
    Hydro_CropType_Cilantro,                                // Cilantro crop
    Hydro_CropType_Coriander,                               // Coriander crop
    Hydro_CropType_CornSweet,                               // Corn (sweet) crop
    Hydro_CropType_Cucumber,                                // Cucumber crop
    Hydro_CropType_Dill,                                    // Dill crop
    Hydro_CropType_Eggplant,                                // Eggplant crop
    Hydro_CropType_Endive,                                  // Endive crop
    Hydro_CropType_Fennel,                                  // Fennel crop
    Hydro_CropType_Fodder,                                  // Fodder crop
    Hydro_CropType_Flowers,                                 // Flowers (generic) crop
    Hydro_CropType_Garlic,                                  // Garlic crop
    Hydro_CropType_Ginger,                                  // Ginger crop
    Hydro_CropType_Kale,                                    // Kale crop
    Hydro_CropType_Lavender,                                // Lavender crop
    Hydro_CropType_Leek,                                    // Leek crop
    Hydro_CropType_LemonBalm,                               // Lemon Balm crop
    Hydro_CropType_Lettuce,                                 // Lettuce crop
    Hydro_CropType_Marrow,                                  // Marrow crop
    Hydro_CropType_Melon,                                   // Melon crop
    Hydro_CropType_Mint,                                    // Mint crop
    Hydro_CropType_MustardCress,                            // Mustard Cress crop
    Hydro_CropType_Okra,                                    // Okra crop
    Hydro_CropType_Onions,                                  // Onions crop
    Hydro_CropType_Oregano,                                 // Oregano crop
    Hydro_CropType_PakChoi,                                 // Pak-choi crop
    Hydro_CropType_Parsley,                                 // Parsley crop
    Hydro_CropType_Parsnip,                                 // Parsnip crop
    Hydro_CropType_Pea,                                     // Pea (common) crop
    Hydro_CropType_PeaSugar,                                // Pea (sugar) crop
    Hydro_CropType_Pepino,                                  // Pepino crop
    Hydro_CropType_PeppersBell,                             // Peppers (bell) crop
    Hydro_CropType_PeppersHot,                              // Peppers (hot) crop
    Hydro_CropType_Potato,                                  // Potato (common) crop
    Hydro_CropType_PotatoSweet,                             // Potato (sweet) crop
    Hydro_CropType_Pumpkin,                                 // Pumpkin crop
    Hydro_CropType_Radish,                                  // Radish crop
    Hydro_CropType_Rhubarb,                                 // Rhubarb crop
    Hydro_CropType_Rosemary,                                // Rosemary crop
    Hydro_CropType_Sage,                                    // Sage crop
    Hydro_CropType_Silverbeet,                              // Silverbeet crop
    Hydro_CropType_Spinach,                                 // Spinach crop
    Hydro_CropType_Squash,                                  // Squash crop
    Hydro_CropType_Sunflower,                               // Sunflower crop
    Hydro_CropType_Strawberries,                            // Strawberries crop
    Hydro_CropType_SwissChard,                              // Swiss Chard crop
    Hydro_CropType_Taro,                                    // Taro crop
    Hydro_CropType_Tarragon,                                // Tarragon crop
    Hydro_CropType_Thyme,                                   // Thyme crop
    Hydro_CropType_Tomato,                                  // Tomato crop
    Hydro_CropType_Turnip,                                  // Turnip crop
    Hydro_CropType_Watercress,                              // Watercress crop
    Hydro_CropType_Watermelon,                              // Watermelon crop
    Hydro_CropType_Zucchini,                                // Zucchini crop

    // Custom crops allow customized parameters for crops to be programmed into the system.

    Hydro_CropType_CustomCrop1,                             // Custom crop 1
    Hydro_CropType_CustomCrop2,                             // Custom crop 2
    Hydro_CropType_CustomCrop3,                             // Custom crop 3
    Hydro_CropType_CustomCrop4,                             // Custom crop 4
    Hydro_CropType_CustomCrop5,                             // Custom crop 5
    Hydro_CropType_CustomCrop6,                             // Custom crop 6
    Hydro_CropType_CustomCrop7,                             // Custom crop 7
    Hydro_CropType_CustomCrop8,                             // Custom crop 8

    Hydro_CropType_Count,                                   // Placeholder
    Hydro_CropType_CustomCropCount = 8,                     // Placeholder
    Hydro_CropType_Undefined = -1                           // Placeholder
};

// Substrate Type
// Common substrate types. Influences feeding scheduling and environment control (TODO).
enum Hydro_SubstrateType : signed char {
    Hydro_SubstrateType_ClayPebbles,                        // Expanded clay pebbles substrate
    Hydro_SubstrateType_CoconutCoir,                        // Coconut coir (aka coco peat) substrate
    Hydro_SubstrateType_Rockwool,                           // Rockwool substrate

    Hydro_SubstrateType_Count,                              // Placeholder
    Hydro_SubstrateType_Undefined = -1                      // Placeholder
};

// Crop Phase
// Common phases of crops. Influences feeding scheduling and environment control.
enum Hydro_CropPhase : signed char {
    Hydro_CropPhase_Seedling,                               // Initial seedling stage
    Hydro_CropPhase_Vegetative,                             // Vegetative stage
    Hydro_CropPhase_Blooming,                               // Flowering stage
    Hydro_CropPhase_Harvest,                                // Harvest stage

    Hydro_CropPhase_Count,                                  // Placeholder
    Hydro_CropPhase_MainCount = 3,                          // Placeholder
    Hydro_CropPhase_Undefined = -1                          // Placeholder
};

// System Run Mode
// Specifies the general tank setup, fluid levels, and waste connection defaults.
enum Hydro_SystemMode : signed char {
    Hydro_SystemMode_Recycling,                             // System consistently recycles water in main feed water reservoir. Default setting, applicable to a wide range of NFT and DWC setups.
    Hydro_SystemMode_DrainToWaste,                          // System refills feed reservoir every time before feeding (with pH balance/feed premix top off), and requires a drainage pump (from feed reservoir) and/or drainage pipe (as drainage/feed pump output). Suitable also for flood-based systems.

    Hydro_SystemMode_Count,                                 // Placeholder
    Hydro_SystemMode_Undefined = -1                         // Placeholder
};

// Measurement Units Mode
// Specifies the standard of measurement style that units will use.
enum Hydro_MeasurementMode : signed char {
    Hydro_MeasurementMode_Imperial,                         // Imperial measurement mode (default setting, °F Ft Gal Lbs M-D-Y Val.X etc)
    Hydro_MeasurementMode_Metric,                           // Metric measurement mode (°C M L Kg Y-M-D Val.X etc)
    Hydro_MeasurementMode_Scientific,                       // Scientific measurement mode (°K M L Kg Y-M-D Val.XX etc)

    Hydro_MeasurementMode_Count,                            // Placeholder
    Hydro_MeasurementMode_Undefined = -1,                   // Placeholder
    Hydro_MeasurementMode_Default = Hydro_MeasurementMode_Metric // Default system measurement mode (alias, feel free to change)
};

// LCD/Display Output Mode
// Specifies what kind of visual output device is to be used.
// Display output mode support provided by tcMenu.
enum Hydro_DisplayOutputMode : signed char {
    Hydro_DisplayOutputMode_Disabled,                       // No display output
    Hydro_DisplayOutputMode_LCD16x2_EN,                     // 16x2 text LCD (with EN first, pins: {EN,RW,RS,BL,Data}), using LiquidCrystalIO (i2c only)
    Hydro_DisplayOutputMode_LCD16x2_RS,                     // 16x2 text LCD (with RS first, pins: {RS,RW,EN,BL,Data}), using LiquidCrystalIO (i2c only)
    Hydro_DisplayOutputMode_LCD20x4_EN,                     // 20x4 text LCD (with EN first, pins: {EN,RW,RS,BL,Data}), using LiquidCrystalIO (i2c only)
    Hydro_DisplayOutputMode_LCD20x4_RS,                     // 20x4 text LCD (with RS first, pins: {RS,RW,EN,BL,Data}), using LiquidCrystalIO (i2c only)
    Hydro_DisplayOutputMode_SSD1305,                        // SSD1305 128x32 OLED, using U8g2 (i2c or SPI)
    Hydro_DisplayOutputMode_SSD1305_x32Ada,                 // Adafruit SSD1305 128x32 OLED, using U8g2 (i2c or SPI)
    Hydro_DisplayOutputMode_SSD1305_x64Ada,                 // Adafruit SSD1305 128x64 OLED, using U8g2 (i2c or SPI)
    Hydro_DisplayOutputMode_SSD1306,                        // SSD1306 128x64 OLED, using U8g2 (i2c or SPI)
    Hydro_DisplayOutputMode_SH1106,                         // SH1106 128x64 OLED, using U8g2 (i2c or SPI)
    Hydro_DisplayOutputMode_CustomOLED,                     // Custom OLED, using U8g2 (i2c or SPI, note: custom device/size defined statically by HYDRO_UI_CUSTOM_OLED_I2C / HYDRO_UI_CUSTOM_OLED_SPI)
    Hydro_DisplayOutputMode_SSD1607,                        // SSD1607 200x200 OLED, using U8g2 (SPI only)
    Hydro_DisplayOutputMode_IL3820,                         // IL3820 296x128 OLED, using U8g2 (SPI only)
    Hydro_DisplayOutputMode_IL3820_V2,                      // IL3820 V2 296x128 OLED, using U8g2 (SPI only)
    Hydro_DisplayOutputMode_ST7735,                         // ST7735 graphical LCD, using AdafruitGFX (SPI only, note: usage requires correct tag color enumeration setting)
    Hydro_DisplayOutputMode_ST7789,                         // ST7789 graphical LCD, using AdafruitGFX (SPI only, note: usage requires correct screen resolution enumeration setting, CustomTFT size defined statically by TFT_GFX_WIDTH & TFT_GFX_HEIGHT)
    Hydro_DisplayOutputMode_ILI9341,                        // ILI9341 graphical LCD, using AdafruitGFX (SPI only)
    Hydro_DisplayOutputMode_TFT,                            // Graphical LCD, using TFT_eSPI (SPI only, note: usage requires editing TFT_eSPI/User_Setup.h & properly defining TFT_CS, TFT_DC, & TFT_RST, size defined statically by TFT_GFX_WIDTH & TFT_GFX_HEIGHT which defaults to TFT_WIDTH & TFT_HEIGHT)

    Hydro_DisplayOutputMode_Count,                          // Placeholder
    Hydro_DisplayOutputMode_Undefined = -1                  // Placeholder
};

// Control Input Mode
// Specifies what kind of control input mode (and pin ribbon set) is to be used.
// Control input mode support provided by tcMenu.
enum Hydro_ControlInputMode : signed char {
    Hydro_ControlInputMode_Disabled,                        // No control input
    Hydro_ControlInputMode_RotaryEncoderOk,                 // Rotary encoder /w momentary Ok button, pins: {eA,eB,Ok}
    Hydro_ControlInputMode_RotaryEncoderOkLR,               // Rotary encoder /w momentary Ok, Back(L), and Next(R) buttons, pins: {eA,eB,Ok,Bk,Nx}
    Hydro_ControlInputMode_UpDownButtonsOk,                 // Momentary Up, Down, and Ok buttons, pins: {Up,Dw,Ok}
    Hydro_ControlInputMode_UpDownButtonsOkLR,               // Momentary Up, Down, Ok, Back(L), and Next(R) buttons, pins: {Up,Dw,Ok,Bk,Nx}
    Hydro_ControlInputMode_UpDownESP32TouchOk,              // ESP32-Touch Up, Down, and Ok keys, pins: {Up,Dw,Ok}
    Hydro_ControlInputMode_UpDownESP32TouchOkLR,            // ESP32-Touch Up, Down, Ok, Back(L), and Next(R) keys, pins: {Up,Dw,Ok,Bk,Nx}
    Hydro_ControlInputMode_AnalogJoystickOk,                // Analog joystick /w momentary Ok button, pins: {aX,aY,Ok} (aX can be unused/-1, else used for back/next)
    Hydro_ControlInputMode_Matrix2x2UpDownButtonsOkL,       // 2x2 matrix keypad as momentary Up, Down, Ok, and Back(L) buttons, pins: {r0,r1,c0,c1}
    Hydro_ControlInputMode_Matrix3x4Keyboard_OptRotEncOk,   // 3x4 numeric matrix keyboard, & optional rotary encoder /w momentary Ok button, pins: {r0,r1,r2,r3,c0,c1,c2,eA,eB,Ok} (eA can be unused/-1)
    Hydro_ControlInputMode_Matrix3x4Keyboard_OptRotEncOkLR, // 3x4 numeric matrix keyboard, & optional rotary encoder /w momentary Ok, Back(L), and Next(R) buttons, pins: {r0,r1,r2,r3,c0,c1,c2,eA,eB,Ok,Bk,Nx} (eA can be unused/-1)
    Hydro_ControlInputMode_Matrix4x4Keyboard_OptRotEncOk,   // 4x4 alpha-numeric matrix keyboard, & optional rotary encoder /w momentary Ok button, pins: {r0,r1,r2,r3,c0,c1,c2,c3,eA,eB,Ok} (eA can be unused/-1)
    Hydro_ControlInputMode_Matrix4x4Keyboard_OptRotEncOkLR, // 4x4 alpha-numeric matrix keyboard, & optional rotary encoder /w momentary Ok, Back(L), and Next(R) buttons, pins: {r0,r1,r2,r3,c0,c1,c2,c3,eA,eB,Ok,Bk,Nx} (eA can be unused/-1)
    Hydro_ControlInputMode_ResistiveTouch,                  // Resistive touchscreen, pins: {X+,X-,Y+,Y-} (X-/Y- analog, X+/Y+ digital)
    Hydro_ControlInputMode_TouchScreen,                     // Full touchscreen (FT6206, or XPT2046 /w setup define), pins: FT6206: {}, XPT2046: {tCS,tIRQ} (tIRQ can be unused/-1, FT6206 hard-coded to use Wire for i2c)
    Hydro_ControlInputMode_TFTTouch,                        // TFT Touch (XPT2046), using TFT_eSPI (Note: usage requires TFT display mode & editing TFT_eSPI/User_Setup.h & properly defining TOUCH_CS), pins: {tCS,tIRQ} (tIRQ can be unused/-1)
    Hydro_ControlInputMode_RemoteControl,                   // Remote controlled (no input /w possibly disabled display), pins: {}

    Hydro_ControlInputMode_Count,                           // Placeholder
    Hydro_ControlInputMode_Undefined = -1                   // Placeholder
};

// Actuator Type
// Control actuator type. Specifies the various controllable equipment and their usage.
enum Hydro_ActuatorType : signed char {
    Hydro_ActuatorType_FanExhaust,                          // Fan exhaust/circulation (binary/analog, feed reservoir only, uses crop linkages)
    Hydro_ActuatorType_GrowLights,                          // Grow lights (binary)
    Hydro_ActuatorType_PeristalticPump,                     // Peristaltic pump (pump, pH-up/down, nutrient, fresh water, or custom additive reservoirs only)
    Hydro_ActuatorType_WaterAerator,                        // Water aerator (binary, feed reservoir only)
    Hydro_ActuatorType_WaterHeater,                         // Water heater (binary, feed reservoir only)
    Hydro_ActuatorType_WaterPump,                           // Water pump (pump, feed or drainage/main-water pipe reservoirs only)
    Hydro_ActuatorType_WaterSprayer,                        // Water sprayer (binary, feed reservoir only, uses crop linkages - assumes infinite water source)

    Hydro_ActuatorType_Count,                               // Placeholder
    Hydro_ActuatorType_Undefined = -1                       // Placeholder
};

// Sensor Type
// Sensor device type. Specifies the various sensors and the kinds of things they measure.
enum Hydro_SensorType : signed char {
    Hydro_SensorType_AirCarbonDioxide,                      // Air CO2 sensor (analog/digital)
    Hydro_SensorType_AirTempHumidity,                       // Air temperature and humidity sensor (digital)
    Hydro_SensorType_PotentialHydrogen,                     // pH sensor (analog/digital, feed reservoir only)
    Hydro_SensorType_PowerLevel,                            // Power level meter (analog)
    Hydro_SensorType_PumpFlow,                              // Water pump flow hall sensor (analog(PWM))
    Hydro_SensorType_SoilMoisture,                          // Soil moisture sensor (analog/digital)
    Hydro_SensorType_TotalDissolvedSolids,                  // TDS salts electrode sensor (analog/digital, feed reservoir only)
    Hydro_SensorType_WaterHeight,                           // Water height meter (analog)
    Hydro_SensorType_WaterLevel,                            // Water level indicator (binary)
    Hydro_SensorType_WaterTemperature,                      // Submersible water sensor (analog/digital)

    Hydro_SensorType_Count,                                 // Placeholder
    Hydro_SensorType_Undefined = -1                         // Placeholder
};

// Reservoir Type
// Common fluid containers. Specifies the various operational containers.
enum Hydro_ReservoirType : signed char {
    Hydro_ReservoirType_FeedWater,                          // Feed water
    Hydro_ReservoirType_DrainageWater,                      // Drainage water
    Hydro_ReservoirType_NutrientPremix,                     // Base nutrient premix (A or B iff mixed 1:1)
    Hydro_ReservoirType_FreshWater,                         // Fresh water
    Hydro_ReservoirType_PhUpSolution,                       // pH-Up solution
    Hydro_ReservoirType_PhDownSolution,                     // pH-Down solution

    // Custom additives allow specialized weekly feeding schedules to be programmed into the system.

    Hydro_ReservoirType_CustomAdditive1,                    // Custom additive 1 solution
    Hydro_ReservoirType_CustomAdditive2,                    // Custom additive 2 solution
    Hydro_ReservoirType_CustomAdditive3,                    // Custom additive 3 solution
    Hydro_ReservoirType_CustomAdditive4,                    // Custom additive 4 solution
    Hydro_ReservoirType_CustomAdditive5,                    // Custom additive 5 solution
    Hydro_ReservoirType_CustomAdditive6,                    // Custom additive 6 solution
    Hydro_ReservoirType_CustomAdditive7,                    // Custom additive 7 solution
    Hydro_ReservoirType_CustomAdditive8,                    // Custom additive 8 solution
    Hydro_ReservoirType_CustomAdditive9,                    // Custom additive 9 solution
    Hydro_ReservoirType_CustomAdditive10,                   // Custom additive 10 solution
    Hydro_ReservoirType_CustomAdditive11,                   // Custom additive 11 solution
    Hydro_ReservoirType_CustomAdditive12,                   // Custom additive 12 solution
    Hydro_ReservoirType_CustomAdditive13,                   // Custom additive 13 solution
    Hydro_ReservoirType_CustomAdditive14,                   // Custom additive 14 solution
    Hydro_ReservoirType_CustomAdditive15,                   // Custom additive 15 solution
    Hydro_ReservoirType_CustomAdditive16,                   // Custom additive 16 solution

    Hydro_ReservoirType_Count,                              // Placeholder
    Hydro_ReservoirType_CustomAdditiveCount = 16,           // Placeholder
    Hydro_ReservoirType_Undefined = -1                      // Placeholder
};

// Power Rail
// Common power rails. Specifies an isolated operational power rail unit.
enum Hydro_RailType : signed char {
    Hydro_RailType_AC110V,                                  // ~110V AC-based power rail
    Hydro_RailType_AC220V,                                  // ~220V AC-based power rail
    Hydro_RailType_DC3V3,                                   // 3.3v DC-based power rail
    Hydro_RailType_DC5V,                                    // 5v DC-based power rail
    Hydro_RailType_DC12V,                                   // 12v DC-based power rail
    Hydro_RailType_DC24V,                                   // 24v DC-based power rail
    Hydro_RailType_DC48V,                                   // 48v DC-based power rail

    Hydro_RailType_Count,                                   // Placeholder
    Hydro_RailType_Undefined = -1,                          // Placeholder
    Hydro_RailType_DefaultAC = Hydro_RailType_AC110V        // Default AC rating for AC-based power rail (alias, feel free to change)
};

// Pin Mode
// Pin mode setting. Specifies what kind of pin and how it's used.
enum Hydro_PinMode : signed char {
    Hydro_PinMode_Digital_Input_Floating,                   // Digital input pin as floating/no-pull (pull-up/pull-down disabled, used during mux channel select, type alias for INPUT/GPIO_PuPd_NOPULL)
    Hydro_PinMode_Digital_Input_PullUp,                     // Digital input pin with pull-up resistor (default pairing for active-low trigger, type alias for INPUT_PULLUP/GPIO_PuPd_UP)
    Hydro_PinMode_Digital_Input_PullDown,                   // Digital input pin with pull-down resistor (or pull-up disabled if not avail, default pairing for active-high trigger, type alias for INPUT_PULLDOWN/GPIO_PuPd_DOWN)
    Hydro_PinMode_Digital_Output_OpenDrain,                 // Digital output pin with open-drain NPN-based sink (default pairing for active-low trigger, type alias for OUTPUT/GPIO_OType_OD)
    Hydro_PinMode_Digital_Output_PushPull,                  // Digital output pin with push-pull NPN+PNP-based sink+src (default pairing for active-high trigger, type alias for GPIO_OType_PP)
    Hydro_PinMode_Analog_Input,                             // Analog input pin (type alias for INPUT)
    Hydro_PinMode_Analog_Output,                            // Analog output pin (type alias for OUTPUT)

    Hydro_PinMode_Count,                                    // Placeholder
    Hydro_PinMode_Undefined = -1,                           // Placeholder
    Hydro_PinMode_Digital_Input = Hydro_PinMode_Digital_Input_Floating, // Default digital input (alias for Floating, type for INPUT)
    Hydro_PinMode_Digital_Output = Hydro_PinMode_Digital_Output_OpenDrain // Default digital output (alias for OpenDrain, type for OUTPUT)
};

// Enable Mode
// Actuator intensity/enablement calculation mode. Specifies how multiple activations get used together.
enum Hydro_EnableMode : signed char {
    Hydro_EnableMode_Highest,                               // Parallel activation using highest drive intensity
    Hydro_EnableMode_Lowest,                                // Parallel activation using lowest drive intensity
    Hydro_EnableMode_Average,                               // Parallel activation using averaged drive intensities
    Hydro_EnableMode_Multiply,                              // Parallel activation using multiplied drive intensities

    Hydro_EnableMode_InOrder,                               // Serial activation using in-order/fifo-queue drive intensities
    Hydro_EnableMode_RevOrder,                              // Serial activation using reverse-order/lifo-stack drive intensities
    Hydro_EnableMode_DescOrder,                             // Serial activation using highest-to-lowest/descending-order drive intensities
    Hydro_EnableMode_AscOrder,                              // Serial activation using lowest-to-highest/ascending-order drive intensities

    Hydro_EnableMode_Count,                                 // Placeholder
    Hydro_EnableMode_Undefined = -1,                        // Placeholder
    Hydro_EnableMode_Serial = Hydro_EnableMode_InOrder      // Serial activation (alias for InOrder)
};

// Direction Mode
// Actuator intensity application mode. Specifies activation directionality and enablement.
enum Hydro_DirectionMode : signed char {
    Hydro_DirectionMode_Forward,                            // Standard/forward direction mode
    Hydro_DirectionMode_Reverse,                            // Opposite/reverse direction mode
    Hydro_DirectionMode_Stop,                               // Stationary/braking direction mode  (intensity undef)

    Hydro_DirectionMode_Count,                              // Placeholder
    Hydro_DirectionMode_Undefined = -1                      // Placeholder
};

// Trigger Status
// Common trigger statuses. Specifies enablement and tripped state.
enum Hydro_TriggerState : signed char {
    Hydro_TriggerState_Disabled,                            // Triggers disabled (not hooked up)
    Hydro_TriggerState_NotTriggered,                        // Not triggered
    Hydro_TriggerState_Triggered,                           // Triggered

    Hydro_TriggerState_Count,                               // Placeholder
    Hydro_TriggerState_Undefined = -1                       // Placeholder
};

// Balancing State
// Common balancing states. Specifies balance or which direction of imbalance.
enum Hydro_BalancingState : signed char {
    Hydro_BalancingState_TooLow,                            // Too low / reading needs to go higher
    Hydro_BalancingState_Balanced,                          // Balanced state
    Hydro_BalancingState_TooHigh,                           // Too high / reading needs to go lower

    Hydro_BalancingState_Count,                             // Placeholder
    Hydro_BalancingState_Undefined = -1                     // Placeholder
};

// Units Category
// Unit of measurement category. Specifies the kind of unit.
enum Hydro_UnitsCategory : signed char {
    Hydro_UnitsCategory_Alkalinity,                         // Alkalinity based unit
    Hydro_UnitsCategory_Concentration,                      // Concentration based unit
    Hydro_UnitsCategory_Distance,                           // Distance/position based unit
    Hydro_UnitsCategory_LiqDilution,                        // Liquid dilution based unit
    Hydro_UnitsCategory_LiqFlowRate,                        // Liquid flow rate based unit
    Hydro_UnitsCategory_LiqVolume,                          // Liquid Volume based unit
    Hydro_UnitsCategory_Temperature,                        // Temperature based unit
    Hydro_UnitsCategory_Percentile,                         // Percentile based unit
    Hydro_UnitsCategory_Power,                              // Power based unit
    Hydro_UnitsCategory_Weight,                             // Weight based unit

    Hydro_UnitsCategory_Count,                              // Placeholder
    Hydro_UnitsCategory_Undefined = -1                      // Placeholder
};

// Units Type
// Unit of measurement type. Specifies the unit type associated with a measured value.
// Note: Rate units may only be in per minute, use PER_X_TO_PER_Y defines to convert.
enum Hydro_UnitsType : signed char {
    Hydro_UnitsType_Raw_1,                                  // Normalized raw value mode [0,1=aRef]
    Hydro_UnitsType_Percentile_100,                         // Percentile mode [0,100]
    Hydro_UnitsType_Alkalinity_pH_14,                       // pH value alkalinity mode [0,14]
    Hydro_UnitsType_Concentration_EC_5,                     // Siemens electrical conductivity concentration mode [0,5] (aka mS/cm)
    Hydro_UnitsType_Concentration_PPM_500,                  // Parts-per-million 500 concentration mode [0,2500] (NaCl-based, common for US)
    Hydro_UnitsType_Concentration_PPM_640,                  // Parts-per-million 640 concentration mode [0,3200] (common for EU)
    Hydro_UnitsType_Concentration_PPM_700,                  // Parts-per-million 700 concentration mode [0,3500] (KCl-based, common for AU)
    Hydro_UnitsType_Distance_Feet,                          // Feet distance mode
    Hydro_UnitsType_Distance_Meters,                        // Meters distance mode
    Hydro_UnitsType_LiqDilution_MilliLiterPerGallon,        // Milli liter per gallon dilution mode
    Hydro_UnitsType_LiqDilution_MilliLiterPerLiter,         // Milli liter per liter dilution mode
    Hydro_UnitsType_LiqFlowRate_GallonsPerMin,              // Gallons per minute liquid flow rate mode
    Hydro_UnitsType_LiqFlowRate_LitersPerMin,               // Liters per minute liquid flow rate mode
    Hydro_UnitsType_LiqVolume_Gallons,                      // Gallons liquid volume mode
    Hydro_UnitsType_LiqVolume_Liters,                       // Liters liquid volume mode
    Hydro_UnitsType_Power_Amperage,                         // Amperage current power mode
    Hydro_UnitsType_Power_Wattage,                          // Wattage power mode
    Hydro_UnitsType_Temperature_Celsius,                    // Celsius temperature mode
    Hydro_UnitsType_Temperature_Fahrenheit,                 // Fahrenheit temperature mode
    Hydro_UnitsType_Temperature_Kelvin,                     // Kelvin temperature mode
    Hydro_UnitsType_Weight_Kilograms,                       // Kilograms weight mode
    Hydro_UnitsType_Weight_Pounds,                          // Pounds weight mode

    Hydro_UnitsType_Count,                                  // Placeholder
    Hydro_UnitsType_Concentration_TDS = Hydro_UnitsType_Concentration_EC_5, // Standard TDS concentration mode alias
    Hydro_UnitsType_Concentration_PPM = Hydro_UnitsType_Concentration_PPM_500, // Standard PPM concentration mode alias
    Hydro_UnitsType_Power_JoulesPerSecond = Hydro_UnitsType_Power_Wattage, // Joules per second power mode alias
    Hydro_UnitsType_Undefined = -1                          // Placeholder
};

#define PER_SEC_TO_PER_MIN(t)       ((t) * (SECS_PER_MIN))  // Per seconds to per minutes
#define PER_SEC_TO_PER_HR(t)        ((t) * (SECS_PER_HOUR)) // Per seconds to per hour
#define PER_MIN_TO_PER_SEC(t)       ((t) / (SECS_PER_MIN))  // Per minutes to per seconds
#define PER_MIN_TO_PER_HR(t)        ((t) * (SECS_PER_MIN))  // Per minutes to per hour
#define PER_HR_TO_PER_SEC(t)        ((t) / (SECS_PER_HOUR)) // Per hour to per seconds
#define PER_HR_TO_PER_MIN(t)        ((t) / (SECS_PER_MIN))  // Per hour to per minutes

// Common forward decls
class Hydruino;
class HydroScheduler;
class HydroLogger;
class HydroPublisher;
struct HydroIdentity;
struct HydroData;
struct HydroSubData;
struct HydroCalibrationData;
struct HydroCropsLibData;
struct HydroCustomAdditiveData;
struct HydroObjectData;
struct HydroSystemData;
struct HydroPin;
struct HydroDigitalPin;
struct HydroAnalogPin;
struct HydroActivation;
struct HydroActivationHandle;
struct HydroMeasurement;
struct HydroSingleMeasurement;
class HydroObject;
class HydroSubObject;
class HydroDLinkObject;
class HydroAttachment;
class HydroActuatorAttachment;
class HydroSensorAttachment;
class HydroTriggerAttachment;
class HydroBalancerAttachment;
class HydroTrigger;
class HydroBalancer;
class HydroActuator;
class HydroSensor;
class HydroCrop;
class HydroReservoir;
class HydroFeedReservoir;
class HydroRail;

// System sketches setup enums (for non-zero resolution)
#define SETUP_ENUM_Disabled                                 -1
#define SETUP_ENUM_None                                     -1
#define SETUP_ENUM_Count                                    -1
#define SETUP_ENUM_Undefined                                -1
#define SETUP_ENUM_Primary                                  10
#define SETUP_ENUM_Fallback                                 11
#define SETUP_ENUM_Minimal                                  20
#define SETUP_ENUM_Full                                     21
#define SETUP_ENUM_UART                                     30
#define SETUP_ENUM_I2C                                      31
#define SETUP_ENUM_SPI                                      32
#define SETUP_ENUM_LCD                                      40
#define SETUP_ENUM_Pixel                                    41
#define SETUP_ENUM_ST7735                                   42
#define SETUP_ENUM_TFT                                      43
#define SETUP_ENUM_Encoder                                  50
#define SETUP_ENUM_Buttons                                  51
#define SETUP_ENUM_Joystick                                 52
#define SETUP_ENUM_Matrix                                   53
#define SETUP_ENUM_Serial                                   60
#define SETUP_ENUM_Simhub                                   61
#define SETUP_ENUM_WiFi                                     62
#define SETUP_ENUM_Ethernet                                 63
#define SETUP_ENUM_Hostname                                 70
#define SETUP_ENUM_IPAddress                                71
// Display & controller setup enums (for non-zero resolution)
#define SETUP_ENUM_LCD16x2_EN                               100
#define SETUP_ENUM_LCD16x2_RS                               101
#define SETUP_ENUM_LCD20x4_EN                               102
#define SETUP_ENUM_LCD20x4_RS                               103
#define SETUP_ENUM_SSD1305                                  104
#define SETUP_ENUM_SSD1305_x32Ada                           105
#define SETUP_ENUM_SSD1305_x64Ada                           106
#define SETUP_ENUM_SSD1306                                  107
#define SETUP_ENUM_SH1106                                   108
#define SETUP_ENUM_CustomOLED                               109
#define SETUP_ENUM_SSD1607                                  110
#define SETUP_ENUM_IL3820                                   111
#define SETUP_ENUM_IL3820_V2                                112
#define SETUP_ENUM_ST7789                                   113
#define SETUP_ENUM_ILI9341                                  114
#define SETUP_ENUM_RotaryEncoderOk                          115
#define SETUP_ENUM_RotaryEncoderOkLR                        116
#define SETUP_ENUM_UpDownButtonsOk                          117
#define SETUP_ENUM_UpDownButtonsOkLR                        118
#define SETUP_ENUM_UpDownESP32TouchOk                       119
#define SETUP_ENUM_UpDownESP32TouchOkLR                     120
#define SETUP_ENUM_AnalogJoystickOk                         121
#define SETUP_ENUM_Matrix2x2UpDownButtonsOkL                122
#define SETUP_ENUM_Matrix3x4Keyboard_OptRotEncOk            123
#define SETUP_ENUM_Matrix3x4Keyboard_OptRotEncOkLR          124
#define SETUP_ENUM_Matrix4x4Keyboard_OptRotEncOk            125
#define SETUP_ENUM_Matrix4x4Keyboard_OptRotEncOkLR          126
#define SETUP_ENUM_ResistiveTouch                           127
#define SETUP_ENUM_TouchScreen                              128
#define SETUP_ENUM_TFTTouch                                 129
#define SETUP_ENUM_RemoteControl                            130
// Checks setup defines for equality, first param SETUP_XXX is substituted (possibly to 0), second param literal should be defined (for non-zero substitution)
#define IS_SETUP_AS(X,Y)                JOIN(SETUP_ENUM,X) == SETUP_ENUM_##Y
// Checks setup defines for inequality, first param SETUP_XXX is substituted (possibly to 0), second param literal should be defined (for non-zero substitution)
#define NOT_SETUP_AS(X,Y)               JOIN(SETUP_ENUM,X) != SETUP_ENUM_##Y

#endif // /ifndef HydroDefines_H
