/*  Arduino Controller for Simple Hydroponics.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensors
*/

#ifndef HydroponicsSensors_H
#define HydroponicsSensors_H

class HydroponicsSensor;
class HydroponicsAnalogSensor;
class HydroponicsOneWireSensor;
class HydroponicsBinarySensor;
class HydroponicsBinaryAnalogSensor;

#include "Hydroponics.h"

//typedef void(*IntrAdvSensorFinish)(double);             // Passes raw value

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
                            Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater);
    virtual ~HydroponicsAnalogSensor();

    double getLastMeasurement() const;
    virtual double takeNewMeasurement();

protected:
    double _lastMeasurement;
    byte _inputPin;
};

class HydroponicsOneWireSensor : public HydroponicsAnalogSensor {
public:
    HydroponicsOneWireSensor(OneWire &oneWire,
                             Hydroponics_SensorType sensorType,
                             Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater);
    virtual ~HydroponicsOneWireSensor();

    OneWire &getOneWire();

protected:
    OneWire *_oneWire;
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

protected:
    bool _lastState;
    bool _activeLow;
};

class HydroponicsBinaryAnalogSensor : public HydroponicsSensor {
public:
    HydroponicsBinaryAnalogSensor(byte inputPin, float tolerance, bool activeBelow,
                                  Hydroponics_SensorType sensorType,
                                  Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_FeedWater);
    virtual ~HydroponicsBinaryAnalogSensor();

    // TODO event listener maybe?
    //void addEventListener(int paramsTODO)

    bool pollState();
    bool getLastState() const;

    float getTolerance() const;
    bool getActiveBelow() const;

protected:
    bool _lastState;
    double _lastMeasurement;
    float _tolerance;
    bool _activeBelow;
};

#endif // /ifndef HydroponicsSensors_H
