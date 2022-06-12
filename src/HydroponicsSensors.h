/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensors
*/

#ifndef HydroponicsSensors_H
#define HydroponicsSensors_H

struct HydroponicsSensorMeasurement;
struct HydroponicsAnalogSensorMeasurement;
struct HydroponicsDHTSensorMeasurement;
struct HydroponicsBinarySensorMeasurement;
struct HydroponicsBinaryAnalogSensorMeasurement;

class HydroponicsSensor;
class HydroponicsAnalogSensor;
class HydroponicsDHTSensor;
class HydroponicsDSSensor;
class HydroponicsBinarySensor;
class HydroponicsBinaryAnalogSensor;

#include "Hydroponics.h"

// TODO: Change ALL measurement stuff below to using async methods.

// TODO
class HydroponicsSensor {
public:
    HydroponicsSensor(Hydroponics_SensorType sensorType,
                      Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_Undefined,
                      Hydroponics_MeasurementMode measurementMode = Hydroponics_MeasurementMode_Default);
    virtual ~HydroponicsSensor();

    virtual HydroponicsSensorMeasurement *takeMeasurement();
    virtual HydroponicsSensorMeasurement *getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    virtual void update();

    String getKey() const;
    static String getKeyFor(Hydroponics_SensorType sensorType,
                            Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_Undefined);
    Hydroponics_SensorType getSensorType() const;
    Hydroponics_FluidReservoir getFluidReservoir() const;
    Hydroponics_UnitsType getMeasurementUnits() const;

protected:
    String _key;                                            // Identifier
    Hydroponics_SensorType _sensorType;                     // Sensor type enumeration
    Hydroponics_FluidReservoir _fluidReservoir;             // Fluid reservoir sensor belongs to
    Hydroponics_UnitsType _measurementUnits;                // Measurement units for sensor
};


// TODO
class HydroponicsAnalogSensor : public HydroponicsSensor {
public:
    HydroponicsAnalogSensor(byte inputPin,
                            Hydroponics_SensorType sensorType,
                            Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                            Hydroponics_MeasurementMode measurementMode = Hydroponics_MeasurementMode_Default,
                            byte readBitResolution = 8);
    virtual ~HydroponicsAnalogSensor();

    virtual HydroponicsAnalogSensorMeasurement *takeMeasurement();
    virtual HydroponicsAnalogSensorMeasurement *getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    byte getInputPin() const;
    int getAnalogMaxAmount() const;
    int getAnalogBitResolution() const;

protected:
    byte _inputPin;                                         // Sensor input pin (should be A0 - AX)
    int _analogMaxAmount;                                   // Maximum value that reads support
    byte _analogBitRes;                                     // Bit resolution of analog reads
    HydroponicsAnalogSensorMeasurement _lastMeasurement;    // Latest successful measurement
};


// TODO
class HydroponicsDHTSensor : public HydroponicsSensor {
public:
    HydroponicsDHTSensor(byte inputPin,
                         Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                         Hydroponics_MeasurementMode measurementMode = Hydroponics_MeasurementMode_Default,
                         uint8_t dhtType = DHT12);
    virtual ~HydroponicsDHTSensor();

    virtual HydroponicsDHTSensorMeasurement *takeMeasurement(bool force = true);
    virtual HydroponicsDHTSensorMeasurement *getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    void setComputeHeatIndex(bool computeHeatIndex);
    bool getComputeHeatIndex();

protected:
    DHT *_dht;                                              // DHT sensor instance (owned)
    HydroponicsDHTSensorMeasurement _lastMeasurement;       // Latest successful measurement
    bool _computeHeatIndex;                                 // Flag to compute heat index
};


// TODO
class HydroponicsDSSensor : public HydroponicsSensor {
public:
    HydroponicsDSSensor(byte inputPin,
                        Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                        Hydroponics_MeasurementMode measurementMode = Hydroponics_MeasurementMode_Default,
                        byte readBitResolution = 9);
    virtual ~HydroponicsDSSensor();

    virtual HydroponicsAnalogSensorMeasurement *takeMeasurement();
    virtual HydroponicsAnalogSensorMeasurement *getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    OneWire &getOneWire() const;

protected:
    OneWire *_oneWire;                                      // OneWire comm instance (owned)
    DallasTemperature *_dt;                                 // DallasTemperature instance (owned)
    HydroponicsAnalogSensorMeasurement _lastMeasurement;    // Latest successful measurement
};


// TODO
class HydroponicsBinarySensor : public HydroponicsSensor {
public:
    HydroponicsBinarySensor(byte inputPin,
                            Hydroponics_SensorType sensorType,
                            Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                            Hydroponics_MeasurementMode measurementMode = Hydroponics_MeasurementMode_Default,
                            bool activeLow = true);
    virtual ~HydroponicsBinarySensor();

    // TODO reg as isr maybe?
    //bool tryRegisterAsISR();

    virtual HydroponicsBinarySensorMeasurement *takeMeasurement();
    virtual HydroponicsBinarySensorMeasurement *getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    byte getInputPin() const;
    bool getActiveLow() const;

protected:
    byte _inputPin;                                         // Sensor input pin (best to use interrupt pin)
    bool _activeLow;                                        // Active when low flag
    HydroponicsBinarySensorMeasurement _lastMeasurement;    // Latest successful measurement
};


class HydroponicsBinaryAnalogSensor : public HydroponicsSensor {
public:
    HydroponicsBinaryAnalogSensor(byte inputPin, float tolerance, bool activeBelow,
                                  Hydroponics_SensorType sensorType,
                                  Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                                  Hydroponics_MeasurementMode measurementMode = Hydroponics_MeasurementMode_Default,
                                  byte readBitResolution = 8);
    virtual ~HydroponicsBinaryAnalogSensor();

    virtual HydroponicsBinaryAnalogSensorMeasurement *takeMeasurement();
    virtual HydroponicsBinaryAnalogSensorMeasurement *getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    byte getInputPin() const;
    float getTolerance() const;
    bool getActiveBelow() const;
    int getAnalogMaxAmount() const;
    int getAnalogBitResolution() const;

protected:
    byte _inputPin;                                         // Sensor input pin (should be A0 - AX)
    float _tolerance;                                       // Trigger tolerance
    bool _activeBelow;                                      // Active when below flag
    int _analogMaxAmount;                                   // Maximum value that reads support
    byte _analogBitRes;                                     // Bit resolution of analog reads
    HydroponicsBinaryAnalogSensorMeasurement _lastMeasurement; // Latest successful measurement
};

#endif // /ifndef HydroponicsSensors_H
