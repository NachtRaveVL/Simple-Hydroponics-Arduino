/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Factory
*/

#ifndef HydroponicsFactory_H
#define HydroponicsFactory_H

class HydroponicsFactory;

#include "Hydroponics.h"

class HydroponicsFactory {
public:
    // Object Factory.

    // Convenience builders for common actuators (shared, nullptr return -> failure).

    // Adds a new grow light relay to the system using the given parameters.
    // Grow lights are essential to almost all plants and are used to mimic natural sun rhythms.
    SharedPtr<HydroponicsRelayActuator> addGrowLightsRelay(pintype_t outputPin);                    // Digital output pin this actuator sits on
    // Adds a new water pump relay to the system using the given parameters.
    // Water pumps are used to feed crops and move liquids around from one reservoir to another.
    SharedPtr<HydroponicsPumpRelayActuator> addWaterPumpRelay(pintype_t outputPin);                 // Digital output pin this actuator sits on
    // Adds a new water heater relay to the system using the given parameters.
    // Water heaters can keep feed water heated during colder months and save off root damage.
    SharedPtr<HydroponicsRelayActuator> addWaterHeaterRelay(pintype_t outputPin);                   // Digital output pin this actuator sits on
    // Adds a new water sprayer relay to the system using the given parameters.
    // Water sprayers can turn on before the lights turn on to provide crops with damp morning soil.
    SharedPtr<HydroponicsRelayActuator> addWaterSprayerRelay(pintype_t outputPin);                  // Digital output pin this actuator sits on
    // Adds a new water aerator relay to the system using the given parameters.
    // Water aerators can help plants grow while also discouraging pathogens from taking root.
    SharedPtr<HydroponicsRelayActuator> addWaterAeratorRelay(pintype_t outputPin);                  // Digital output pin this actuator sits on
    // Adds a new fan exhaust relay to the system using the given parameters.
    // Fan exhausts can move air around to modify nearby CO2 levels that plants use to breathe.
    SharedPtr<HydroponicsRelayActuator> addFanExhaustRelay(pintype_t outputPin);                    // Digital output pin this actuator sits on
    // Adds a new analog PWM-based fan exhaust to the system using the given parameters.
    // PWM fan exhausts allow a graduated adaptive speed control to manage CO2 levels.
    SharedPtr<HydroponicsPWMActuator> addAnalogPWMFanExhaust(pintype_t outputPin,                   // PWM output pin this actuator sits on
                                                             uint8_t outputBitRes = 10);            // PWM bit resolution to use
    // Adds a new peristaltic dosing pump relay to the system using the given parameters.
    // Peristaltic pumps allow proper dosing of nutrients and other additives.
    SharedPtr<HydroponicsPumpRelayActuator> addPeristalticPumpRelay(pintype_t outputPin);           // Digital output pin this actuator sits on

    // Convenience builders for common sensors (shared, nullptr return -> failure).

    // Adds a new binary level indicator to the system using the given parameters.
    // Level indicators can be used to control filled/empty status of a liquid reservoir.
    SharedPtr<HydroponicsBinarySensor> addLevelIndicator(pintype_t inputPin);                       // Digital input pin this sensor sits on (can make interruptable)

    // Adds a new analog PH meter to the system using the given parameters.
    // pH meters are vital in ensuring the proper alkalinity level is used in feed water.
    SharedPtr<HydroponicsAnalogSensor> addAnalogPhMeter(pintype_t inputPin,                         // Analog input pin this sensor sits on
                                                         uint8_t inputBitRes = 10);                 // ADC bit resolution to use
    // Adds a new analog TDS electrode to the system using the given parameters.
    // TDS electrodes are vital in ensuring the proper nutrition levels are used in feed water.
    SharedPtr<HydroponicsAnalogSensor> addAnalogTDSElectrode(pintype_t inputPin,                    // Analog input pin this sensor sits on
                                                              int ppmScale = 500,                   // PPM measurement scaling (default: 500, aka TDS/PPM500)
                                                              uint8_t inputBitRes = 10);            // ADC bit resolution to use
    // Adds a new analog temperature sensor to the system using the given parameters.
    // Temperature sensors can be used to ensure proper temperature conditions.
    SharedPtr<HydroponicsAnalogSensor> addAnalogTemperatureSensor(pintype_t inputPin,               // Analog input pin this sensor sits on
                                                                   uint8_t inputBitRes = 10);       // ADC bit resolution to use
    // Adds a new analog CO2 sensor to the system using the given parameters.
    // CO2 sensors can be used to ensure proper CO2 levels.
    SharedPtr<HydroponicsAnalogSensor> addAnalogCO2Sensor(pintype_t inputPin,                       // Analog input pin this sensor sits on
                                                           uint8_t inputBitRes = 10);               // ADC bit resolution to use
    // Adds a new analog moisture sensor to the system using the given parameters.
    // Soil moisture sensors can be used to drive feedings for crops.
    SharedPtr<HydroponicsAnalogSensor> addAnalogMoistureSensor(pintype_t inputPin,                  // Analog input pin this sensor sits on
                                                                uint8_t inputBitRes = 10);          // ADC bit resolution to use
    // Adds a new analog PWM-based pump flow sensor to the system using the given parameters.
    // Pump flow sensors can allow for more precise liquid volume pumping calculations.
    SharedPtr<HydroponicsAnalogSensor> addAnalogPWMPumpFlowSensor(pintype_t inputPin,               // Analog input pin this sensor sits on
                                                                    uint8_t inputBitRes = 10);      // ADC bit resolution to use
    // Adds a new analog water height meter to the system using the given parameters.
    // Water height meters can be used to determine the volume of a container.
    SharedPtr<HydroponicsAnalogSensor> addAnalogWaterHeightMeter(pintype_t inputPin,                // Analog input pin this sensor sits on
                                                                  uint8_t inputBitRes = 10);        // ADC bit resolution to use
    // Adds a new downward-facing analog ultrasonic distance sensor to the system using the given parameters.
    // Downward-facing ultrasonic distance sensors can be used to determine the volume of a container.
    // (Pro-tip: These widely available inexpensive sensors don't sit in the water and thus won't corrode.)
    SharedPtr<HydroponicsAnalogSensor> addUltrasonicDistanceSensor(pintype_t inputPin,              // Analog input pin this sensor sits on
                                                                    uint8_t inputBitRes = 10);      // ADC bit resolution to use
    // Adds a new analog power usage meter to the system using the given parameters.
    // Power usage meters can be used to determine and manage the energy demands of a power rail.
    SharedPtr<HydroponicsAnalogSensor> addPowerUsageMeter(pintype_t inputPin,                       // Analog input pin this sensor sits on
                                                           bool isWattageBased,                     // If power meter measures wattage (true) or amperage (false)
                                                           uint8_t inputBitRes = 10);               // ADC bit resolution to use

    // Adds a new digital DHT* OneWire temperature & humidity sensor to the system using the given parameters.
    // Uses the DHT library. A very common digital sensor, included in most Arduino starter kits.
    SharedPtr<HydroponicsDHTTempHumiditySensor> addDHTTempHumiditySensor(pintype_t inputPin,        // OneWire digital input pin this sensor sits on
                                                                          uint8_t dhtType = DHT12); // Kind of DHT sensor (see DHT* defines)
    // Adds a new digital DS18* OneWire submersible temperature sensor to the system using the given parameters.
    // Uses the DallasTemperature library. A specialized submersible sensor meant for long-term usage.
    SharedPtr<HydroponicsDSTemperatureSensor> addDSTemperatureSensor(pintype_t inputPin,            // OneWire digital input pin this sensor sits on
                                                                      uint8_t inputBitRes = 9,      // Sensor ADC bit resolution to use
                                                                      pintype_t pullupPin = -1);    // Strong pullup pin (if used, else -1)

    // Convenience builders for common crops (shared, nullptr return -> failure).

    // Adds a new simple timer-fed crop to the system using the given parameters.
    // Timer fed crops use a simple on/off timer for driving their feeding signal.
    SharedPtr<HydroponicsTimedCrop> addTimerFedCrop(Hydroponics_CropType cropType,                  // Crop type
                                                     Hydroponics_SubstrateType substrateType,       // Substrate type
                                                     DateTime sowDate,                              // Sow date
                                                     uint8_t minsOn = 15,                           // Feeding signal on-time interval, in minutes
                                                     uint8_t minsOff = 45);                         // Feeding signal off-time interval, in minutes
    // Adds a new simple timer-fed crop to the system using the given parameters (perennials only).
    // Perennials that grow back are easier to define from their last end-of-harvest date instead of when they were planted.
    SharedPtr<HydroponicsTimedCrop> addTimerFedPerennialCrop(Hydroponics_CropType cropType,         // Crop type
                                                              Hydroponics_SubstrateType substrateType, // Substrate type
                                                              DateTime lastHarvestDate,             // Last harvest date
                                                              uint8_t minsOn = 15,                  // Feeding signal on-time interval, in minutes
                                                              uint8_t minsOff = 45);                // Feeding signal off-time interval, in minutes

    // Adds a new adaptive trigger-fed crop to the system using the given parameters.
    // Adaptive crops use soil based sensors, such as moisture sensors, to drive their feeding signal.
    SharedPtr<HydroponicsAdaptiveCrop> addAdaptiveFedCrop(Hydroponics_CropType cropType,            // Crop type
                                                           Hydroponics_SubstrateType substrateType, // Substrate type
                                                           DateTime sowDate);                       // Sow date
    // Adds a new adaptive trigger-fed crop to the system using the given parameters (perennials only).
    // Perennials that grow back are easier to define from their last end-of-harvest date instead of when they were planted.
    SharedPtr<HydroponicsAdaptiveCrop> addAdaptiveFedPerennialCrop(Hydroponics_CropType cropType,   // Crop type
                                                                    Hydroponics_SubstrateType substrateType, // Substrate type
                                                                    DateTime lastHarvestDate);      // Last harvest date

    // Convenience builders for common reservoirs (shared, nullptr return -> failure).

    // Adds a new simple fluid reservoir to the system using the given parameters.
    // Fluid reservoirs are basically just buckets of some liquid solution with a known or measurable volume.
    SharedPtr<HydroponicsFluidReservoir> addFluidReservoir(Hydroponics_ReservoirType reservoirType, // Reservoir type
                                                            float maxVolume,                        // Maximum volume
                                                            bool beginFilled = false);              // If reservoir should begin filled or empty

    // Adds a new feed reservoir to the system using the given parameters.
    // Feed reservoirs, aka channels, are the reservoirs used to feed crops and provide a central point for managing feeding.
    SharedPtr<HydroponicsFeedReservoir> addFeedWaterReservoir(float maxVolume,                      // Maximum volume
                                                               bool beginFilled = false,            // If feed reservoir should begin filled or empty
                                                               DateTime lastChangeDate = DateTime((uint32_t)unixNow()), // Last water change date
                                                               DateTime lastPruningDate = DateTime()); // Last pruning date

    // Adds a drainage pipe to the system using the given parameters.
    // Drainage pipes are never-filled infinite reservoirs that can always be pumped/drained into.
    SharedPtr<HydroponicsInfiniteReservoir> addDrainagePipe();
    // Adds a fresh water main to the system using the given parameters.
    // Fresh water mains are always-filled infinite reservoirs that can always be pumped/sourced from.
    SharedPtr<HydroponicsInfiniteReservoir> addFreshWaterMain();

    // Convenience builders for common power rails (shared, nullptr return -> failure).

    // Adds a new simple power rail to the system using the given parameters.
    // Simple power rail uses a max active at once counting strategy to manage energy consumption.
    SharedPtr<HydroponicsSimpleRail> addSimplePowerRail(Hydroponics_RailType railType,              // Rail type
                                                         int maxActiveAtOnce = 2);                  // Maximum active devices

    // Adds a new regulated power rail to the system using the given parameters.
    // Regulated power rails can use a power meter to measure energy consumption to limit overdraw.
    SharedPtr<HydroponicsRegulatedRail> addRegulatedPowerRail(Hydroponics_RailType railType,        // Rail type
                                                               float maxPower);                     // Maximum allowed power
};

#endif // /ifndef HydroponicsFactory_H
