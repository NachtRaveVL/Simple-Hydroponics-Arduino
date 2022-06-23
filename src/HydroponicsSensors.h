/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensors
*/

#ifndef HydroponicsSensors_H
#define HydroponicsSensors_H

class HydroponicsSensor;
class HydroponicsBinarySensor;
class HydroponicsAnalogSensor;
class HydroponicsDHTTempHumiditySensor;
class HydroponicsDSTemperatureSensor;
class HydroponicsTMPSoilMoistureSensor;

#include "Hydroponics.h"

// Returns default measurement units to use based on sensorType and measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultSensorMeasurementUnits(Hydroponics_SensorType sensorType,
                                                           Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);

// Hydroponics Sensor Base
// This is the base class for all sensors, which defines how the sensor is identified,
// where it lives, and what it's attached to.
class HydroponicsSensor : public HydroponicsObject {
public:
    const enum { Binary, Analog, Digital, DHT1W, DS1W, TMP1W, Unknown = -1 } classType; // Sensor class type (custom RTTI)
    inline bool isBinaryClass() { return classType == Binary; }
    inline bool isAnalogClass() { return classType == Analog; }
    inline bool isDigitalClass() { return classType == Digital; }
    inline bool isDHTClass() { return classType == DHT1W; }
    inline bool isDSClassType() { return classType == DS1W; }
    inline bool isTMPClass() { return classType == TMP1W; }
    inline bool isUnknownClass() { return classType == Unknown; }
    inline bool isAnyAnalogClass() { return isAnalogClass(); }
    inline bool isAnyDigitalClass() { return isDigitalClass() || isDHTClass() || isDSClassType() || isTMPClass(); }

    HydroponicsSensor(Hydroponics_SensorType sensorType,
                      Hydroponics_PositionIndex sensorIndex,
                      byte inputPin = -1,
                      int classType = Unknown);
    virtual ~HydroponicsSensor();

    virtual void resolveLinks() override;

    virtual HydroponicsMeasurement *getMeasurement(bool force = false) = 0;
    virtual HydroponicsMeasurement *getLastMeasurement() = 0;

    virtual void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow = 0) = 0;
    virtual Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const = 0;

    void setCrop(HydroponicsIdentity cropId);
    void setCrop(shared_ptr<HydroponicsCrop> crop);
    shared_ptr<HydroponicsCrop> getCrop();

    void setReservoir(HydroponicsIdentity reservoirId);
    void setReservoir(shared_ptr<HydroponicsReservoir> reservoir);
    shared_ptr<HydroponicsReservoir> getReservoir();

    byte getInputPin() const;
    Hydroponics_SensorType getSensorType() const;
    Hydroponics_PositionIndex getSensorIndex() const;

    Signal<HydroponicsMeasurement *> &getMeasurementSignal();

protected:
    byte _inputPin;                                         // Input pin
    HydroponicsDLinkObject<HydroponicsCrop> _crop;          // Crop linkage
    HydroponicsDLinkObject<HydroponicsReservoir> _reservoir; // Reservoir linkage
    Signal<HydroponicsMeasurement *> _measureSignal;        // New measurement signal
};


// Simple Binary Sensor
// This class can both read from and assign interrupt routines to a digital input signal,
// allowing it to act as an on/off switch of sorts. Examples include water level indicators.
class HydroponicsBinarySensor : public HydroponicsSensor {
public:
    HydroponicsBinarySensor(Hydroponics_SensorType sensorType,
                            Hydroponics_PositionIndex sensorIndex,
                            byte inputPin,
                            bool activeLow = true,
                            int classType = Binary);
    virtual ~HydroponicsBinarySensor();

    HydroponicsMeasurement *getMeasurement(bool force = false) override;
    HydroponicsMeasurement *getLastMeasurement() override;

    void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow = 0) override;
    Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const override;

    // TODO reg as isr
    //bool tryRegisterAsISR();

    bool getActiveLow() const;

    Signal<bool> &getStateSignal();

protected:
    bool _activeLow;                                        // Active when low flag
    // TODO isr reg state data
    HydroponicsBinaryMeasurement _lastMeasurement;          // Latest successful measurement
    Hydroponics_UnitsType _measurementUnits;                // Preferred measurement units
    Signal<bool> _stateSignal;                              // State changed signal
};


// Standard Analog Sensor
// The ever reliant master of the analog read, this class manages polling an analog input
// signal and converting it into the proper figures for use. Examples include everything
// from TDS EC meters to PWM based flow sensors.
class HydroponicsAnalogSensor : public HydroponicsSensor {
public:
    HydroponicsAnalogSensor(Hydroponics_SensorType sensorType,
                            Hydroponics_PositionIndex sensorIndex,
                            byte inputPin,
                            byte inputBitRes = 8,
                            int classType = Analog);
    virtual ~HydroponicsAnalogSensor();

    virtual void resolveLinks() override;

    HydroponicsMeasurement *getMeasurement(bool force = false) override;
    HydroponicsMeasurement *getLastMeasurement() override;

    void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow = 0) override;
    Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const override;

    HydroponicsCalibrationData *loadInputCalibration(bool create = true);
    void saveInputCalibration(HydroponicsCalibrationData *newData);

    void setTemperatureSensor(HydroponicsIdentity sensorId);
    void setTemperatureSensor(shared_ptr<HydroponicsSensor> sensor);
    shared_ptr<HydroponicsSensor> getTemperatureSensor();

    HydroponicsBitResolution getInputResolution() const;

protected:
    HydroponicsBitResolution _inputResolution;              // Analog input resolution
    HydroponicsSingleMeasurement _lastMeasurement;          // Latest successful measurement
    Hydroponics_UnitsType _measurementUnits;                // Preferred measurement units
    HydroponicsCalibrationData *_inputCalib;                // Input calibration data if loaded (owned)
    HydroponicsDLinkObject<HydroponicsSensor> _tempSensor;  // Temperature sensor linkage (for temperature adjusted measurements)
};


// TODO
class HydroponicsDigitalSensor : public HydroponicsSensor {
public:
    HydroponicsDigitalSensor(Hydroponics_SensorType sensorType,
                             Hydroponics_PositionIndex sensorIndex,
                             byte inputPin = -1,
                             int classType = Digital);
    virtual ~HydroponicsDigitalSensor();

protected:
};


// Digital DHT* Temperature & Humidity Sensor
// This class is for working with DHT* OneWire-based air temperature and humidity sensors.
class HydroponicsDHTTempHumiditySensor : public HydroponicsDigitalSensor {
public:
    HydroponicsDHTTempHumiditySensor(Hydroponics_PositionIndex sensorIndex,
                                     byte inputPin,
                                     uint8_t dhtType = DHT12,
                                     bool computeHeatIndex = true,
                                     int classType = DHT1W);
    virtual ~HydroponicsDHTTempHumiditySensor();

    HydroponicsMeasurement *getMeasurement(bool force = false) override;
    HydroponicsMeasurement *getLastMeasurement() override;

    void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow = 0) override;
    Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const override;

    void setComputeHeatIndex(bool computeHeatIndex);
    bool getComputeHeatIndex() const;

protected:
    DHT *_dht;                                              // DHT sensor instance (owned)
    bool _computeHeatIndex;                                 // Flag to compute heat index
    HydroponicsTripleMeasurement _lastMeasurement;          // Latest successful measurement
    Hydroponics_UnitsType _measurementUnits[3];             // Preferred measurement units
};


// Digital DS18* Submersible Temperature Sensor
// This class is for working with DS18* OneWire-based submersible temperature sensors.
class HydroponicsDSTemperatureSensor : public HydroponicsDigitalSensor {
public:
    HydroponicsDSTemperatureSensor(Hydroponics_PositionIndex sensorIndex,
                                   byte inputPin,
                                   byte inputBitRes = 9,
                                   int classType = DS1W);
    virtual ~HydroponicsDSTemperatureSensor();

    HydroponicsMeasurement *getMeasurement(bool force = false) override;
    HydroponicsMeasurement *getLastMeasurement() override;

    void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow = 0) override;
    Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const override;

    OneWire &getOneWire() const;

protected:
    OneWire *_oneWire;                                      // OneWire comm instance (owned)
    DallasTemperature *_dt;                                 // DallasTemperature instance (owned)
    HydroponicsSingleMeasurement _lastMeasurement;          // Latest successful measurement
    Hydroponics_UnitsType _measurementUnits;                // Preferred measurement units
};


// Digital TMP* Soil Moisture Sensor
// This class is for working with TMP* OneWire-based soil moisture sensors.
class HydroponicsTMPSoilMoistureSensor : public HydroponicsDigitalSensor {
public:
    HydroponicsTMPSoilMoistureSensor(Hydroponics_PositionIndex sensorIndex,
                                     byte inputPin,
                                     byte inputBitRes = 9,
                                     int classType = TMP1W);
    virtual ~HydroponicsTMPSoilMoistureSensor();

    HydroponicsMeasurement *getMeasurement(bool force = false) override;
    HydroponicsMeasurement *getLastMeasurement() override;

    void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow = 0) override;
    Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const override;

    OneWire &getOneWire() const;

protected:
    OneWire *_oneWire;                                      // OneWire comm instance (owned)
    // TODO: Find class for working with this one
    HydroponicsSingleMeasurement _lastMeasurement;          // Latest successful measurement
    Hydroponics_UnitsType _measurementUnits;                // Preferred measurement units
};

#endif // /ifndef HydroponicsSensors_H
