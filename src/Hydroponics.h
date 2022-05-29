/*  Arduino Controller for Simple Hydroponics.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    This permission notice shall be included in all copies or
    substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

    Hydroponics-Arduino - Version 0.1
*/

#ifndef Hydroponics_H
#define Hydroponics_H

// Library Setup

// NOTE: While editing the main header file isn't ideal, it is often the easiest given
// the Arduino IDE's limited custom build flag support. Editing this header file directly
// will affect all projects compiled on your system using these library files.

// Uncomment or -D this define to disable usage of the Scheduler library on SAM/SAMD architecures.
//#define HYDRO_DISABLE_SCHEDULER                 // https://github.com/arduino-libraries/Scheduler
// Uncomment or -D this define to disable usage of LCD library. Maybe? TODO
// Uncomment or -D this define to disable usage of LCD OLED library. Maybe? TODO

// Uncomment or -D this define to enable debug output.
#define HYDRO_ENABLE_DEBUG_OUTPUT


// Hookup Callouts
// -PLEASE READ-
// TODO.

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <DallasTemperature.h>          // DS18* submersible water temp probe
#include <DHT.h>                        // DHT* OneWire air temp/humidity probe
#include <EasyBuzzer\src\EasyBuzzer.h>  // Asyncronous piezo buzzer library
#include <I2C_EEPROM\I2C_eeprom.h>      // i2c EEPROM interface library
#include <Keypad\src\Keypad.h>          // 4-way directional matrix keypad
#include <LiquidCrystal_I2C.h>          // i2c LCD library

#ifndef __STM32F1__
#include <OneWire.h>                    // OneWire for DHT* probes             
#else
#include <OneWireSTM.h>
#endif

#include <RTCLib\src\RTClib.h>          // i2c RTC library

#if !defined(HYDRO_DISABLE_SCHEDULER) && (defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD))
#include "Scheduler.h"
#define HYDRO_USE_SCHEDULER
#endif

#ifndef HYDRO_ENABLE_SOFTWARE_I2C
#include <Wire.h>
#if BUFFER_LENGTH
#define HYDRO_I2C_BUFFER_LENGTH   BUFFER_LENGTH
#elif I2C_BUFFER_LENGTH
#define HYDRO_I2C_BUFFER_LENGTH   I2C_BUFFER_LENGTH
#else
#warning "i2c buffer length not defined - using default value of 32a, which may not be correct for your microcontroller. Check Wire.h (or similar) for your hardware and manually define BUFFER_LENGTH or I2C_BUFFER_LENGTH to remove this warning."
#define HYDRO_I2C_BUFFER_LENGTH   32
#endif // /if BUFFER_LENGTH
#else
#include <avr/io.h>
#define HYDRO_USE_SOFTWARE_I2C
#endif // /ifndef HYDRO_ENABLE_SOFTWARE_I2C

#include <Time\TimeLib.h>
#include <TimeAlarms\TimeAlarms.h>

#include "HydroponicsDefines.h"
#include "HydroponicsInlines.hpp"

#include "HydroponicsActuators.h"
#include "HydroponicsCrops.h"
#include "HydroponicsSensors.h"

class Hydroponics {
public:
    // Library constructor. Typically called during class instantiation, before setup().
    // TODO
    Hydroponics(byte piezoBuzzerPin = 0,
                byte i2cAddressEEPROM = B000000,
                byte i2cAddressRTC = B000000,
                byte i2cAddressLCD = B000000,
                byte controlInputPin1 = 0,
                TwoWire& i2cWire = Wire, uint32_t i2cSpeed = 400000);
    Hydroponics(TwoWire& i2cWire, uint32_t i2cSpeed = 400000,
                byte piezoBuzzerPin = 0,
                byte i2cAddressEEPROM = B000000,
                byte i2cAddressRTC = B000000,
                byte i2cAddressLCD = B000000,
                byte controlInputPin1 = 0);
    ~Hydroponics();

    // Initializes module. Typically called in setup().
    // See individual enums for more info.
    void init(Hydroponics_WaterReservoirMode waterResMode = Hydroponics_WaterReservoirMode_Recycling,
              Hydroponics_TemperatureMode tempMode = Hydroponics_TemperatureMode_Celsius,
              Hydroponics_LCDOutputMode lcdOutMode = Hydroponics_LCDOutputMode_Disabled,
              Hydroponics_ControlInputMode ctrlInMode = Hydroponics_ControlInputMode_Disabled);

    // Update method. Typically called in loop().
    void update();


    // Actuator, sensor, and crop registration.

    // Adds/removes acuator to/from system (ownership transfer - system will delete object upon class deconstruction unless unregistered).
    void registerActuator(HydroponicsActuator *actuator);
    void unregisterActuator(HydroponicsActuator *actuator);

    // Convenience builders for actuators (unowned, NULL return = failure)
    HydroponicsActuator *addGrowLightsRelay(byte outputPin);
    HydroponicsActuator *addWaterPumpRelay(byte outputPin, Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater);
    HydroponicsActuator *addWaterHeaterRelay(byte outputPin);
    HydroponicsActuator *addWaterAeratorRelay(byte outputPin);
    HydroponicsActuator *addFanCirculationRelay(byte outputPin);
    HydroponicsActuator *addFanExhaustRelay(byte outputPin);
    HydroponicsActuator *addPhUpPeristalticPumpRelay(byte outputPin);
    HydroponicsActuator *addPhDownPeristalticPumpRelay(byte outputPin);
    HydroponicsActuator *addNutrientPremixPeristalticPumpRelay(byte outputPin);
    HydroponicsActuator *addFreshWaterPeristalticPumpRelay(byte outputPin);

    // Adds/removes sensor to/from system (ownership transfer)
    void registerSensor(HydroponicsSensor *sensor);
    void unregisterSensor(HydroponicsSensor *sensor);

    // Convenience builders for common sensors (unowned, NULL return = failure)
    HydroponicsSensor *addAirDHTTempHumiditySensor(OneWire &oneWire);
    HydroponicsSensor *addWaterPhMeter(byte inputPin);
    HydroponicsSensor *addWaterTDSElectrode(byte inputPin);
    HydroponicsSensor *addWaterDSTempSensor(byte inputPin);
    HydroponicsSensor *addWaterPumpFlowSensor(byte inputPin, Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater);
    HydroponicsSensor *addLowWaterLevelIndicator(byte inputPin, Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater);
    HydroponicsSensor *addHighWaterLevelIndicator(byte inputPin, Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater);
    HydroponicsSensor *addLowWaterHeightMeter(byte inputPin, Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater);
    HydroponicsSensor *addHighWaterHeightMeter(byte inputPin, Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater);
    HydroponicsSensor *addLowWaterUltrasonicSensor(byte inputPin, Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater);
    HydroponicsSensor *addHighWaterUltrasonicSensor(byte inputPin, Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater);

    // Adds/removes crops to/from system (ownership transfer)
    void registerCrop(HydroponicsCrop *crop);
    void unregisterCrop(HydroponicsCrop *crop);

    // Convenience builders for crops (unowned, NULL return = failure)
    HydroponicsCrop *addCropFromSowDate(const Hydroponics_CropType cropType, time_t sowDate, int positionIndex = -1);
    HydroponicsCrop *addCropFromLastHarvest(const Hydroponics_CropType cropType, time_t lastHarvestDate, int positionIndex = -1);


    // Accessors.

    Hydroponics_WaterReservoirMode getWaterReservoirMode() const;   // System water reservoir mode (default: recycling)
    Hydroponics_TemperatureMode getTemperatureMode() const;         // System temperature mode (default: celsius)
    Hydroponics_LCDOutputMode getLCDOutputMode() const;             // System LCD output mode (default: disabled)
    Hydroponics_ControlInputMode getControlInputMode() const;       // System control input mode (default: disabled)

    EasyBuzzer *getPiezoBuzzer() const;                         // Piezo buzzer EasyBuzzer instance, available after init (default: NULL)
    I2C_eeprom *getEEPROM() const;                              // EEPROM instance, available after init() (default: NULL)
    RTC_DS3231 *getRealTimeClock() const;                       // Real time clock instance, available after init() (default: NULL)
    LiquidCrystal_I2C *getLiquidCrystalDisplay() const;         // Liquid crystal display instance, available after init() (default: NULL)
    Keypad *getControlKeypad() const;                           // Control Keypad instance, available after init() (default: NULL)

    int getRelayCount(Hydroponics_RelayRail relayRail = Hydroponics_RelayRail_Undefined) const;           // Current number of relay devices registered with system, for the given rail (undefined-rail = all)
    int getActiveRelayCount(Hydroponics_RelayRail relayRail = Hydroponics_RelayRail_Undefined) const;     // Current number of active relay devices, for the given rail (undefined-rail = all)
    int getMaxActiveRelayCount(Hydroponics_RelayRail relayRail = Hydroponics_RelayRail_Undefined) const;  // Maximum number of relay devices allowed active at a time, for the given rail (default: 2, undefined-rail = all)

    int getActuatorCount() const;                               // Current number of total actuators registered with system
    int getSensorCount() const;                                 // Current number of total sensors registered with system

    const char * getSystemName() const;                         // System display name (default: "Hydroduino", 31 char limit)
    uint8_t getCropPositionsCount() const;                      // Total number of crop positions available in system (default: 16)
    float getReservoirSize(Hydroponics_FluidReservoir fluidReservoir) const;    // Fluid reservoir size, for given reservoir (liters)
    float getPumpFlowRate(Hydroponics_FluidReservoir fluidReservoir) const;     // Fluid pump flow rate, for given reservoir (liters/sec)

    // Mutators.

    void setMaxActiveRelayCount(int maxActiveCount, Hydroponics_RelayRail relayRail);   // Sets maximum number of relay devices allowed active at a time, for the given rail

    void setSystemName(const char * systemName);                // Sets display name of system (31 char limit)
    void setCropPositionsCount(uint8_t cropPositionsCount);     // Sets number of crop positions
    void setReservoirSize(float reservoirSize, Hydroponics_FluidReservoir fluidReservoir);  // Sets reservoir size, for the given reservoir (liters)
    void setPumpFlowRate(float pumpFlowRate, Hydroponics_FluidReservoir fluidReservoir);    // Sets pump flow rate, for the given reservoir (liters/sec)

    typedef void(*UserDelayFunc)(unsigned int);                 // Passes delay timeout (where 0 indicates inside long blocking call / yield attempt suggested)
    // Sets user delay functions to call when a delay has to occur for processing to
    // continue. User functions here can customize what this means - typically it would
    // mean to call into a thread barrier() or yield() mechanism. Default implementation
    // simply calls standard delay() and delayMicroseconds(), unless on SAM/SAMD
    // architectures where Scheduler is available, in which case when timeout > 1ms
    // Scheduler.yield() is called until timeout expires.
    void setUserDelayFuncs(UserDelayFunc delayMillisFunc, UserDelayFunc delayMicrosFunc);


    // Internal use (friend functions).

    bool tryEnableActuator(HydroponicsActuator *actuator);
    bool tryMeasureSensor(HydroponicsSensor *sensor);


protected:
    byte _i2cAddressLCD;                                    // LCD i2c address
    byte _controlInputPin1;                                 // Control input pin 1
    TwoWire* _i2cWire;                                      // Wire class instance (unowned) (default: Wire)
    uint32_t _i2cSpeed;                                     // Module's i2c clock speed (default: 400000)

    Hydroponics_WaterReservoirMode _waterResMode;           // System water reservoir mode
    Hydroponics_TemperatureMode _tempMode;                  // System temperature mode
    Hydroponics_LCDOutputMode _lcdOutMode;                  // System LCD mode
    Hydroponics_ControlInputMode _ctrlInMode;               // System control input mode

    EasyBuzzer *_buzzer;                                    // Piezo buzzer instance (unowned)
    I2C_eeprom *_eeprom;                                    // EEPROM instance (owned)
    RTC_DS3231 *_rtc;                                       // Real time clock instance (owned)
    LiquidCrystal_I2C *_lcd;                                // Liquid crystal display instance (owned)
    Keypad *_keypad;                                        // Control matrix keypad (owned)

    HydroponicsSystemData *_systemData;                     // System data (owned, saved to EEPROM)

    // TODO maybe we use?
    UserDelayFunc _uDelayMillisFunc;                        // User millisecond delay function
    UserDelayFunc _uDelayMicrosFunc;                        // User microsecond delay function

    // TODO maybe we use?
    // static float kelvin100ToCelsius(uint16_t kelvin100);
    // static float kelvin100ToFahrenheit(uint16_t kelvin100);
    // static float kelvin100ToKelvin(uint16_t kelvin100);
    // static uint16_t celsiusToKelvin100(float celsius);
    // static uint16_t fahrenheitToKelvin100(float fahrenheit);
    // static uint16_t kelvinToKelvin100(float kelvin);
};

#endif // /ifndef Hydroponics_H
