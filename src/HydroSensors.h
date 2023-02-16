/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Sensors
*/

#ifndef HydroSensors_H
#define HydroSensors_H

class HydroSensor;
class HydroBinarySensor;
class HydroAnalogSensor;
class HydroDigitalSensor;
class HydroDHTTempHumiditySensor;
class HydroDSTemperatureSensor;

struct HydroSensorData;
struct HydroBinarySensorData;
struct HydroAnalogSensorData;
struct HydroDigitalSensorData;
struct HydroDHTTempHumiditySensorData;
struct HydroDSTemperatureSensorData;

#include "Hydruino.h"
#include "HydroDatas.h"

// Creates sensor object from passed sensor data (return ownership transfer - user code *must* delete returned object)
extern HydroSensor *newSensorObjectFromData(const HydroSensorData *dataIn);

// Returns default measurement units based on sensorType, optional row index, and measureMode (if undefined then uses active controller's measurement mode, else default measurement mode).
extern Hydro_UnitsType defaultUnitsForSensor(Hydro_SensorType sensorType, uint8_t measurementRow = 0, Hydro_MeasurementMode measureMode = Hydro_MeasurementMode_Undefined);
// Returns default measurement category based on sensorType and optional row index (note: this may not accurately produce the correct category, e.g. an ultrasonic distance sensor being used for distance and not volume).
extern Hydro_UnitsCategory defaultCategoryForSensor(Hydro_SensorType sensorType, uint8_t measurementRow = 0);


// Sensor Base
// This is the base class for all sensors, which defines how the sensor is identified,
// where it lives, and what it's attached to.
class HydroSensor : public HydroObject, public HydroSensorObjectInterface, public HydroMeasurementUnitsInterface, public HydroParentCropAttachmentInterface, public HydroParentReservoirAttachmentInterface {
public:
    const enum : signed char { Binary, Analog, Digital, DHT1W, DS1W, Unknown = -1 } classType; // Sensor class type (custom RTTI)
    inline bool isBinaryClass() const { return classType == Binary; }
    inline bool isAnalogClass() const { return classType == Analog; }
    inline bool isDigitalClass() const { return classType == Digital; }
    inline bool isDHTClass() const { return classType == DHT1W; }
    inline bool isDSClassType() const { return classType == DS1W; }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    HydroSensor(Hydro_SensorType sensorType,
                hposi_t sensorIndex,
                int classType = Unknown);
    HydroSensor(const HydroSensorData *dataIn);
    virtual ~HydroSensor();

    virtual void update() override;

    virtual bool takeMeasurement(bool force = false) = 0;
    virtual const HydroMeasurement *getMeasurement(bool poll = false) = 0;
    virtual bool isTakingMeasurement() const override;
    virtual bool needsPolling(hframe_t allowance = 0) const = 0;

    virtual HydroAttachment &getParentCropAttachment() override;
    virtual HydroAttachment &getParentReservoirAttachment() override;

    void setUserCalibrationData(HydroCalibrationData *userCalibrationData);
    inline const HydroCalibrationData *getUserCalibrationData() const { return _calibrationData; }

    // Transformation methods that convert from normalized reading intensity/driver value to calibration units
    inline float calibrationTransform(float value) const { return _calibrationData ? _calibrationData->transform(value) : value; }
    inline void calibrationTransform(float *valueInOut, Hydro_UnitsType *unitsOut = nullptr) const { if (valueInOut && _calibrationData) { _calibrationData->transform(valueInOut, unitsOut); } }
    inline HydroSingleMeasurement calibrationTransform(HydroSingleMeasurement measurement) { return _calibrationData ? HydroSingleMeasurement(_calibrationData->transform(measurement.value), _calibrationData->calibrationUnits, measurement.timestamp, measurement.frame) : measurement; }
    inline void calibrationTransform(HydroSingleMeasurement *measurementInOut) const { if (measurementInOut && _calibrationData) { _calibrationData->transform(&measurementInOut->value, &measurementInOut->units); } }

    // Transformation methods that convert from calibration units to normalized reading intensity/driver value
    inline float calibrationInvTransform(float value) const { return _calibrationData ? _calibrationData->inverseTransform(value) : value; }
    inline void calibrationInvTransform(float *valueInOut, Hydro_UnitsType *unitsOut = nullptr) const { if (valueInOut && _calibrationData) { _calibrationData->inverseTransform(valueInOut, unitsOut); } }
    inline HydroSingleMeasurement calibrationInvTransform(HydroSingleMeasurement measurement) { return _calibrationData ? HydroSingleMeasurement(_calibrationData->inverseTransform(measurement.value), _calibrationData->calibrationUnits, measurement.timestamp, measurement.frame) : measurement; }
    inline void calibrationInvTransform(HydroSingleMeasurement *measurementInOut) const { if (measurementInOut && _calibrationData) { _calibrationData->inverseTransform(&measurementInOut->value, &measurementInOut->units); } }

    inline Hydro_SensorType getSensorType() const { return _id.objTypeAs.sensorType; }
    inline hposi_t getSensorIndex() const { return _id.posIndex; }

    Signal<const HydroMeasurement *, HYDRO_SENSOR_SIGNAL_SLOTS> &getMeasurementSignal();

protected:
    bool _isTakingMeasure;                                  // Taking measurement flag
    HydroAttachment _parentCrop;                            // Parent crop attachment
    HydroAttachment _parentReservoir;                       // Parent reservoir attachment
    const HydroCalibrationData *_calibrationData;           // Calibration data
    Signal<const HydroMeasurement *, HYDRO_SENSOR_SIGNAL_SLOTS> _measureSignal; // New measurement signal

    virtual HydroData *allocateData() const override;
    virtual void saveToData(HydroData *dataOut) override;
};


// Simple Binary Sensor
// This class can both read from and assign interrupt routines to a digital input signal,
// allowing it to act as an on/off switch of sorts. Examples include water level indicators.
class HydroBinarySensor : public HydroSensor {
public:
    HydroBinarySensor(Hydro_SensorType sensorType,
                      hposi_t sensorIndex,
                      HydroDigitalPin inputPin,
                      int classType = Binary);
    HydroBinarySensor(const HydroBinarySensorData *dataIn);
    virtual ~HydroBinarySensor();

    virtual bool takeMeasurement(bool force = false) override;
    virtual const HydroMeasurement *getMeasurement(bool poll = false) override;
    virtual bool needsPolling(hframe_t allowance = 0) const override;

    virtual void setMeasurementUnits(Hydro_UnitsType measurementUnits, uint8_t measurementRow = 0) override;
    virtual Hydro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const override;

    bool tryRegisterAsISR();

    inline const HydroDigitalPin &getInputPin() const { return _inputPin; }

    Signal<bool, HYDRO_SENSOR_SIGNAL_SLOTS> &getStateSignal();

    inline void notifyISRTriggered() { takeMeasurement(true); }

protected:
    HydroDigitalPin _inputPin;                              // Digital input pin
    bool _usingISR;                                         // Using ISR flag
    HydroBinaryMeasurement _lastMeasurement;                // Latest successful measurement
    Signal<bool, HYDRO_SENSOR_SIGNAL_SLOTS> _stateSignal;   // State changed signal

    virtual void saveToData(HydroData *dataOut) override;
};


// Standard Analog Sensor
// The ever reliant master of the analog read, this class manages polling an analog input
// signal and converting it into the proper figures for use. Examples include everything
// from TDS EC meters to PWM based flow sensors.
class HydroAnalogSensor : public HydroSensor, public HydroMeasurementUnitsInterfaceStorageSingle {
public:
    HydroAnalogSensor(Hydro_SensorType sensorType,
                      hposi_t sensorIndex,
                      HydroAnalogPin inputPin,
                      bool inputInversion = false,
                      int classType = Analog);
    HydroAnalogSensor(const HydroAnalogSensorData *dataIn);

    virtual bool takeMeasurement(bool force = false) override;
    virtual const HydroMeasurement *getMeasurement(bool poll = false) override;
    virtual bool needsPolling(hframe_t allowance = 0) const override;

    virtual void setMeasurementUnits(Hydro_UnitsType measurementUnits, uint8_t measurementRow = 0) override;
    virtual Hydro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const override;

    inline const HydroAnalogPin &getInputPin() const { return _inputPin; }
    inline bool getInputInversion() const { return _inputInversion; }

protected:
    HydroAnalogPin _inputPin;                               // Analog input pin
    bool _inputInversion;                                   // Analog input inversion
    HydroSingleMeasurement _lastMeasurement;                // Latest successful measurement

    void _takeMeasurement(unsigned int taskId);

    virtual void saveToData(HydroData *dataOut) override;
};


// Digital Sensor
// Intermediate class for all digital sensors.
class HydroDigitalSensor : public HydroSensor {
public:
    HydroDigitalSensor(Hydro_SensorType sensorType,
                       hposi_t sensorIndex,
                       HydroDigitalPin inputPin,
                       uint8_t bitRes1W = 9,
                       bool allocate1W = false,
                       int classType = Digital);
    HydroDigitalSensor(const HydroDigitalSensorData *dataIn, bool allocate1W = false);

    virtual bool setWirePositionIndex(hposi_t wirePosIndex);
    virtual hposi_t getWirePositionIndex() const;

    virtual bool setWireDeviceAddress(const uint8_t wireDevAddress[8]);
    virtual const uint8_t *getWireDeviceAddress() const;

    inline OneWire *getOneWire() const { return _oneWire; }

protected:
    HydroDigitalPin _inputPin;                              // Digital input pin
    OneWire *_oneWire;                                      // OneWire comm instance (strong, nullptr when not used)
    uint8_t _wireBitRes;                                    // OneWire bit resolution
    hposi_t _wirePosIndex;                                  // OneWire sensor position index
    uint8_t _wireDevAddress[8];                             // OneWire sensor device address

    void resolveDeviceAddress();

    virtual void saveToData(HydroData *dataOut) override;
};


// Digital DHT* Temperature & Humidity Sensor
// This class is for working with DHT* OneWire-based air temperature and humidity sensors.
class HydroDHTTempHumiditySensor : public HydroDigitalSensor, public HydroMeasurementUnitsInterfaceStorageTriple {
public:
    HydroDHTTempHumiditySensor(hposi_t sensorIndex,
                               HydroDigitalPin inputPin,
                               Hydro_DHTType dhtType,
                               bool computeHeatIndex = true,
                               int classType = DHT1W);
    HydroDHTTempHumiditySensor(const HydroDHTTempHumiditySensorData *dataIn);
    virtual ~HydroDHTTempHumiditySensor();

    virtual bool takeMeasurement(bool force = false) override;
    virtual const HydroMeasurement *getMeasurement(bool poll = false) override;
    virtual bool needsPolling(hframe_t allowance = 0) const override;

    inline uint8_t getMeasurementRowForTemperature() const { return 0; }
    inline uint8_t getMeasurementRowForHumidity() const { return 1; }
    inline uint8_t getMeasurementRowForHeatIndex() const { return 2; }

    virtual void setMeasurementUnits(Hydro_UnitsType measurementUnits, uint8_t measurementRow = 0) override;
    virtual Hydro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const override;

    virtual bool setWirePositionIndex(hposi_t wirePosIndex) override; // disabled
    virtual hposi_t getWirePositionIndex() const override; // disabled

    virtual bool setWireDeviceAddress(const uint8_t wireDevAddress[8]) override; // disabled
    virtual const uint8_t *getWireDeviceAddress() const override; // disabled

    void setComputeHeatIndex(bool computeHeatIndex);
    inline bool getComputeHeatIndex() const { return _computeHeatIndex; }

protected:
    Hydro_DHTType _dhtType;                                 // DHT sensor type
    DHT *_dht;                                              // DHT sensor instance (owned)
    bool _computeHeatIndex;                                 // Flag to compute heat index
    HydroTripleMeasurement _lastMeasurement;                // Latest successful measurement

    void _takeMeasurement(unsigned int taskId);

    virtual void saveToData(HydroData *dataOut) override;
};


// Digital DS18* Submersible Temperature Sensor
// This class is for working with DS18* OneWire-based submersible temperature sensors.
class HydroDSTemperatureSensor : public HydroDigitalSensor, public HydroMeasurementUnitsInterfaceStorageSingle {
public:
    HydroDSTemperatureSensor(hposi_t sensorIndex,
                             HydroDigitalPin inputPin,
                             uint8_t bitRes1W = 9,
                             HydroDigitalPin pullupPin = HydroDigitalPin(),
                             int classType = DS1W);
    HydroDSTemperatureSensor(const HydroDSTemperatureSensorData *dataIn);
    virtual ~HydroDSTemperatureSensor();

    virtual bool takeMeasurement(bool force = false) override;
    virtual const HydroMeasurement *getMeasurement(bool poll = false) override;
    virtual bool needsPolling(hframe_t allowance = 0) const override;

    inline uint8_t getMeasurementRowForTemperature() const { return 0; }

    virtual void setMeasurementUnits(Hydro_UnitsType measurementUnits, uint8_t measurementRow = 0) override;
    virtual Hydro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const override;

    inline const HydroDigitalPin &getPullupPin() const { return _pullupPin; }

protected:
    DallasTemperature *_dt;                                 // DallasTemperature instance (owned)
    HydroDigitalPin _pullupPin;                             // Pullup pin, if used
    HydroSingleMeasurement _lastMeasurement;                // Latest successful measurement

    void _takeMeasurement(unsigned int taskId);

    virtual void saveToData(HydroData *dataOut) override;
};


// Sensor Serialization Data
struct HydroSensorData : public HydroObjectData {
    HydroPinData inputPin;
    char cropName[HYDRO_NAME_MAXSIZE];
    char reservoirName[HYDRO_NAME_MAXSIZE];

    HydroSensorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Binary Sensor Serialization Data
struct HydroBinarySensorData : public HydroSensorData {
    bool usingISR;

    HydroBinarySensorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Analog Sensor Serialization Data
struct HydroAnalogSensorData : public HydroSensorData {
    bool inputInversion;
    Hydro_UnitsType measurementUnits;

    HydroAnalogSensorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Digital Sensor Serialization Data
struct HydroDigitalSensorData : public HydroSensorData {
    uint8_t wireBitRes;
    hposi_t wirePosIndex;
    uint8_t wireDevAddress[8];

    HydroDigitalSensorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// DHT TempHumid Sensor Serialization Data
struct HydroDHTTempHumiditySensorData : public HydroDigitalSensorData {
    Hydro_DHTType dhtType;
    bool computeHeatIndex;
    Hydro_UnitsType measurementUnits;

    HydroDHTTempHumiditySensorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// DS Temp Sensor Serialization Data
struct HydroDSTemperatureSensorData : public HydroDigitalSensorData {
    HydroPinData pullupPin;
    Hydro_UnitsType measurementUnits;

    HydroDSTemperatureSensorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroSensors_H
