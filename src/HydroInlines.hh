/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Common Inlines
*/
#ifndef HydroInlines_HPP
#define HydroInlines_HPP

struct I2CDeviceSetup;
struct SPIDeviceSetup;
struct UARTDeviceSetup;
struct DeviceSetup;
struct BitResolution;
struct Location;
struct Twilight;

#include "Hydruino.h"
// Only have defines included at this point, complex inline imps at top of Hydruino.hpp

// Returns if pin is valid
inline bool isValidPin(pintype_t pin) { return pin != (pintype_t)-1; }
// Returns if channel is valid
inline bool isValidChannel(uint8_t channel) { return channel != (uint8_t)-1; }
// Returns if measurement row is valid
inline bool isValidRow(uint8_t row) { return row != (uint8_t)-1; }
// Returns if taskId is valid
inline bool isValidTask(unsigned int taskId) { return taskId != 0xffffU; } // purposeful, not using library define incase not included
// Returns if time millis is valid
inline bool isValidTime(millis_t time) { return time != millis_none; }
// Returns if position index is valid
inline bool isValidIndex(hposi_t index) { return index != hposi_none; }
// Returns if id key is valid
inline bool isValidKey(hkey_t key) { return key != hkey_none; }
// Returns if id type is valid
inline bool isValidType(hid_t type) { return type != hid_none; }
// Returns if frame is valid
inline bool isValidFrame(hframe_t frame) { return frame != hframe_none; }

// Returns if two single-precision floating point values are equal with respect to defined error epsilon.
inline bool isFPEqual(float lhs, float rhs) { return fabsf(rhs - lhs) <= FLT_EPSILON; }
// Returns if two double-precision floating point values are equal with respect to defined error epsilon.
inline bool isFPEqual(double lhs, double rhs) { return fabs(rhs - lhs) <= DBL_EPSILON; }

// Returns the first unit in parameter list that isn't undefined, allowing defaulting chains to be nicely defined.
inline Hydro_UnitsType definedUnitsElse(Hydro_UnitsType units1, Hydro_UnitsType units2) {
    return units1 != Hydro_UnitsType_Undefined ? units1 : units2;
}

// Returns the first unit in parameter list that isn't undefined, allowing defaulting chains to be nicely defined.
inline Hydro_UnitsType definedUnitsElse(Hydro_UnitsType units1, Hydro_UnitsType units2, Hydro_UnitsType units3) {
    return units1 != Hydro_UnitsType_Undefined ? units1 : (units2 != Hydro_UnitsType_Undefined ? units2 : units3);
}

// Rounds floating point value to the number of decimal places.
inline float roundToDecimalPlaces(float value, int decimalPlaces) {
    if (decimalPlaces >= 0) {
        float shiftScaler = powf(10.0f, decimalPlaces);
        return roundf(value * shiftScaler) / shiftScaler;
    }
    return value;
}

// I2C Device Setup
// A quick and easy structure for storing I2C device connection settings.
struct I2CDeviceSetup {
    TwoWire *wire;                      // I2C wire instance
    uint32_t speed;                     // I2C max data speed (Hz)
    uint8_t address;                    // I2C device address

    inline I2CDeviceSetup(TwoWire *i2cWire = HYDRO_USE_WIRE, uint32_t i2cSpeed = 100000U, uint8_t i2cAddress = 0b000) : wire(i2cWire), speed(i2cSpeed), address(i2cAddress) { ; }
    inline I2CDeviceSetup(uint32_t i2cSpeed, uint8_t i2cAddress = 0b000, TwoWire *i2cWire = HYDRO_USE_WIRE) : wire(i2cWire), speed(i2cSpeed), address(i2cAddress) { ; }
    inline I2CDeviceSetup(uint8_t i2cAddress, TwoWire *i2cWire = HYDRO_USE_WIRE, uint32_t i2cSpeed = 100000U) : wire(i2cWire), speed(i2cSpeed), address(i2cAddress) { ; }
};

// SPI Device Setup
// A quick and easy structure for storing SPI device connection settings.
struct SPIDeviceSetup {
    SPIClass *spi;                      // SPI class instance
    uint32_t speed;                     // SPI max data speed (Hz)
    pintype_t cs;                       // SPI cable select pin (active-low)

    inline SPIDeviceSetup(SPIClass *spiClass = HYDRO_USE_SPI, uint32_t spiSpeed = F_SPD, pintype_t spiCS = -1) : spi(spiClass), speed(spiSpeed), cs(spiCS) { ; }
    inline SPIDeviceSetup(uint32_t spiSpeed, pintype_t spiCS = -1, SPIClass *spiClass = HYDRO_USE_SPI) : spi(spiClass), speed(spiSpeed), cs(spiCS) { ; }
    inline SPIDeviceSetup(pintype_t spiCS, SPIClass *spiClass = HYDRO_USE_SPI, uint32_t spiSpeed = F_SPD) : spi(spiClass), speed(spiSpeed), cs(spiCS) { ; }
};

// UART Device Setup
// A quick and easy structure for storing serial device connection settings.
struct UARTDeviceSetup {
    SerialClass *serial;                // UART class instance
    uint32_t baud;                      // UART baud rate (bps)

    inline UARTDeviceSetup(SerialClass *serialClass = HYDRO_USE_SERIAL1, uint32_t serialBaud = 9600U) : serial(serialClass), baud(serialBaud) { ; }
    inline UARTDeviceSetup(uint32_t serialBaud, SerialClass *serialClass = HYDRO_USE_SERIAL1) : serial(serialClass), baud(serialBaud) { ; }
};

// Combined Device Setup
// A union of the various device setup structures, to assist with user device settings.
struct DeviceSetup {
    enum : signed char { None, I2CSetup, SPISetup, UARTSetup } cfgType; // Config type
    union {
        I2CDeviceSetup i2c;             // I2C config
        SPIDeviceSetup spi;             // SPI config
        UARTDeviceSetup uart;           // UART config
    } cfgAs;                            // Config data

    inline DeviceSetup() : cfgType(None), cfgAs{.i2c=I2CDeviceSetup(nullptr)} { ; }
    inline DeviceSetup(I2CDeviceSetup i2cSetup) : cfgType(I2CSetup), cfgAs{.i2c=i2cSetup} { ; }
    inline DeviceSetup(SPIDeviceSetup spiSetup) : cfgType(SPISetup), cfgAs{.spi=spiSetup} { ; }
    inline DeviceSetup(UARTDeviceSetup uartSetup) : cfgType(UARTSetup), cfgAs{.uart=uartSetup} { ; }
};

// Analog Bit Resolution
// Used to calculate analog pin range boundary values and convert between integer and
// normalized floating-point formats. The #-of-bits of accuracy will correspond to an
// e.g. lower analogRead() of 0 and an upper analogRead() of 2 ^ #-of-bits, aka maxVal.
// Note: Off-by-one? No, b/c for e.g. 12-bit analogRead(): 0 => no-sig/bin-low,
//       1 => min-sig/PWM-wf, 4095 => max-sig/PWM-wf, 4096=full-sig/bin-high
struct BitResolution {
    const uint8_t bits;                                     // Bit resolution (#-of-bits)
    const int_least32_t maxVal;                             // Maximum value (2 ^ #-of-bits)

    // Bit resolution from # of bits
    inline BitResolution(uint8_t numBits = 8) : bits(numBits), maxVal(1 << numBits) { ; }

    // Transforms value from raw/integer [0,2^#bits] into normalized fp intensity [0.0,1.0].
    inline float transform(int value) const { return constrain(value / (float)maxVal, 0.0f, 1.0f); }

    // Inverse transforms value from normalized fp intensity [0.0,1.0] back into raw/integer [0,2^#bits].
    inline int inverseTransform(float value) const { return constrain((int)((float)maxVal * value), 0, maxVal); }
};

// Device Location Data
// Used in calculating twilight times, UTC offsets, and sun's positioning.
struct Location {
    double latitude;                    // Latitude (degrees)
    double longitude;                   // Longitude (minutes)
    double altitude;                    // Altitude (MSL)

    inline Location() : latitude(DBL_UNDEF), longitude(DBL_UNDEF), altitude(DBL_UNDEF) { ; }
    inline Location(double latitudeIn, double longitudeIn, double altitudeIn = DBL_UNDEF) : latitude(latitudeIn), longitude(longitudeIn), altitude(altitudeIn) { ; }

    inline bool hasPosition() const { return latitude != DBL_UNDEF && longitude != DBL_UNDEF; }
    inline bool hasAltitude() const { return altitude != DBL_UNDEF; }

    // Determines sun altitude for accurate sunrise/sunset calculations. Note: Costly method due to sqrt().
    inline double resolveSunAlt(double defaultSunAlt = SUNRISESET_STD_ALTITUDE) const {
        return hasAltitude() ? SUNRISESET_STD_ALTITUDE - 0.0353 * sqrt(altitude) : defaultSunAlt; // msl-to-sunAlt eq from SolarCalculator example code
    }
};

// Twilight Timing Data
// Used in calculating sunrise/sunset hours and checking if times are in the daytime or nighttime.
struct Twilight {
    double sunrise;                     // Hour of sunrise (+fractional)
    double sunset;                      // Hour of sunset (+fractional)
    bool isUTC;                         // Sunrise/sunset hours stored in UTC format flag

    inline Twilight() : sunrise(HYDRO_NIGHT_FINISH_HR), sunset(HYDRO_NIGHT_START_HR), isUTC(false) { ; }
    inline Twilight(double sunriseIn, double sunsetIn, bool isUTCIn = true) : sunrise(sunriseIn), sunset(sunsetIn), isUTC(isUTCIn) { ; }

    // Determines if passed unix/UTC time is in daytime hours.
    inline bool isDaytime(time_t unixTime = unixNow()) const;
    // Determins if passed local DateTime (offset by system TZ) is in daytime hours.
    inline bool isDaytime(DateTime localTime) const;
    // Determines if passed unix/UTC time is in nighttime hours.
    inline bool isNighttime(time_t unixTime = unixNow()) const { return !isDaytime(unixTime); }
    // Determins if passed local DateTime (offset by system TZ) is in nighttime hours.
    inline bool isNighttime(DateTime localTime) const { return !isDaytime(localTime); }

    // Converts fractional sunrise/sunset hours to unix/UTC time or local DateTime (offset by system TZ).
    static inline time_t hourToUnixTime(double hour, bool isUTC);
    static inline DateTime hourToLocalTime(double hour, bool isUTC);

    // Converts sunrise/sunset hours to unix/UTC time or local DateTime (offset by system TZ).
    inline time_t getSunriseUnixTime() const { return hourToUnixTime(sunrise, isUTC); }
    inline time_t getSunsetUnixTime() const { return hourToUnixTime(sunset, isUTC); }
    inline DateTime getSunriseLocalTime() const { return hourToLocalTime(sunrise, isUTC); }
    inline DateTime getSunsetLocalTime() const { return hourToLocalTime(sunset, isUTC); }
};

#endif // /ifndef HydroInlines_HPP
