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
class HydroponicsTMPMoistureSensor;

struct HydroponicsSensorData;
struct HydroponicsBinarySensorData;
struct HydroponicsAnalogSensorData;
struct HydroponicsDigitalSensorData;
struct HydroponicsDHTTempHumiditySensorData;
struct HydroponicsDSTemperatureSensorData;
struct HydroponicsTMPMoistureSensorData;

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
    inline bool isBinaryClass() const { return classType == Binary; }
    inline bool isAnalogClass() const { return classType == Analog; }
    inline bool isDigitalClass() const { return classType == Digital; }
    inline bool isDHTClass() const { return classType == DHT1W; }
    inline bool isDSClassType() const { return classType == DS1W; }
    inline bool isTMPClass() const { return classType == TMP1W; }
    inline bool isUnknownClass() const { return classType <= Unknown; }
    inline bool isAnyAnalogClass() const { return isAnalogClass(); }
    inline bool isAnyDigitalClass() const { return isDigitalClass() || isDHTClass() || isDSClassType() || isTMPClass(); }

    HydroponicsSensor(Hydroponics_SensorType sensorType,
                      Hydroponics_PositionIndex sensorIndex,
                      byte inputPin = -1,
                      int classType = Unknown);
    HydroponicsSensor(const HydroponicsSensorData *dataIn);
    virtual ~HydroponicsSensor();

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    virtual bool takeMeasurement(bool override = false) = 0;
    virtual const HydroponicsMeasurement *getLatestMeasurement() const = 0;
    virtual bool getIsTakingMeasurement() const override;
    virtual bool getNeedsPolling() const override;

    virtual void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow = 0) = 0;
    virtual Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const = 0;

    virtual void setCrop(HydroponicsIdentity cropId) override;
    virtual void setCrop(shared_ptr<HydroponicsCrop> crop) override;
    virtual shared_ptr<HydroponicsCrop> getCrop() override;

    virtual void setReservoir(HydroponicsIdentity reservoirId) override;
    virtual void setReservoir(shared_ptr<HydroponicsReservoir> reservoir) override;
    virtual shared_ptr<HydroponicsReservoir> getReservoir() override;

    void setUserCalibrationData(HydroponicsCalibrationData *userCalibrationData);
    const HydroponicsCalibrationData *getUserCalibrationData() const;

    byte getInputPin() const;
    Hydroponics_SensorType getSensorType() const;
    Hydroponics_PositionIndex getSensorIndex() const;

    Signal<const HydroponicsMeasurement *> &getMeasurementSignal();

protected:
    byte _inputPin;                                         // Input pin
    bool _isTakingMeasure;                                  // Taking measurement flag
    HydroponicsDLinkObject<HydroponicsCrop> _crop;          // Crop linkage
    HydroponicsDLinkObject<HydroponicsReservoir> _reservoir; // Reservoir linkage
    const HydroponicsCalibrationData *_calibrationData;     // Calibration data
    Signal<const HydroponicsMeasurement *> _measureSignal;        // New measurement signal

    virtual HydroponicsData *allocateData() const override;
    virtual void saveToData(HydroponicsData *dataOut) override;
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
    HydroponicsBinarySensor(const HydroponicsBinarySensorData *dataIn);
    virtual ~HydroponicsBinarySensor();

    virtual bool takeMeasurement(bool override = false) override;
    virtual const HydroponicsMeasurement *getLatestMeasurement() const override;

    virtual void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow = 0) override;
    virtual Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const override;

    // TODO reg as isr
    //bool tryRegisterAsISR();

    bool getActiveLow() const;

    Signal<bool> &getStateSignal();

protected:
    bool _activeLow;                                        // Active when low flag
    // TODO isr reg state data
    HydroponicsBinaryMeasurement _lastMeasurement;          // Latest successful measurement
    Signal<bool> _stateSignal;                              // State changed signal

    virtual void saveToData(HydroponicsData *dataOut) override;
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
                            bool inputInversion = false,
                            int classType = Analog);
    HydroponicsAnalogSensor(const HydroponicsAnalogSensorData *dataIn);
    virtual ~HydroponicsAnalogSensor();

    virtual void resolveLinks() override;

    virtual bool takeMeasurement(bool override = false) override;
    virtual const HydroponicsMeasurement *getLatestMeasurement() const override;

    virtual void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow = 0) override;
    virtual Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const override;

    void setTemperatureSensor(HydroponicsIdentity sensorId);
    void setTemperatureSensor(shared_ptr<HydroponicsSensor> sensor);
    shared_ptr<HydroponicsSensor> getTemperatureSensor();

    HydroponicsBitResolution getInputResolution() const;
    bool getInputInversion() const;

protected:
    HydroponicsBitResolution _inputResolution;              // Analog input resolution
    bool _inputInversion;                                   // Analog input inversion
    HydroponicsSingleMeasurement _lastMeasurement;          // Latest successful measurement
    Hydroponics_UnitsType _measurementUnits;                // Measurement units preferred
    HydroponicsDLinkObject<HydroponicsSensor> _tempSensor;  // Temperature sensor linkage

    void _takeMeasurement(taskid_t taskId);

    virtual void saveToData(HydroponicsData *dataOut) override;
};


// Digital Sensor
// Intermediate class for all digital sensors.
class HydroponicsDigitalSensor : public HydroponicsSensor {
public:
    HydroponicsDigitalSensor(Hydroponics_SensorType sensorType,
                             Hydroponics_PositionIndex sensorIndex,
                             byte inputPin = -1, byte inputBitRes = 9,
                             bool allocate1W = false,
                             int classType = Digital);
    HydroponicsDigitalSensor(const HydroponicsDigitalSensorData *dataIn, bool allocate1W = false);
    virtual ~HydroponicsDigitalSensor();

    virtual void resolveLinks() override;

    virtual bool setWirePositionIndex(Hydroponics_PositionIndex wirePosIndex);
    virtual Hydroponics_PositionIndex getWirePositionIndex() const;

    virtual bool setWireDeviceAddress(const uint8_t wireDevAddress[8]);
    virtual const uint8_t *getWireDeviceAddress() const;

    void setTemperatureSensor(HydroponicsIdentity sensorId);
    void setTemperatureSensor(shared_ptr<HydroponicsSensor> sensor);
    shared_ptr<HydroponicsSensor> getTemperatureSensor();

    OneWire *getOneWire();

protected:
    byte _inputBitRes;                                      // Input bit resolution
    OneWire *_oneWire;                                      // OneWire comm instance (strong, nullptr when not used)
    Hydroponics_PositionIndex _wirePosIndex;                // OneWire sensor position index
    uint8_t _wireDevAddress[8];                             // OneWire sensor device address
    HydroponicsDLinkObject<HydroponicsSensor> _tempSensor;  // Temperature sensor linkage

    void resolveDeviceAddress();

    virtual void saveToData(HydroponicsData *dataOut) override;
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

    virtual bool takeMeasurement(bool override = false) override;
    virtual const HydroponicsMeasurement *getLatestMeasurement() const override;

    virtual void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow) override;
    virtual Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const override;

    virtual bool setWirePositionIndex(Hydroponics_PositionIndex wirePosIndex) override; // disabled
    virtual Hydroponics_PositionIndex getWirePositionIndex() const override; // disabled

    virtual bool setWireDeviceAddress(const uint8_t wireDevAddress[8]) override; // disabled
    virtual const uint8_t *getWireDeviceAddress() const override; // disabled

    void setComputeHeatIndex(bool computeHeatIndex);
    bool getComputeHeatIndex() const;

protected:
    byte _dhtType;                                          // DHT type
    DHT *_dht;                                              // DHT sensor instance (owned)
    bool _computeHeatIndex;                                 // Flag to compute heat index
    HydroponicsTripleMeasurement _lastMeasurement;          // Latest successful measurement
    Hydroponics_UnitsType _measurementUnits[3];             // Measurement units preferred

    void _takeMeasurement(taskid_t taskId);

    virtual void saveToData(HydroponicsData *dataOut) override;
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

    virtual bool takeMeasurement(bool override = false) override;
    virtual const HydroponicsMeasurement *getLatestMeasurement() const override;

    virtual void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow = 0) override;
    virtual Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const override;

    void setPullupPin(byte pullupPin);
    byte getPullupPin() const;

protected:
    DallasTemperature *_dt;                                 // DallasTemperature instance (owned)
    byte _pullupPin;                                        // Pullup pin (if used, else -1)
    HydroponicsSingleMeasurement _lastMeasurement;          // Latest successful measurement
    Hydroponics_UnitsType _measurementUnits;                // Measurement units preferred

    void _takeMeasurement(taskid_t taskId);

    virtual void saveToData(HydroponicsData *dataOut) override;
};


// Digital TMP* Soil Moisture Sensor
// This class is for working with TMP* OneWire-based soil moisture sensors.
class HydroponicsTMPMoistureSensor : public HydroponicsDigitalSensor {
public:
    HydroponicsTMPMoistureSensor(Hydroponics_PositionIndex sensorIndex,
                                 byte inputPin,
                                 byte inputBitRes = 9,
                                 int classType = TMP1W);
    HydroponicsTMPMoistureSensor(const HydroponicsTMPMoistureSensorData *dataIn);
    virtual ~HydroponicsTMPMoistureSensor();

    virtual bool takeMeasurement(bool override = false) override;
    virtual const HydroponicsMeasurement *getLatestMeasurement() const override;

    virtual void setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow = 0) override;
    virtual Hydroponics_UnitsType getMeasurementUnits(int measurementRow = 0) const override;

protected:
    // TODO: Find class for working with this one
    HydroponicsSingleMeasurement _lastMeasurement;          // Latest successful measurement
    Hydroponics_UnitsType _measurementUnits;                // Measurement units preferred

    virtual void saveToData(HydroponicsData *dataOut) override;
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
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Analog Sensor Serialization Data
struct HydroponicsAnalogSensorData : public HydroponicsSensorData {
    byte inputBitRes;
    bool inputInversion;
    Hydroponics_UnitsType measurementUnits;
    char tempSensorName[HYDRUINO_NAME_MAXSIZE];

    HydroponicsAnalogSensorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Digital Sensor Serialization Data
struct HydroponicsDigitalSensorData : public HydroponicsSensorData {
    byte inputBitRes;
    Hydroponics_PositionIndex wirePosIndex;
    uint8_t wireDevAddress[8];
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
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// DS Temp Sensor Serialization Data
struct HydroponicsDSTemperatureSensorData : public HydroponicsDigitalSensorData {
    byte pullupPin;
    Hydroponics_UnitsType measurementUnits;

    HydroponicsDSTemperatureSensorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// TMP Moisture Sensor Serialization Data
struct HydroponicsTMPMoistureSensorData : public HydroponicsDigitalSensorData {
    Hydroponics_UnitsType measurementUnits;

    HydroponicsTMPMoistureSensorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroponicsSensors_H
