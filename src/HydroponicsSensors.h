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

// TODO
struct HydroponicsSensorMeasurement : public HydroponicsLoggableDataInterface
{
    time_t timestamp;                       // TODO
};

// TODO
struct HydroponicsAnalogSensorMeasurement : public HydroponicsSensorMeasurement
{
    float value;                            // TODO
    Hydroponics_UnitsType units;            // TODO
};

// TODO
struct HydroponicsDHTSensorMeasurement : public HydroponicsSensorMeasurement
{
    float temperature;                      // TODO
    Hydroponics_UnitsType temperatureUnits; // TODO
    float humidity;                         // TODO
    Hydroponics_UnitsType humidityUnits;    // TODO
    float heatIndex;                        // TODO
    Hydroponics_UnitsType heatIndexUnits;   // TODO
};

// TODO
struct HydroponicsBinarySensorMeasurement : public HydroponicsSensorMeasurement
{
    bool state;                             // TODO
};

// TODO
struct HydroponicsBinaryAnalogSensorMeasurement : public HydroponicsSensorMeasurement
{
    float value;                            // TODO
    Hydroponics_UnitsType units;            // TODO
    bool state;                             // TODO
};


// TODO
class HydroponicsSensor {
public:
    HydroponicsSensor(Hydroponics_SensorType sensorType,
                      Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_Undefined);
    virtual ~HydroponicsSensor();

    virtual HydroponicsSensorMeasurement *takeMeasurement();
    virtual HydroponicsSensorMeasurement *getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    virtual void update();

    Hydroponics_SensorType getSensorType() const;
    Hydroponics_FluidReservoir getFluidReservoir() const;

protected:
    Hydroponics_SensorType _sensorType;                     // TODO
    Hydroponics_FluidReservoir _fluidReservoir;             // TODO
};


// TODO
class HydroponicsAnalogSensor : public HydroponicsSensor {
public:
    HydroponicsAnalogSensor(byte inputPin,
                            Hydroponics_SensorType sensorType,
                            Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                            byte readBitResolution = 8);
    virtual ~HydroponicsAnalogSensor();

    virtual HydroponicsAnalogSensorMeasurement *takeMeasurement();
    virtual HydroponicsAnalogSensorMeasurement *getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    byte getInputPin() const;
    int getAnalogMaxAmount() const;
    int getAnalogBitResolution() const;

    void setMeasurementUnits(Hydroponics_UnitsType units);

protected:
    byte _inputPin;                                         // TODO
    int _analogMaxAmount;                                   // TODO
    byte _analogBitRes;                                     // TODO
    HydroponicsAnalogSensorMeasurement _lastMeasurement;    // TODO
    Hydroponics_UnitsType _measurementUnits;                // TODO
};


// TODO
class HydroponicsDHTSensor : public HydroponicsSensor {
public:
    HydroponicsDHTSensor(byte inputPin,
                         Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                         uint8_t dhtType = DHT12);
    virtual ~HydroponicsDHTSensor();

    virtual HydroponicsDHTSensorMeasurement *takeMeasurement(bool force = true);
    virtual HydroponicsDHTSensorMeasurement *getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    void setComputeHeatIndex(bool computeHeatIndex);
    bool getComputeHeatIndex();

    void setMeasurementUnits(Hydroponics_UnitsType units);

protected:
    DHT *_dht;                                              // TODO
    HydroponicsDHTSensorMeasurement _lastMeasurement;       // TODO
    bool _computeHeatIndex;                                 // TODO
    Hydroponics_UnitsType _measurementUnits;                // TODO
};


// TODO
class HydroponicsDSSensor : public HydroponicsSensor {
public:
    HydroponicsDSSensor(byte inputPin,
                        Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                        byte readBitResolution = 9);
    virtual ~HydroponicsDSSensor();

    virtual HydroponicsAnalogSensorMeasurement *takeMeasurement();
    virtual HydroponicsAnalogSensorMeasurement *getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    OneWire &getOneWire() const;

    void setMeasurementUnits(Hydroponics_UnitsType units);

protected:
    OneWire *_oneWire;                                      // TODO
    DallasTemperature *_dt;                                 // TODO
    HydroponicsAnalogSensorMeasurement _lastMeasurement;    // TODO
    Hydroponics_UnitsType _measurementUnits;                // TODO
};


// TODO
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

    virtual HydroponicsBinarySensorMeasurement *takeMeasurement();
    virtual HydroponicsBinarySensorMeasurement *getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    byte getInputPin() const;
    bool getActiveLow() const;

protected:
    byte _inputPin;                                         // TODO
    bool _activeLow;                                        // TODO
    HydroponicsBinarySensorMeasurement _lastMeasurement;    // TODO
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

    virtual HydroponicsBinaryAnalogSensorMeasurement *takeMeasurement();
    virtual HydroponicsBinaryAnalogSensorMeasurement *getLastMeasurement() const;
    virtual time_t getLastMeasurementTime() const;

    byte getInputPin() const;
    float getTolerance() const;
    bool getActiveBelow() const;
    int getAnalogMaxAmount() const;
    int getAnalogBitResolution() const;

    void setMeasurementUnits(Hydroponics_UnitsType units);

protected:
    byte _inputPin;                                         // TODO
    float _tolerance;                                       // TODO
    bool _activeBelow;                                      // TODO
    int _analogMaxAmount;                                   // TODO
    byte _analogBitRes;                                     // TODO
    HydroponicsBinaryAnalogSensorMeasurement _lastMeasurement; // TODO
    Hydroponics_UnitsType _measurementUnits;                // TODO
};

#endif // /ifndef HydroponicsSensors_H
