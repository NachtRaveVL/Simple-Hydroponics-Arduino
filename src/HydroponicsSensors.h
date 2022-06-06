/*  Arduino Controller for Simple Hydroponics.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensors
*/

#ifndef HydroponicsSensors_H
#define HydroponicsSensors_H

class HydroponicsSensor;
class HydroponicsAnalogSensor;
class HydroponicsDHTSensor;
class HydroponicsDSSensor;
class HydroponicsBinarySensor;
class HydroponicsBinaryAnalogSensor;

#include "Hydroponics.h"

//typedef void(*IntrAdvSensorFinish)(float);             // Passes raw value

class HydroponicsSensor {
public:
    HydroponicsSensor(Hydroponics_SensorType sensorType,
                      Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_Undefined);
    virtual ~HydroponicsSensor();

    Hydroponics_SensorType getSensorType() const;
    Hydroponics_FluidReservoir getFluidReservoir() const;
    time_t getLastMeasurementTime() const;

protected:
    Hydroponics_SensorType _sensorType;
    Hydroponics_FluidReservoir _fluidReservoir;
    time_t _lastMeasureTime;
};

class HydroponicsAnalogSensor : public HydroponicsSensor {
public:
    HydroponicsAnalogSensor(byte inputPin,
                            Hydroponics_SensorType sensorType,
                            Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                            byte readBitResolution = 8);
    virtual ~HydroponicsAnalogSensor();

    float getLastMeasurement() const;
    virtual float takeMeasurement();

    byte getInputPin() const;
    int getAnalogMaxAmount() const;
    int getAnalogBitResolution() const;

protected:
    byte _inputPin;
    int _analogMaxAmount;
    byte _analogBitRes;
    float _lastMeasurement;
};

struct DHTMeasurement {
    float temperature;
    float humidity;
};

class HydroponicsDHTSensor : public HydroponicsSensor {
public:
    HydroponicsDHTSensor(byte inputPin,
                         Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                         uint8_t dhtType = DHT12);
    virtual ~HydroponicsDHTSensor();

    DHTMeasurement getLastMeasurement() const;
    DHTMeasurement takeMeasurement(bool force = true);

protected:
    DHT *_dht;
    DHTMeasurement _lastMeasurement;
};

class HydroponicsDSSensor : public HydroponicsSensor {
public:
    HydroponicsDSSensor(byte inputPin,
                        Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater,
                        byte readBitResolution = 9);
    virtual ~HydroponicsDSSensor();

    float getLastMeasurement() const;
    virtual float takeMeasurement();

    OneWire &getOneWire() const;

protected:
    OneWire *_oneWire;
    DallasTemperature *_dt;
    float _lastMeasurement;
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

    bool pollState();
    bool getLastState() const;

    byte getInputPin() const;
    bool getActiveLow() const;

protected:
    byte _inputPin;
    bool _activeLow;
    bool _lastState;
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

    bool pollState();
    bool getLastState() const;
    float getLastMeasurement() const;

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
    bool _lastState;
    float _lastMeasurement;
};

#endif // /ifndef HydroponicsSensors_H
