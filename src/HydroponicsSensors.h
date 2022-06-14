/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensors
*/

#ifndef HydroponicsSensors_H
#define HydroponicsSensors_H

class HydroponicsSensor;
class HydroponicsBinarySensor;
class HydroponicsAnalogSensor;
class HydroponicsDHTOneWireSensor;
class HydroponicsDSOneWireSensor;
class HydroponicsTMPOneWireSensor;

#include "Hydroponics.h"

extern Hydroponics_UnitsType getDefaultSensorUnits(Hydroponics_SensorType sensorType,
                                                   Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);

// Hydroponics Sensor Base
// This is the base class for all sensors, which defines how the sensor is identified and
// what it's paired to. Other than that, consider it a pure virtual base class.
class HydroponicsSensor : public HydroponicsObject {
public:
    HydroponicsSensor(Hydroponics_SensorType sensorType,
                      Hydroponics_PositionIndex sensorIndex,
                      byte inputPin = -1);
    virtual ~HydroponicsSensor();

    virtual HydroponicsSensorMeasurement *takeMeasurement(bool force = true) = 0;
    virtual HydroponicsSensorMeasurement *getLastMeasurement() const = 0;
    virtual time_t getLastMeasurementTime() const = 0;

    byte getInputPin() const;
    Hydroponics_SensorType getSensorType() const;
    Hydroponics_PositionIndex getSensorIndex() const;

protected:
    byte _inputPin;                                         // Input pin
};


// Simple Binary Sensor
// This class can both read from and assign interrupt routines to a digital input signal,
// allowing it to act as an on/off switch of sorts. Examples include water level indicators.
class HydroponicsBinarySensor : public HydroponicsSensor {
public:
    HydroponicsBinarySensor(Hydroponics_SensorType sensorType,
                            Hydroponics_PositionIndex sensorIndex,
                            byte inputPin,
                            bool activeLow = true);
    virtual ~HydroponicsBinarySensor();

    // TODO reg as isr
    //bool tryRegisterAsISR();

    virtual HydroponicsSensorMeasurement *takeMeasurement(bool force = true);
    virtual HydroponicsSensorMeasurement *getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    bool getActiveLow() const;

protected:
    bool _activeLow;                                        // Active when low flag
    // TODO isr reg state data
    HydroponicsBinarySensorMeasurement _lastMeasurement;    // Latest successful measurement
};


// Simple Analog Sensor
// The ever reliant master of analogRead()'ing, this class manages polling an analog input
// signal and converting it into the proper figures for use. Examples include everything
// from TDS EC meters to PWM based flow sensors.
class HydroponicsAnalogSensor : public HydroponicsSensor {
public:
    HydroponicsAnalogSensor(Hydroponics_SensorType sensorType,
                            Hydroponics_PositionIndex sensorIndex,
                            byte inputPin,
                            byte inputBitRes = 8);
    virtual ~HydroponicsAnalogSensor();

    virtual HydroponicsSensorMeasurement *takeMeasurement(bool force = true);
    virtual HydroponicsSensorMeasurement *getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    HydroponicsCalibrationData *loadInputCalibration(bool create = true);
    void saveInputCalibration(HydroponicsCalibrationData *newData);

    HydroponicsBitResolution getInputResolution() const;

protected:
    HydroponicsBitResolution _inputResolution;              // Analog input resolution
    HydroponicsAnalogSensorMeasurement _lastMeasurement;    // Latest successful measurement
    HydroponicsCalibrationData *_inputCalib;                // Input calibration data if loaded (owned)
};


// Digital DHT* Temperature & Humidity Sensor
// This class is for working with DHT* OneWire-based air temperature and humidity sensors.
class HydroponicsDHTOneWireSensor : public HydroponicsSensor {
public:
    HydroponicsDHTOneWireSensor(Hydroponics_PositionIndex sensorIndex,
                                byte inputPin,
                                uint8_t dhtType = DHT12,
                                bool computeHeatIndex = true);
    virtual ~HydroponicsDHTOneWireSensor();

    virtual HydroponicsSensorMeasurement *takeMeasurement(bool force = true);
    virtual HydroponicsSensorMeasurement *getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    void setComputeHeatIndex(bool computeHeatIndex);
    bool getComputeHeatIndex();

protected:
    DHT *_dht;                                                  // DHT sensor instance (owned)
    bool _computeHeatIndex;                                     // Flag to compute heat index
    HydroponicsDHTOneWireSensorMeasurement _lastMeasurement;    // Latest successful measurement
};


// Digital DS18* Submersible Temperature Sensor
// This class is for working with DS18* OneWire-based submersible temperature sensors.
class HydroponicsDSOneWireSensor : public HydroponicsSensor {
public:
    HydroponicsDSOneWireSensor(Hydroponics_PositionIndex sensorIndex,
                               byte inputPin,
                               byte inputBitRes = 12);
    virtual ~HydroponicsDSOneWireSensor();

    virtual HydroponicsSensorMeasurement *takeMeasurement(bool force = true);
    virtual HydroponicsSensorMeasurement *getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    OneWire &getOneWire() const;

protected:
    OneWire *_oneWire;                                      // OneWire comm instance (owned)
    DallasTemperature *_dt;                                 // DallasTemperature instance (owned)
    HydroponicsAnalogSensorMeasurement _lastMeasurement;    // Latest successful measurement
};


// Digital TMP* Soil Moisture Sensor
// This class is for working with TMP* OneWire-based soil moisture sensors.
class HydroponicsTMPOneWireSensor : public HydroponicsSensor {
public:
    HydroponicsTMPOneWireSensor(Hydroponics_PositionIndex sensorIndex,
                                byte inputPin,
                                byte inputBitRes = 8);
    virtual ~HydroponicsTMPOneWireSensor();

    virtual HydroponicsSensorMeasurement *takeMeasurement(bool force = true);
    virtual HydroponicsSensorMeasurement *getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    OneWire &getOneWire() const;

protected:
    OneWire *_oneWire;                                      // OneWire comm instance (owned)
    // TODO: Find class for working with this one
    HydroponicsAnalogSensorMeasurement _lastMeasurement;    // Latest successful measurement
};

#endif // /ifndef HydroponicsSensors_H
