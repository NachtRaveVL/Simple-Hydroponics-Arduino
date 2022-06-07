/*  Arduino Controller for Simple Hydroponics.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensors
*/

#ifndef HydroponicsSensors_H
#define HydroponicsSensors_H

struct HydroponicsSensorMeasurement;
struct HydroponicsSensorAnalogMeasurement;
struct HydroponicsSensorDHTMeasurement;
struct HydroponicsSensorBinaryMeasurement;
struct HydroponicsSensorBinaryAnalogMeasurement;

class HydroponicsSensor;
class HydroponicsAnalogSensor;
class HydroponicsDHTSensor;
class HydroponicsDSSensor;
class HydroponicsBinarySensor;
class HydroponicsBinaryAnalogSensor;

#include "Hydroponics.h"


struct HydroponicsSensorMeasurement
{
    time_t timestamp;
};

struct HydroponicsSensorAnalogMeasurement : public HydroponicsSensorMeasurement
{
    float value;
    // TODO: units?
};

struct HydroponicsSensorDHTMeasurement : public HydroponicsSensorMeasurement
{
    float temperature;
    float humidity;
    float heatIndex;
};

struct HydroponicsSensorBinaryMeasurement : public HydroponicsSensorMeasurement
{
    bool state;
};

struct HydroponicsSensorBinaryAnalogMeasurement : public HydroponicsSensorMeasurement
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

    HydroponicsSensorAnalogMeasurement takeMeasurement();
    HydroponicsSensorAnalogMeasurement getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    byte getInputPin() const;
    int getAnalogMaxAmount() const;
    int getAnalogBitResolution() const;

protected:
    byte _inputPin;
    int _analogMaxAmount;
    byte _analogBitRes;
    HydroponicsSensorAnalogMeasurement _lastMeasurement;
};

class HydroponicsDHTSensor : public HydroponicsSensor {
public:
    HydroponicsDHTSensor(byte inputPin,
                         Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                         uint8_t dhtType = DHT12);
    virtual ~HydroponicsDHTSensor();

    HydroponicsSensorDHTMeasurement takeMeasurement(bool force = true);
    HydroponicsSensorDHTMeasurement getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    void setComputeHeatIndex(bool computeHeatIndex);
    bool getComputeHeatIndex();

protected:
    DHT *_dht;
    HydroponicsSensorDHTMeasurement _lastMeasurement;
    bool _computeHeatIndex;
};


class HydroponicsDSSensor : public HydroponicsSensor {
public:
    HydroponicsDSSensor(byte inputPin,
                        Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                        byte readBitResolution = 9);
    virtual ~HydroponicsDSSensor();

    HydroponicsSensorAnalogMeasurement takeMeasurement();
    HydroponicsSensorAnalogMeasurement getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    OneWire &getOneWire() const;

protected:
    OneWire *_oneWire;
    DallasTemperature *_dt;
    HydroponicsSensorAnalogMeasurement _lastMeasurement;
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

    HydroponicsSensorBinaryMeasurement takeMeasurement();
    HydroponicsSensorBinaryMeasurement getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    byte getInputPin() const;
    bool getActiveLow() const;

protected:
    byte _inputPin;
    bool _activeLow;
    HydroponicsSensorBinaryMeasurement _lastMeasurement;
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

    HydroponicsSensorBinaryAnalogMeasurement takeMeasurement();
    HydroponicsSensorBinaryAnalogMeasurement getLastMeasurement() const;
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
    HydroponicsSensorBinaryAnalogMeasurement _lastMeasurement;
};

#endif // /ifndef HydroponicsSensors_H
