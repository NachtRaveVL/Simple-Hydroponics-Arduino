/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensors
*/

#ifndef HydroponicsSensors_H
#define HydroponicsSensors_H

class HydroponicsSensor;
class HydroponicsBinarySensor;
class HydroponicsAnalogSensor;
class HydroponicsDigitalSensor;
class HydroponicsDHTTempHumiditySensor;
class HydroponicsDSTemperatureSensor;
class HydroponicsTMPSoilMoistureSensor;

struct HydroponicsSensorData;
struct HydroponicsBinarySensorData;
struct HydroponicsAnalogSensorData;
struct HydroponicsDigitalSensorData;
struct HydroponicsDHTTempHumiditySensorData;
struct HydroponicsDSTemperatureSensorData;
struct HydroponicsTMPSoilMoistureSensorData;

#include "Hydroponics.h"

// Creates sensor object from passed sensor data (return ownership transfer - user code *must* delete returned object)
extern HydroponicsSensor *newSensorObjectFromData(const HydroponicsSensorData *dataIn);

// Returns default measurement units to use based on sensorType and measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultMeasureUnitsForSensorType(Hydroponics_SensorType sensorType,
                                                           Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);


// Hydroponics Sensor Base
// This is the base class for all sensors, which defines how the sensor is identified,
// where it lives, and what it's attached to.
class HydroponicsSensor : public HydroponicsObject, public HydroponicsSensorObjectInterface, public HydroponicsCropAttachmentInterface, public HydroponicsReservoirAttachmentInterface {
public:
    const enum { Binary, Analog, Digital, DHT1W, DS1W, TMP1W, Unknown = -1 } classType; // Sensor class type (custom RTTI)
    inline bool isBinaryClass() { return classType == Binary; }
    inline bool isAnalogClass() { return classType == Analog; }
    inline bool isDigitalClass() { return classType == Digital; }
    inline bool isDHTClass() { return classType == DHT1W; }
    inline bool isDSClassType() { return classType == DS1W; }
    inline bool isTMPClass() { return classType == TMP1W; }
    inline bool isUnknownClass() { return classType <= Unknown; }
    inline bool isAnyAnalogClass() { return isAnalogClass(); }
    inline bool isAnyDigitalClass() { return isDigitalClass() || isDHTClass() || isDSClassType() || isTMPClass(); }

    HydroponicsSensor(Hydroponics_SensorType sensorType,
                      Hydroponics_PositionIndex sensorIndex,
                      byte inputPin = -1,
                      int classType = Unknown);
    HydroponicsSensor(const HydroponicsSensorData *dataIn);
    virtual ~HydroponicsSensor();

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    virtual void takeMeasurement(bool override = false) = 0;
    virtual const HydroponicsMeasurement *getLatestMeasurement() const = 0;
    virtual bool getIsTakingMeasurement() const override;
    virtual bool getNeedsPolling() const override;

    virtual void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow = 0) = 0;
    virtual Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const = 0;

    void setCrop(HydroponicsIdentity cropId) override;
    void setCrop(shared_ptr<HydroponicsCrop> crop) override;
    shared_ptr<HydroponicsCrop> getCrop() override;

    void setReservoir(HydroponicsIdentity reservoirId) override;
    void setReservoir(shared_ptr<HydroponicsReservoir> reservoir) override;
    shared_ptr<HydroponicsReservoir> getReservoir() override;

    void setUserCalibrationData(HydroponicsCalibrationData *userCalibrationData);
    const HydroponicsCalibrationData *getUserCalibrationData() const;

    byte getInputPin() const;
    Hydroponics_SensorType getSensorType() const;
    Hydroponics_PositionIndex getSensorIndex() const;

    Signal<HydroponicsMeasurement *> &getMeasurementSignal();

protected:
    byte _inputPin;                                         // Input pin
    bool _isTakingMeasure;                                  // Taking measurement flag
    HydroponicsDLinkObject<HydroponicsCrop> _crop;          // Crop linkage
    HydroponicsDLinkObject<HydroponicsReservoir> _reservoir; // Reservoir linkage
    const HydroponicsCalibrationData *_calibrationData;     // Calibration data
    Signal<HydroponicsMeasurement *> _measureSignal;        // New measurement signal

    HydroponicsData *allocateData() const override;
    virtual void saveToData(HydroponicsData *dataOut) const override;
};


// Simple Binary Sensor
// This class can both read from and assign interrupt routines to a digital input signal,
// allowing it to act typeAs an on/off switch of sorts. Examples include water level indicators.
class HydroponicsBinarySensor : public HydroponicsSensor {
public:
    HydroponicsBinarySensor(Hydroponics_SensorType sensorType,
                            Hydroponics_PositionIndex sensorIndex,
                            byte inputPin,
                            bool activeLow = true,
                            int classType = Binary);
    HydroponicsBinarySensor(const HydroponicsBinarySensorData *dataIn);
    virtual ~HydroponicsBinarySensor();

    void takeMeasurement(bool override = false) override;
    const HydroponicsMeasurement *getLatestMeasurement() const override;

    void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow = 0) override;
    Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const override;

    // TODO reg typeAs isr
    //bool tryRegisterAsISR();

    bool getActiveLow() const;

    Signal<bool> &getStateSignal();

protected:
    bool _activeLow;                                        // Active when low flag
    // TODO isr reg state data
    HydroponicsBinaryMeasurement _lastMeasurement;          // Latest successful measurement
    Signal<bool> _stateSignal;                              // State changed signal

    void saveToData(HydroponicsData *dataOut) const override;
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
    HydroponicsAnalogSensor(const HydroponicsAnalogSensorData *dataIn);
    virtual ~HydroponicsAnalogSensor();

    virtual void resolveLinks() override;

    void takeMeasurement(bool override = false) override;
    const HydroponicsMeasurement *getLatestMeasurement() const override;

    void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow = 0) override;
    Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const override;

    void setTemperatureSensor(HydroponicsIdentity sensorId);
    void setTemperatureSensor(shared_ptr<HydroponicsSensor> sensor);
    shared_ptr<HydroponicsSensor> getTemperatureSensor();

    HydroponicsBitResolution getInputResolution() const;

protected:
    HydroponicsBitResolution _inputResolution;              // Analog input resolution
    HydroponicsSingleMeasurement _lastMeasurement;          // Latest successful measurement
    Hydroponics_UnitsType _measurementUnits;                // Measurement units preferred
    HydroponicsDLinkObject<HydroponicsSensor> _tempSensor;  // Temperature sensor linkage

    void saveToData(HydroponicsData *dataOut) const override;
};


// Digital Sensor
// Intermediate class for all digital sensors.
class HydroponicsDigitalSensor : public HydroponicsSensor {
public:
    HydroponicsDigitalSensor(Hydroponics_SensorType sensorType,
                             Hydroponics_PositionIndex sensorIndex,
                             byte inputPin = -1,
                             bool allocate1W = false,
                             int classType = Digital);
    HydroponicsDigitalSensor(const HydroponicsDigitalSensorData *dataIn, bool allocate1W = false);
    virtual ~HydroponicsDigitalSensor();

    virtual void resolveLinks() override;

    void setTemperatureSensor(HydroponicsIdentity sensorId);
    void setTemperatureSensor(shared_ptr<HydroponicsSensor> sensor);
    shared_ptr<HydroponicsSensor> getTemperatureSensor();

protected:
    OneWire *_oneWire;                                      // OneWire comm instance (owned)
    HydroponicsDLinkObject<HydroponicsSensor> _tempSensor;  // Temperature sensor linkage

    void saveToData(HydroponicsData *dataOut) const override;
};


// Digital DHT* Temperature & Humidity Sensor
// This class is for working with DHT* OneWire-based air temperature and humidity sensors.
class HydroponicsDHTTempHumiditySensor : public HydroponicsDigitalSensor {
public:
    HydroponicsDHTTempHumiditySensor(Hydroponics_PositionIndex sensorIndex,
                                     byte inputPin,
                                     byte dhtType = DHT12,
                                     bool computeHeatIndex = true,
                                     int classType = DHT1W);
    HydroponicsDHTTempHumiditySensor(const HydroponicsDHTTempHumiditySensorData *dataIn);
    virtual ~HydroponicsDHTTempHumiditySensor();

    void takeMeasurement(bool override = false) override;
    const HydroponicsMeasurement *getLatestMeasurement() const override;

    void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow = 0) override;
    Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const override;

    void setComputeHeatIndex(bool computeHeatIndex);
    bool getComputeHeatIndex() const;

protected:
    byte _dhtType;                                          // DHT type
    DHT *_dht;                                              // DHT sensor instance (owned)
    bool _computeHeatIndex;                                 // Flag to compute heat index
    HydroponicsTripleMeasurement _lastMeasurement;          // Latest successful measurement
    Hydroponics_UnitsType _measurementUnits[3];             // Measurement units preferred

    void saveToData(HydroponicsData *dataOut) const override;
};


// Digital DS18* Submersible Temperature Sensor
// This class is for working with DS18* OneWire-based submersible temperature sensors.
class HydroponicsDSTemperatureSensor : public HydroponicsDigitalSensor {
public:
    HydroponicsDSTemperatureSensor(Hydroponics_PositionIndex sensorIndex,
                                   byte inputPin,
                                   byte inputBitRes = 9,
                                   int classType = DS1W);
    HydroponicsDSTemperatureSensor(const HydroponicsDSTemperatureSensorData *dataIn);
    virtual ~HydroponicsDSTemperatureSensor();

    void takeMeasurement(bool override = false) override;
    const HydroponicsMeasurement *getLatestMeasurement() const override;

    void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow = 0) override;
    Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const override;

    OneWire &getOneWire() const;

protected:
    DallasTemperature *_dt;                                 // DallasTemperature instance (owned)
    HydroponicsSingleMeasurement _lastMeasurement;          // Latest successful measurement
    Hydroponics_UnitsType _measurementUnits;                // Measurement units preferred

    void saveToData(HydroponicsData *dataOut) const override;
};


// Digital TMP* Soil Moisture Sensor
// This class is for working with TMP* OneWire-based soil moisture sensors.
class HydroponicsTMPSoilMoistureSensor : public HydroponicsDigitalSensor {
public:
    HydroponicsTMPSoilMoistureSensor(Hydroponics_PositionIndex sensorIndex,
                                     byte inputPin,
                                     byte inputBitRes = 9,
                                     int classType = TMP1W);
    HydroponicsTMPSoilMoistureSensor(const HydroponicsTMPSoilMoistureSensorData *dataIn);
    virtual ~HydroponicsTMPSoilMoistureSensor();

    void takeMeasurement(bool override = false) override;
    const HydroponicsMeasurement *getLatestMeasurement() const override;

    void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow = 0) override;
    Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const override;

    OneWire &getOneWire() const;

protected:
    // TODO: Find class for working with this one
    byte _inputBitRes;                                      // Input bit resolution
    HydroponicsSingleMeasurement _lastMeasurement;          // Latest successful measurement
    Hydroponics_UnitsType _measurementUnits;                // Measurement units preferred

    void saveToData(HydroponicsData *dataOut) const override;
};


// Sensor Serialization Data
struct HydroponicsSensorData : public HydroponicsObjectData {
    byte inputPin;
    char cropName[HYDRUINO_NAME_MAXSIZE];
    char reservoirName[HYDRUINO_NAME_MAXSIZE];

    HydroponicsSensorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Binary Sensor Serialization Data
struct HydroponicsBinarySensorData : public HydroponicsSensorData {
    bool activeLow;
    //TODO: bool registerISR;

    HydroponicsBinarySensorData();
    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Analog Sensor Serialization Data
struct HydroponicsAnalogSensorData : public HydroponicsSensorData {
    byte inputBitRes;
    Hydroponics_UnitsType measurementUnits;
    char tempSensorName[HYDRUINO_NAME_MAXSIZE];

    HydroponicsAnalogSensorData();
    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Digital Sensor Serialization Data
struct HydroponicsDigitalSensorData : public HydroponicsSensorData {
    char tempSensorName[HYDRUINO_NAME_MAXSIZE];

    HydroponicsDigitalSensorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// DHT TempHumid Sensor Serialization Data
struct HydroponicsDHTTempHumiditySensorData : public HydroponicsDigitalSensorData {
    byte dhtType;
    bool computeHeatIndex;
    Hydroponics_UnitsType measurementUnits[3];

    HydroponicsDHTTempHumiditySensorData();
    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;
};

// DS Temp Sensor Serialization Data
struct HydroponicsDSTemperatureSensorData : public HydroponicsDigitalSensorData {
    byte inputBitRes;
    Hydroponics_UnitsType measurementUnits;

    HydroponicsDSTemperatureSensorData();
    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;
};

// TMP Moisture Sensor Serialization Data
struct HydroponicsTMPSoilMoistureSensorData : public HydroponicsDigitalSensorData {
    byte inputBitRes;
    Hydroponics_UnitsType measurementUnits;

    HydroponicsTMPSoilMoistureSensorData();
    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroponicsSensors_H
