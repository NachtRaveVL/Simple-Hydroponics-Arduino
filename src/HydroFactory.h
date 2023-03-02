/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Factory
*/

#ifndef HydroFactory_H
#define HydroFactory_H

class HydroFactory;

#include "Hydruino.h"

// Object Factory
// Contains many methods that automate the creation of various system objects.
// Objects created this way are properly registered with the controller and
// have various cursory checks performed that can alert on an improper setup.
class HydroFactory {
public:

    // Convenience builders for common actuators (shared, nullptr return -> failure).

    // Some actuators, especially variable based, are intended to have user calibration
    // data set to configure input/output ranges. Actuators without user calibration data
    // will assume activation values as normalized raw driving intensities [0,1]/[-1,1].

    // Adds a new relay-based grow light to the system using the given parameters.
    // Grow lights are essential to almost all plants and are used to mimic natural sun rhythms.
    SharedPtr<HydroRelayActuator> addGrowLightsRelay(pintype_t outputPin,                   // Digital output pin this actuator sits on
                                                     uint8_t muxChannel = -1);              // Multiplexer channel number (if multiplexed), else -1

    // Adds a new relay-based water pump to the system using the given parameters.
    // Water pumps are used to feed crops and move liquids around from one reservoir to another.
    SharedPtr<HydroRelayPumpActuator> addWaterPumpRelay(pintype_t outputPin,                // Digital output pin this actuator sits on
                                                        uint8_t muxChannel = -1);           // Multiplexer channel number (if multiplexed), else -1

// TODO: #18 in Hydruino.
//     // Adds a new analog PWM-based water pump to the system using the given parameters.
//     // Analog water pumps allow a graduated adaptive speed control to manage liquid movement.
//     SharedPtr<HydroVariablePumpActuator> addAnalogFanExhaust(pintype_t outputPin,           // PWM output pin this actuator sits on
//                                                              uint8_t outputBitRes = DAC_RESOLUTION, // PWM output bit resolution to use
// #ifdef ESP32
//                                                              uint8_t pwmChannel = 1,        // PWM output channel (0 reserved for buzzer)
// #endif
// #ifdef ESP_PLATFORM
//                                                              float pwmFrequency = 1000,     // PWM output frequency
// #endif
//                                                              uint8_t muxChannel = -1);      // Multiplexer channel number (if multiplexed), else -1

    // Adds a new relay-based water heater to the system using the given parameters.
    // Water heaters can keep feed water heated during colder months and save off root damage.
    SharedPtr<HydroRelayActuator> addWaterHeaterRelay(pintype_t outputPin,                  // Digital output pin this actuator sits on
                                                      uint8_t muxChannel = -1);             // Multiplexer channel number (if multiplexed), else -1

    // Adds a new relay-based water sprayer to the system using the given parameters.
    // Water sprayers can turn on before the lights turn on to provide crops with damp morning soil.
    SharedPtr<HydroRelayActuator> addWaterSprayerRelay(pintype_t outputPin,                 // Digital output pin this actuator sits on
                                                       uint8_t muxChannel = -1);            // Multiplexer channel number (if multiplexed), else -1

    // Adds a new relay-based water aerator to the system using the given parameters.
    // Water aerators can help plants grow while also discouraging pathogens from taking root.
    SharedPtr<HydroRelayActuator> addWaterAeratorRelay(pintype_t outputPin,                 // Digital output pin this actuator sits on
                                                       uint8_t muxChannel = -1);            // Multiplexer channel number (if multiplexed), else -1

    // Adds a new relay-based fan exhaust to the system using the given parameters.
    // Fan exhausts can move air around to modify nearby CO2 levels that plants use to breathe.
    SharedPtr<HydroRelayActuator> addFanExhaustRelay(pintype_t outputPin,                   // Digital output pin this actuator sits on
                                                     uint8_t muxChannel = -1);              // Multiplexer channel number (if multiplexed), else -1

    // Adds a new analog PWM-based fan exhaust to the system using the given parameters.
    // PWM fan exhausts allow a graduated adaptive speed control to manage CO2 levels.
    SharedPtr<HydroVariableActuator> addAnalogFanExhaust(pintype_t outputPin,               // PWM output pin this actuator sits on
                                                         uint8_t outputBitRes = DAC_RESOLUTION, // PWM output bit resolution to use
#ifdef ESP32
                                                         uint8_t pwmChannel = 1,            // PWM output channel (0 reserved for buzzer)
#endif
#ifdef ESP_PLATFORM
                                                         float pwmFrequency = 1000,         // PWM output frequency
#endif
                                                         uint8_t muxChannel = -1);          // Multiplexer channel number (if multiplexed), else -1

    // Adds a new peristaltic dosing pump relay to the system using the given parameters.
    // Peristaltic pumps allow proper dosing of nutrients and other additives.
    SharedPtr<HydroRelayPumpActuator> addPeristalticPumpRelay(pintype_t outputPin,          // Digital output pin this actuator sits on
                                                              uint8_t muxChannel = -1);     // Multiplexer channel number (if multiplexed), else -1

    // Convenience builders for common sensors (shared, nullptr return -> failure).

    // Many sensors, especially analog based, are intended to have user calibration data
    // set to configure input/output ranges. Sensors without user calibration data will
    // return measurements in raw reading intensity units [0,1].

    // Adds a new binary level indicator to the system using the given parameters.
    // Level indicators can be used to control filled/empty status of a liquid reservoir.
    SharedPtr<HydroBinarySensor> addLevelIndicator(pintype_t inputPin,                      // Digital input pin this sensor sits on (can make interruptable)
                                                   bool isActiveLow = true,                 // If indication is active when a LOW signal is read (aka active-low)
                                                   uint8_t muxChannel = -1);                // Multiplexer channel number (if multiplexed), else -1

    // Adds a new analog PH meter to the system using the given parameters.
    // pH meters are vital in ensuring the proper alkalinity level is used in feed water.
    SharedPtr<HydroAnalogSensor> addAnalogPhMeter(pintype_t inputPin,                       // Analog input pin this sensor sits on
                                                  uint8_t inputBitRes = ADC_RESOLUTION,     // ADC input bit resolution to use
                                                  uint8_t muxChannel = -1);                 // Multiplexer channel number (if multiplexed), else -1

    // Adds a new analog TDS electrode to the system using the given parameters.
    // TDS electrodes are vital in ensuring the proper nutrition levels are used in feed water.
    SharedPtr<HydroAnalogSensor> addAnalogTDSElectrode(pintype_t inputPin,                  // Analog input pin this sensor sits on
                                                       uint8_t inputBitRes = ADC_RESOLUTION, // ADC input bit resolution to use
                                                       uint8_t muxChannel = -1);            // Multiplexer channel number (if multiplexed), else -1

    // Adds a new analog temperature sensor to the system using the given parameters.
    // Temperature sensors can be used to ensure proper temperature conditions are being met.
    SharedPtr<HydroAnalogSensor> addAnalogTemperatureSensor(pintype_t inputPin,             // Analog input pin this sensor sits on
                                                            uint8_t inputBitRes = ADC_RESOLUTION, // ADC input bit resolution to use
                                                            uint8_t muxChannel = -1);       // Multiplexer channel number (if multiplexed), else -1

    // Adds a new analog CO2 sensor to the system using the given parameters.
    // CO2 sensors can be used to ensure proper CO2 levels are being met.
    // Creates user calibration data calibrated to ppm scaling if ppmScale isn't standard 500/640/700.
    SharedPtr<HydroAnalogSensor> addAnalogCO2Sensor(pintype_t inputPin,                     // Analog input pin this sensor sits on
                                                    int ppmScale = 500,                     // PPM measurement scaling (500, 640, 700 - default: 500)
                                                    uint8_t inputBitRes = ADC_RESOLUTION,   // ADC input bit resolution to use
                                                    uint8_t muxChannel = -1);               // Multiplexer channel number (if multiplexed), else -1

    // Adds a new analog moisture sensor to the system using the given parameters.
    // Soil moisture sensors can be used to drive feedings for crops.
    SharedPtr<HydroAnalogSensor> addAnalogMoistureSensor(pintype_t inputPin,                // Analog input pin this sensor sits on
                                                         uint8_t inputBitRes = ADC_RESOLUTION, // ADC input bit resolution to use
                                                         uint8_t muxChannel = -1);          // Multiplexer channel number (if multiplexed), else -1

    // Adds a new analog PWM-based pump flow sensor to the system using the given parameters.
    // Pump flow sensors can allow for more precise liquid volume pumping calculations.
    SharedPtr<HydroAnalogSensor> addAnalogPumpFlowSensor(pintype_t inputPin,                // Analog input pin this sensor sits on
                                                         uint8_t inputBitRes = ADC_RESOLUTION, // ADC input bit resolution to use
                                                         uint8_t muxChannel = -1);          // Multiplexer channel number (if multiplexed), else -1

    // Adds a new analog water height meter to the system using the given parameters.
    // Water height meters can be used to determine the volume of a container.
    SharedPtr<HydroAnalogSensor> addAnalogWaterHeightMeter(pintype_t inputPin,              // Analog input pin this sensor sits on
                                                           uint8_t inputBitRes = ADC_RESOLUTION, // ADC input bit resolution to use
                                                           uint8_t muxChannel = -1);        // Multiplexer channel number (if multiplexed), else -1

    // Adds a new downward-facing analog ultrasonic distance sensor to the system using the given parameters.
    // Downward-facing ultrasonic distance sensors can be used to determine the volume of a container.
    // (Pro-tip: These widely available inexpensive sensors don't sit in the water and thus won't corrode as fast.)
    SharedPtr<HydroAnalogSensor> addUltrasonicDistanceSensor(pintype_t inputPin,            // Analog input pin this sensor sits on
                                                             uint8_t inputBitRes = ADC_RESOLUTION, // ADC input bit resolution to use
                                                             uint8_t muxChannel = -1);      // Multiplexer channel number (if multiplexed), else -1

    // Adds a new analog power usage meter to the system using the given parameters.
    // Power usage meters can be used to determine and manage the energy demands of a power rail.
    SharedPtr<HydroAnalogSensor> addPowerLevelMeter(pintype_t inputPin,                     // Analog input pin this sensor sits on
                                                    bool isWattageBased = true,             // If power meter measures wattage (true) or amperage (false)
                                                    uint8_t inputBitRes = ADC_RESOLUTION,   // ADC input bit resolution to use
                                                    uint8_t muxChannel = -1);               // Multiplexer channel number (if multiplexed), else -1

    // Adds a new digital DHT* OneWire temperature & humidity sensor to the system using the given parameters.
    // Uses the DHT library. A very common digital sensor, included in most Arduino starter kits.
    SharedPtr<HydroDHTTempHumiditySensor> addDHTTempHumiditySensor(pintype_t inputPin,      // OneWire digital input pin this sensor sits on
                                                                   Hydro_DHTType dhtType = Hydro_DHTType_DHT12); // DHT sensor type

    // Adds a new digital DS18* OneWire submersible temperature sensor to the system using the given parameters.
    // Uses the DallasTemperature library. A specialized submersible sensor meant for long-term usage.
    SharedPtr<HydroDSTemperatureSensor> addDSTemperatureSensor(pintype_t inputPin,          // OneWire digital input pin this sensor sits on
                                                               uint8_t inputBitRes = 9,     // Sensor ADC input bit resolution to use
                                                               pintype_t pullupPin = -1);   // Strong pullup pin (if used, else -1)

    // Convenience builders for common crops (shared, nullptr return -> failure).

    // Adds a new simple timer-fed crop to the system using the given parameters, by past or future date of sowing.
    // Timer fed crops use an hourly time-on/time-off schedule for driving their feeding times.
    SharedPtr<HydroTimedCrop> addTimerFedCrop(Hydro_CropType cropType,                      // Crop type (kind of plant)
                                              Hydro_SubstrateType substrateType,            // Substrate type (soil composition)
                                              DateTime sowTime,                             // Sow date (may be a future date to stagger enable feed reservoirs)
                                              uint8_t minsOn = 15,                          // Feeding signal on-time interval, in minutes
                                              uint8_t minsOff = 45);                        // Feeding signal off-time interval, in minutes

    // Adds a new simple timer-fed crop to the system using the given parameters, by last or expected harvest date.
    // Certain crops, such as perennials that grow back every year, may be easier to define from their harvest date instead.
    SharedPtr<HydroTimedCrop> addTimerFedCropByHarvest(Hydro_CropType cropType,             // Crop type (kind of plant)
                                                       Hydro_SubstrateType substrateType,   // Substrate type (soil composition)
                                                       DateTime harvestTime,                // Harvest time (year ignored),
                                                       uint8_t minsOn = 15,                 // Feeding signal on-time interval, in minutes
                                                       uint8_t minsOff = 45);               // Feeding signal off-time interval, in minutes

    // Adds a new adaptive trigger-fed crop to the system using the given parameters, by past or future date of sowing.
    // Adaptive crops use soil based sensing, such as soil moisture sensors, to drive their feeding times.
    SharedPtr<HydroAdaptiveCrop> addAdaptiveFedCrop(Hydro_CropType cropType,                // Crop type (kind of plant)
                                                    Hydro_SubstrateType substrateType,      // Substrate type (soil composition)
                                                    DateTime sowTime);                      // Sow date (start of growth cycle)

    // Adds a new adaptive trigger-fed crop to the system using the given parameters, by last or expected harvest date (year ignored/replaced with current).
    // Certain crops, such as perennials that grow back every year, may be easier to define from their harvest date instead.
    SharedPtr<HydroAdaptiveCrop> addAdaptiveFedCropByHarvest(Hydro_CropType cropType,       // Crop type (kind of plant)
                                                             Hydro_SubstrateType substrateType, // Substrate type (soil composition)
                                                             DateTime harvestTime);         // Harvest date (last or expected)

    // Convenience builders for common reservoirs (shared, nullptr return -> failure).

    // Adds a new simple fluid reservoir to the system using the given parameters.
    // Fluid reservoirs are basically just buckets of some liquid solution with a known or measurable volume.
    SharedPtr<HydroFluidReservoir> addFluidReservoir(Hydro_ReservoirType reservoirType,     // Reservoir type (liquid contents)
                                                     float maxVolume,                       // Maximum volume
                                                     bool beginFilled = false);             // If reservoir should begin filled or empty

    // Adds a new feed reservoir to the system using the given parameters.
    // Feed reservoirs, aka channels, are the reservoirs used to feed crops and provide a central point for managing feeding.
    SharedPtr<HydroFeedReservoir> addFeedWaterReservoir(float maxVolume,                    // Maximum volume
                                                        bool beginFilled = false,           // If feed reservoir should begin filled or empty
                                                        DateTime lastChangeTime = localNow(), // Last water change date
                                                        DateTime lastPruningTime = localNow()); // Last pruning date

    // Adds a drainage pipe to the system using the given parameters.
    // Drainage pipes are never-filled infinite reservoirs that can always be pumped/drained into.
    SharedPtr<HydroInfiniteReservoir> addDrainagePipe();

    // Adds a fresh water main to the system using the given parameters.
    // Fresh water mains are always-filled infinite reservoirs that can always be pumped/sourced from.
    SharedPtr<HydroInfiniteReservoir> addFreshWaterMain();

    // Convenience builders for common power rails (shared, nullptr return -> failure).

    // Adds a new simple power rail to the system using the given parameters.
    // Simple power rail uses a max active at once counting strategy to manage energy consumption.
    SharedPtr<HydroSimpleRail> addSimplePowerRail(Hydro_RailType railType,                  // Rail type
                                                  int maxActiveAtOnce = 2);                 // Maximum active devices

    // Adds a new regulated power rail to the system using the given parameters.
    // Regulated power rails can use a power meter to measure energy consumption to limit overdraw.
    SharedPtr<HydroRegulatedRail> addRegulatedPowerRail(Hydro_RailType railType,            // Rail type
                                                        float maxPower);                    // Maximum allowed power
};

#endif // /ifndef HydroFactory_H
