/*  Arduino Controller for Simple Hydroponics.
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


struct HydroponicsSensorMeasurement : public HydroponicsLoggableDataInterface
{
    time_t timestamp;
};

struct HydroponicsAnalogSensorMeasurement : public HydroponicsSensorMeasurement
{
    float value;
    // TODO: units?
};

struct HydroponicsDHTSensorMeasurement : public HydroponicsSensorMeasurement
{
    float temperature;
    float humidity;
    float heatIndex;
};

struct HydroponicsBinarySensorMeasurement : public HydroponicsSensorMeasurement
{
    bool state;
};

struct HydroponicsBinaryAnalogSensorMeasurement : public HydroponicsSensorMeasurement
{
    float value;
    bool state;
};


class HydroponicsSensor {
public:
    HydroponicsSensor(Hydroponics_SensorType sensorType,
                      Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_Undefined);
    virtual ~HydroponicsSensor();

    Hydroponics_SensorType getSensorType() const;
    Hydroponics_FluidReservoir getFluidReservoir() const;
    virtual time_t getLastMeasurementTime() const = 0;

    virtual void update();

protected:
    Hydroponics_SensorType _sensorType;
    Hydroponics_FluidReservoir _fluidReservoir;
};


class HydroponicsAnalogSensor : public HydroponicsSensor {
public:
    HydroponicsAnalogSensor(byte inputPin,
                            Hydroponics_SensorType sensorType,
                            Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                            byte readBitResolution = 8);
    virtual ~HydroponicsAnalogSensor();

    HydroponicsAnalogSensorMeasurement takeMeasurement();
    HydroponicsAnalogSensorMeasurement getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    byte getInputPin() const;
    int getAnalogMaxAmount() const;
    int getAnalogBitResolution() const;

protected:
    byte _inputPin;
    int _analogMaxAmount;
    byte _analogBitRes;
    HydroponicsAnalogSensorMeasurement _lastMeasurement;
};

class HydroponicsDHTSensor : public HydroponicsSensor {
public:
    HydroponicsDHTSensor(byte inputPin,
                         Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                         uint8_t dhtType = DHT12);
    virtual ~HydroponicsDHTSensor();

    HydroponicsDHTSensorMeasurement takeMeasurement(bool force = true);
    HydroponicsDHTSensorMeasurement getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    void setComputeHeatIndex(bool computeHeatIndex);
    bool getComputeHeatIndex();

protected:
    DHT *_dht;
    HydroponicsDHTSensorMeasurement _lastMeasurement;
    bool _computeHeatIndex;
};


class HydroponicsDSSensor : public HydroponicsSensor {
public:
    HydroponicsDSSensor(byte inputPin,
                        Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                        byte readBitResolution = 9);
    virtual ~HydroponicsDSSensor();

    HydroponicsAnalogSensorMeasurement takeMeasurement();
    HydroponicsAnalogSensorMeasurement getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    OneWire &getOneWire() const;

protected:
    OneWire *_oneWire;
    DallasTemperature *_dt;
    HydroponicsAnalogSensorMeasurement _lastMeasurement;
};


class HydroponicsBinarySensor : public HydroponicsSensor {
public:
    HydroponicsBinarySensor(byte inputPin,
                            Hydroponics_SensorType sensorType,
                            Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                            bool activeLow = true);
    virtual ~HydroponicsBinarySensor();

    // TODO reg as isr maybe?
    //void registerAsISR();

    // TODO event listener maybe?
    //void addEventListener(int paramsTODO)

    HydroponicsBinarySensorMeasurement takeMeasurement();
    HydroponicsBinarySensorMeasurement getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    byte getInputPin() const;
    bool getActiveLow() const;

protected:
    byte _inputPin;
    bool _activeLow;
    HydroponicsBinarySensorMeasurement _lastMeasurement;
};


class HydroponicsBinaryAnalogSensor : public HydroponicsSensor {
public:
    HydroponicsBinaryAnalogSensor(byte inputPin, float tolerance, bool activeBelow,
                                  Hydroponics_SensorType sensorType,
                                  Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                                  byte readBitResolution = 8);
    virtual ~HydroponicsBinaryAnalogSensor();

    // TODO event listener maybe?
    //void addEventListener(int paramsTODO)

    HydroponicsBinaryAnalogSensorMeasurement takeMeasurement();
    HydroponicsBinaryAnalogSensorMeasurement getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    byte getInputPin() const;
    float getTolerance() const;
    bool getActiveBelow() const;
    int getAnalogMaxAmount() const;
    int getAnalogBitResolution() const;

protected:
    byte _inputPin;
    float _tolerance;
    bool _activeBelow;
    int _analogMaxAmount;
    byte _analogBitRes;
    HydroponicsBinaryAnalogSensorMeasurement _lastMeasurement;
};

#endif // /ifndef HydroponicsSensors_H
