/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Common Inlines
*/
#ifndef HydroInlines_HPP
#define HydroInlines_HPP

#include "Hydruino.h"

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

    inline I2CDeviceSetup(TwoWire *i2cWire = HYDRO_USE_WIRE, uint32_t i2cSpeed = 100000U, uint8_t i2cAddress = B000) : wire(i2cWire), speed(i2cSpeed), address(i2cAddress) { ; }
    inline I2CDeviceSetup(TwoWire *i2cWire, uint8_t i2cAddress, uint32_t i2cSpeed = 100000U) : wire(i2cWire), speed(i2cSpeed), address(i2cAddress) { ; }
    inline I2CDeviceSetup(uint32_t i2cSpeed, TwoWire *i2cWire, uint8_t i2cAddress = B000) : wire(i2cWire), speed(i2cSpeed), address(i2cAddress) { ; }
    inline I2CDeviceSetup(uint32_t i2cSpeed, uint8_t i2cAddress = B000, TwoWire *i2cWire = HYDRO_USE_WIRE) : wire(i2cWire), speed(i2cSpeed), address(i2cAddress) { ; }
    inline I2CDeviceSetup(uint8_t i2cAddress, TwoWire *i2cWire, uint32_t i2cSpeed = 100000U) : wire(i2cWire), speed(i2cSpeed), address(i2cAddress) { ; }
    inline I2CDeviceSetup(uint8_t i2cAddress, uint32_t i2cSpeed = 100000U, TwoWire *i2cWire = HYDRO_USE_WIRE) : wire(i2cWire), speed(i2cSpeed), address(i2cAddress) { ; }
};

// SPI Device Setup
// A quick and easy structure for storing SPI device connection settings.
struct SPIDeviceSetup {
    SPIClass *spi;                      // SPI class instance
    uint32_t speed;                     // SPI max data speed (Hz)
    pintype_t cs;                       // SPI cable select pin (active-low)

    inline SPIDeviceSetup(SPIClass *spiClass = HYDRO_USE_SPI, uint32_t spiSpeed = F_SPD, pintype_t spiCS = -1) : spi(spiClass), speed(spiSpeed), cs(spiCS) { ; }
    inline SPIDeviceSetup(SPIClass *spiClass, pintype_t spiCS, uint32_t spiSpeed = F_SPD) : spi(spiClass), speed(spiSpeed), cs(spiCS) { ; }
    inline SPIDeviceSetup(uint32_t spiSpeed, SPIClass *spiClass, pintype_t spiCS = -1) : spi(spiClass), speed(spiSpeed), cs(spiCS) { ; }
    inline SPIDeviceSetup(uint32_t spiSpeed, pintype_t spiCS = -1, SPIClass *spiClass = HYDRO_USE_SPI) : spi(spiClass), speed(spiSpeed), cs(spiCS) { ; }
    inline SPIDeviceSetup(pintype_t spiCS, SPIClass *spiClass, uint32_t spiSpeed = F_SPD) : spi(spiClass), speed(spiSpeed), cs(spiCS) { ; }
    inline SPIDeviceSetup(pintype_t spiCS, uint32_t spiSpeed = F_SPD, SPIClass *spiClass = HYDRO_USE_SPI) : spi(spiClass), speed(spiSpeed), cs(spiCS) { ; }
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

// Location Data
struct Location {
    double latitude;                    // Latitude (degrees)
    double longitude;                   // Longitude (minutes)
    double altitude;                    // Altitude (MSL)

    inline Location() : latitude(DBL_UNDEF), longitude(DBL_UNDEF), altitude(DBL_UNDEF) { ; }
    inline Location(double latitudeIn, double longitudeIn, double altitudeIn = DBL_UNDEF) : latitude(latitudeIn), longitude(longitudeIn), altitude(altitudeIn) { ; }

    inline bool hasPosition() const { return !isFPEqual(latitude, DBL_UNDEF) && !isFPEqual(longitude, DBL_UNDEF); }
    inline bool hasAltitude() const { return !isFPEqual(altitude, DBL_UNDEF); }

    // Determines sun altitude for accurate sunrise/sunset calculations. Note: Costly method due to sqrt().
    inline double resolveSunAlt(double defaultSunAlt = SUNRISESET_STD_ALTITUDE) const {
        return hasAltitude() ? SUNRISESET_STD_ALTITUDE - 0.0353 * sqrt(altitude) : defaultSunAlt;
    }
};

// Twilight Data
struct Twilight {
    double sunrise;                     // Hour of sunrise (+fractional)
    double sunset;                      // Hour of sunset (+fractional)
    bool isUTC;                         // UTC flag

    inline Twilight() : sunrise(HYDRO_NIGHT_FINISH_HR), sunset(HYDRO_NIGHT_START_HR), isUTC(false) { ; }
    inline Twilight(double sunriseIn, double sunsetIn, bool isUTCIn = true) : sunrise(sunriseIn), sunset(sunsetIn), isUTC(isUTCIn) { ; }

    // Correctly determines if passed UTC time is in daytime hours or not, with proper respect to system TZ settings/fractional hours.
    inline bool isDaytime(time_t time = unixNow()) const;
};

#endif // /ifndef HydroInlines_HPP
