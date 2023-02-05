/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
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

// Returns default measurement units to use based on sensorType, optional row index, and measureMode (if undefined then uses active Hydruino instance's measurement mode, else default mode).
extern Hydro_UnitsType defaultMeasureUnitsForSensorType(Hydro_SensorType sensorType, uint8_t measurementRow = 0, Hydro_MeasurementMode measureMode = Hydro_MeasurementMode_Undefined);
// Returns default measurement category to use based on sensorType and optional row index (note: this may not accurately produce the correct category, e.g. an ultrasonic distance sensor being used for distance and not volume).
extern Hydro_UnitsCategory defaultMeasureCategoryForSensorType(Hydro_SensorType sensorType, uint8_t measurementRow = 0);


// Sensor Base
// This is the base class for all sensors, which defines how the sensor is identified,
// where it lives, and what it's attached to.
class HydroSensor : public HydroObject, public HydroSensorObjectInterface, public HydroCropAttachmentInterface, public HydroReservoirAttachmentInterface {
public:
    const enum : signed char { Binary, Analog, Digital, DHT1W, DS1W, Unknown = -1 } classType; // Sensor class type (custom RTTI)
    inline bool isBinaryClass() const { return classType == Binary; }
    inline bool isAnalogClass() const { return classType == Analog; }
    inline bool isDigitalClass() const { return classType == Digital; }
    inline bool isDHTClass() const { return classType == DHT1W; }
    inline bool isDSClassType() const { return classType == DS1W; }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    HydroSensor(Hydro_SensorType sensorType,
                Hydro_PositionIndex sensorIndex,
                int classType = Unknown);
    HydroSensor(const HydroSensorData *dataIn);
    virtual ~HydroSensor();

    virtual void update() override;

    virtual bool takeMeasurement(bool force = false) = 0;
    virtual const HydroMeasurement *getLatestMeasurement() const = 0;
    virtual bool isTakingMeasurement() const override;
    virtual bool getNeedsPolling(uint32_t allowance = 0) const override;

    virtual void setMeasurementUnits(Hydro_UnitsType measurementUnits, uint8_t measurementRow = 0) = 0;
    virtual Hydro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const = 0;

    virtual HydroAttachment &getParentCrop(bool resolve = true) override;
    virtual HydroAttachment &getParentReservoir(bool resolve = true) override;

    void setUserCalibrationData(HydroCalibrationData *userCalibrationData);
    inline const HydroCalibrationData *getUserCalibrationData() const { return _calibrationData; }

    inline Hydro_SensorType getSensorType() const { return _id.objTypeAs.sensorType; }
    inline Hydro_PositionIndex getSensorIndex() const { return _id.posIndex; }

    Signal<const HydroMeasurement *, HYDRO_SENSOR_SIGNAL_SLOTS> &getMeasurementSignal();

protected:
    bool _isTakingMeasure;                                  // Taking measurement flag
    HydroAttachment _crop;                                  // Crop attachment
    HydroAttachment _reservoir;                             // Reservoir attachment
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
                      Hydro_PositionIndex sensorIndex,
                      HydroDigitalPin inputPin,
                      int classType = Binary);
    HydroBinarySensor(const HydroBinarySensorData *dataIn);
    virtual ~HydroBinarySensor();

    virtual bool takeMeasurement(bool force = false) override;
    virtual const HydroMeasurement *getLatestMeasurement() const override;

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
    Signal<bool, HYDRO_SENSOR_SIGNAL_SLOTS> _stateSignal; // State changed signal

    virtual void saveToData(HydroData *dataOut) override;
};


// Standard Analog Sensor
// The ever reliant master of the analog read, this class manages polling an analog input
// signal and converting it into the proper figures for use. Examples include everything
// from TDS EC meters to PWM based flow sensors.
class HydroAnalogSensor : public HydroSensor {
public:
    HydroAnalogSensor(Hydro_SensorType sensorType,
                      Hydro_PositionIndex sensorIndex,
                      HydroAnalogPin inputPin,
                      bool inputInversion = false,
                      int classType = Analog);
    HydroAnalogSensor(const HydroAnalogSensorData *dataIn);

    virtual bool takeMeasurement(bool force = false) override;
    virtual const HydroMeasurement *getLatestMeasurement() const override;

    virtual void setMeasurementUnits(Hydro_UnitsType measurementUnits, uint8_t measurementRow = 0) override;
    virtual Hydro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const override;

    inline const HydroAnalogPin &getInputPin() const { return _inputPin; }
    inline bool getInputInversion() const { return _inputInversion; }

protected:
    HydroAnalogPin _inputPin;                               // Analog input pin
    bool _inputInversion;                                   // Analog input inversion
    HydroSingleMeasurement _lastMeasurement;                // Latest successful measurement
    Hydro_UnitsType _measurementUnits;                      // Measurement units preferred

    void _takeMeasurement(unsigned int taskId);

    virtual void saveToData(HydroData *dataOut) override;
};


// Digital Sensor
// Intermediate class for all digital sensors.
class HydroDigitalSensor : public HydroSensor {
public:
    HydroDigitalSensor(Hydro_SensorType sensorType,
                       Hydro_PositionIndex sensorIndex,
                       HydroDigitalPin inputPin,
                       uint8_t bitRes1W = 9,
                       bool allocate1W = false,
                       int classType = Digital);
    HydroDigitalSensor(const HydroDigitalSensorData *dataIn, bool allocate1W = false);

    virtual bool setWirePositionIndex(Hydro_PositionIndex wirePosIndex);
    virtual Hydro_PositionIndex getWirePositionIndex() const;

    virtual bool setWireDeviceAddress(const uint8_t wireDevAddress[8]);
    virtual const uint8_t *getWireDeviceAddress() const;

    inline OneWire *getOneWire() const { return _oneWire; }

protected:
    HydroDigitalPin _inputPin;                              // Digital input pin
    OneWire *_oneWire;                                      // OneWire comm instance (strong, nullptr when not used)
    uint8_t _wireBitRes;                                    // OneWire bit resolution
    Hydro_PositionIndex _wirePosIndex;                      // OneWire sensor position index
    uint8_t _wireDevAddress[8];                             // OneWire sensor device address

    void resolveDeviceAddress();

    virtual void saveToData(HydroData *dataOut) override;
};


// Digital DHT* Temperature & Humidity Sensor
// This class is for working with DHT* OneWire-based air temperature and humidity sensors.
class HydroDHTTempHumiditySensor : public HydroDigitalSensor {
public:
    HydroDHTTempHumiditySensor(Hydro_PositionIndex sensorIndex,
                               HydroDigitalPin inputPin,
                               Hydro_DHTType dhtType,
                               bool computeHeatIndex = true,
                               int classType = DHT1W);
    HydroDHTTempHumiditySensor(const HydroDHTTempHumiditySensorData *dataIn);
    virtual ~HydroDHTTempHumiditySensor();

    virtual bool takeMeasurement(bool force = false) override;
    virtual const HydroMeasurement *getLatestMeasurement() const override;

    inline uint8_t getMeasurementRowForTemperature() const { return 0; }
    inline uint8_t getMeasurementRowForHumidity() const { return 1; }
    inline uint8_t getMeasurementRowForHeatIndex() const { return 2; }

    virtual void setMeasurementUnits(Hydro_UnitsType measurementUnits, uint8_t measurementRow) override;
    virtual Hydro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const override;

    virtual bool setWirePositionIndex(Hydro_PositionIndex wirePosIndex) override; // disabled
    virtual Hydro_PositionIndex getWirePositionIndex() const override; // disabled

    virtual bool setWireDeviceAddress(const uint8_t wireDevAddress[8]) override; // disabled
    virtual const uint8_t *getWireDeviceAddress() const override; // disabled

    void setComputeHeatIndex(bool computeHeatIndex);
    inline bool getComputeHeatIndex() const { return _computeHeatIndex; }

protected:
    Hydro_DHTType _dhtType;                                 // DHT sensor type
    DHT *_dht;                                              // DHT sensor instance (owned)
    bool _computeHeatIndex;                                 // Flag to compute heat index
    HydroTripleMeasurement _lastMeasurement;                // Latest successful measurement
    Hydro_UnitsType _measurementUnits[3];                   // Measurement units preferred

    void _takeMeasurement(unsigned int taskId);

    virtual void saveToData(HydroData *dataOut) override;
};


// Digital DS18* Submersible Temperature Sensor
// This class is for working with DS18* OneWire-based submersible temperature sensors.
class HydroDSTemperatureSensor : public HydroDigitalSensor {
public:
    HydroDSTemperatureSensor(Hydro_PositionIndex sensorIndex,
                             HydroDigitalPin inputPin,
                             uint8_t bitRes1W = 9,
                             HydroDigitalPin pullupPin = HydroDigitalPin(),
                             int classType = DS1W);
    HydroDSTemperatureSensor(const HydroDSTemperatureSensorData *dataIn);
    virtual ~HydroDSTemperatureSensor();

    virtual bool takeMeasurement(bool force = false) override;
    virtual const HydroMeasurement *getLatestMeasurement() const override;

    inline uint8_t getMeasurementRowForTemperature() const { return 0; }

    virtual void setMeasurementUnits(Hydro_UnitsType measurementUnits, uint8_t measurementRow = 0) override;
    virtual Hydro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const override;

    inline const HydroDigitalPin &getPullupPin() const { return _pullupPin; }

protected:
    DallasTemperature *_dt;                                 // DallasTemperature instance (owned)
    HydroDigitalPin _pullupPin;                             // Pullup pin, if used
    HydroSingleMeasurement _lastMeasurement;                // Latest successful measurement
    Hydro_UnitsType _measurementUnits;                      // Measurement units preferred

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
    Hydro_PositionIndex wirePosIndex;
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
