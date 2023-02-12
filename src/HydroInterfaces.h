/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Interfaces
*/

#ifndef HydroInterfaces_H
#define HydroInterfaces_H

struct HydroJSONSerializableInterface;

class HydroObjInterface;
class HydruinoUIInterface;
class HydroRTCInterface;

struct HydroDigitalInputPinInterface;
struct HydroDigitalOutputPinInterface;
struct HydroAnalogInputPinInterface;
struct HydroAnalogOutputPinInterface;

class HydroConcentrateUnitsInterface;
class HydroFlowRateUnitsInterface;
class HydroMeasureUnitsInterface;
class HydroPowerUnitsInterface;
class HydroTemperatureUnitsInterface;
class HydroVolumeUnitsInterface;

class HydroActuatorObjectInterface;
class HydroSensorObjectInterface;
class HydroCropObjectInterface;
class HydroReservoirObjectInterface;
class HydroRailObjectInterface;
class HydroBalancerObjectInterface;
class HydroTriggerObjectInterface;

class HydroPumpObjectInterface;

class HydroActuatorAttachmentInterface;
class HydroSensorAttachmentInterface;
class HydroCropAttachmentInterface;
class HydroReservoirAttachmentInterface;
class HydroRailAttachmentInterface;

class HydroFeedReservoirAttachmentInterface;

class HydroFlowSensorAttachmentInterface;
class HydroVolumeSensorAttachmentInterface;
class HydroPowerProductionSensorAttachmentInterface;
class HydroPowerUsageSensorAttachmentInterface;
class HydroWaterTemperatureSensorAttachmentInterface;
class HydroWaterPHSensorAttachmentInterface;
class HydroWaterTDSSensorAttachmentInterface;
class HydroSoilMoistureSensorAttachmentInterface;
class HydroAirTemperatureSensorAttachmentInterface;
class HydroAirCO2SensorAttachmentInterface;

class HydroFullTriggerAttachmentInterface;
class HydroEmptyTriggerAttachmentInterface;
class HydroFeedTriggerAttachmentInterface;
class HydroFedTriggerAttachmentInterface;
class HydroLimitTriggerAttachmentInterface;

#include "Hydruino.h"

// JSON Serializable Interface
struct HydroJSONSerializableInterface {
    // Given a JSON element to fill in, writes self to JSON format.
    virtual void toJSONObject(JsonObject &objectOut) const = 0;

    // Given a JSON element to read from, reads overtop self from JSON format.
    virtual void fromJSONObject(JsonObjectConst &objectIn) = 0;
};


// Object Interface
class HydroObjInterface {
public:
    virtual void unresolveAny(HydroObject *obj) = 0;

    virtual HydroIdentity getId() const = 0;
    virtual hkey_t getKey() const = 0;
    virtual String getKeyString() const = 0;
    virtual SharedPtr<HydroObjInterface> getSharedPtr() const = 0;

    virtual bool isObject() const = 0;
    inline bool isSubObject() const { return !isObject(); }
};

// UI Interface
class HydruinoUIInterface {
public:
    virtual void begin() = 0;

    virtual void setNeedsLayout() = 0;
};

// RTC Module Interface
class HydroRTCInterface {
public:
    virtual bool begin(TwoWire *wireInstance) = 0;
    virtual void adjust(const DateTime &dt) = 0;
    virtual bool lostPower(void) = 0;
    virtual DateTime now() = 0;
};


// Digital Input Pin Interface
struct HydroDigitalInputPinInterface {
    virtual ard_pinstatus_t digitalRead() = 0;
    inline int get() { return digitalRead(); }
};

// Digital Output Pin Interface
struct HydroDigitalOutputPinInterface {
    virtual void digitalWrite(ard_pinstatus_t status) = 0;
    inline void set(ard_pinstatus_t status) { digitalWrite(status); }
};

// Analog Input Pin Interface
struct HydroAnalogInputPinInterface {
    virtual float analogRead() = 0;
    virtual int analogRead_raw() = 0;
    inline float get() { return analogRead(); }
    inline int get_raw() { return analogRead_raw(); }
};

// Analog Output Pin Interface
struct HydroAnalogOutputPinInterface {
    virtual void analogWrite(float amount) = 0;
    virtual void analogWrite_raw(int amount) = 0;
    inline void set(float amount) { analogWrite(amount); }
    inline void set_raw(int amount) { analogWrite_raw(amount); }
};


// Concentrate Units Interface
class HydroConcentrateUnitsInterface {
public:
    virtual void setConcentrateUnits(Hydro_UnitsType concentrateUnits) = 0;
    inline Hydro_UnitsType getConcentrateUnits() const { return _concentrateUnits; }

protected:
    Hydro_UnitsType _concentrateUnits;
    inline HydroConcentrateUnitsInterface(Hydro_UnitsType concentrationUnits = defaultConcentrateUnits()) : _concentrateUnits(concentrationUnits) { ; }
};

// Flow Rate Units Interface
class HydroFlowRateUnitsInterface {
public:
    virtual void setFlowRateUnits(Hydro_UnitsType flowRateUnits) = 0;
    inline Hydro_UnitsType getFlowRateUnits() const { return _flowRateUnits; }
    inline Hydro_UnitsType getVolumeUnits() const { return baseUnits(_flowRateUnits); }

protected:
    Hydro_UnitsType _flowRateUnits;
    inline HydroFlowRateUnitsInterface(Hydro_UnitsType flowRateUnits = defaultFlowRateUnits()) : _flowRateUnits(flowRateUnits) { ; }
};

// Measure Units Interface
class HydroMeasureUnitsInterface {
public:
    virtual void setMeasureUnits(Hydro_UnitsType measureUnits) = 0;
    inline Hydro_UnitsType getMeasureUnits() const { return _measureUnits; }
    inline Hydro_UnitsType getRateUnits() const { rateUnits(_measureUnits); }
    inline Hydro_UnitsType getBaseUnits() const { baseUnits(_measureUnits); }

protected:
    Hydro_UnitsType _measureUnits;
    inline HydroMeasureUnitsInterface(Hydro_UnitsType measureUnits = Hydro_UnitsType_Undefined) : _measureUnits(measureUnits) { ; }
};

// Power Units Interface
class HydroPowerUnitsInterface {
public:
    virtual void setPowerUnits(Hydro_UnitsType powerUnits) = 0;
    inline Hydro_UnitsType getPowerUnits() const { return _powerUnits; }

protected:
    Hydro_UnitsType _powerUnits;
    inline HydroPowerUnitsInterface(Hydro_UnitsType powerUnits = defaultPowerUnits()) : _powerUnits(powerUnits) { ; }
};

// Temperature Units Interface
class HydroTemperatureUnitsInterface {
public:
    virtual void setTemperatureUnits(Hydro_UnitsType temperatureUnits) = 0;
    inline Hydro_UnitsType getTemperatureUnits() const { return _temperatureUnits; }

protected:
    Hydro_UnitsType _temperatureUnits;
    inline HydroTemperatureUnitsInterface(Hydro_UnitsType temperatureUnits = defaultTemperatureUnits()) : _temperatureUnits(temperatureUnits) { ; }
};

// Volume Units Interface
class HydroVolumeUnitsInterface {
public:
    virtual void setVolumeUnits(Hydro_UnitsType volumeUnits) = 0;
    inline Hydro_UnitsType getVolumeUnits() const { return _volumeUnits; }
    inline Hydro_UnitsType getFlowRateUnits() const { return rateUnits(_volumeUnits); }
    inline Hydro_UnitsType getUnits() const { return dilutionUnits(_volumeUnits); }

protected:
    Hydro_UnitsType _volumeUnits;
    inline HydroVolumeUnitsInterface(Hydro_UnitsType volumeUnits = defaultVolumeUnits()) : _volumeUnits(volumeUnits) { ; }
};


// Actuator Object Interface
class HydroActuatorObjectInterface {
public:
    virtual bool getCanEnable() = 0;
    virtual float getDriveIntensity() const = 0;
    virtual bool isEnabled(float tolerance = 0.0f) const = 0;

    virtual void setContinuousPowerUsage(HydroSingleMeasurement contPowerUsage) = 0;
    virtual const HydroSingleMeasurement &getContinuousPowerUsage() = 0;
    inline void setContinuousPowerUsage(float contPowerUsage, Hydro_UnitsType contPowerUsageUnits = Hydro_UnitsType_Undefined);

protected:
    virtual void _enableActuator(float intensity = 1.0) = 0;
    virtual void _disableActuator() = 0;
    virtual void handleActivation() = 0;
};

// Sensor Object Interface
class HydroSensorObjectInterface {
public:
    virtual bool takeMeasurement(bool force = false) = 0;
    virtual const HydroMeasurement *getLatestMeasurement() const = 0;
    virtual bool isTakingMeasurement() const = 0;
    virtual bool getNeedsPolling(hframe_t allowance = 0) const = 0;

protected:
    //virtual void handleMeasurement() = 0;
};

// Crop Object Interface
class HydroCropObjectInterface {
public:
    virtual bool getNeedsFeeding() = 0;
    virtual void notifyFeedingBegan() = 0;
    virtual void notifyFeedingEnded() = 0;
};

// Reservoir Object Interface
class HydroReservoirObjectInterface {
public:
    virtual bool canActivate(HydroActuator *actuator) = 0;
    virtual bool isFilled() = 0;
    virtual bool isEmpty() = 0;

    virtual HydroSensorAttachment &getWaterVolume(bool poll = false) = 0;
};

// Rail Object Interface
class HydroRailObjectInterface {
public:
    virtual bool canActivate(HydroActuator *actuator) = 0;
    virtual float getCapacity() = 0;

    virtual void setPowerUnits(Hydro_UnitsType powerUnits) = 0;
    virtual Hydro_UnitsType getPowerUnits() const = 0;

    virtual float getRailVoltage() const = 0;
};

// Balancer Object Interface
class HydroBalancerObjectInterface {
public:
    virtual void setTargetSetpoint(float targetSetpoint) = 0;
    virtual Hydro_BalancingState getBalancingState() const = 0;
    inline bool isBalanced() const;
};

// Trigger Object Interface
class HydroTriggerObjectInterface {
public:
    virtual Hydro_TriggerState getTriggerState() const = 0;
};


// Pump Object Interface
class HydroPumpObjectInterface {
public:
    virtual bool canPump(float volume, Hydro_UnitsType volumeUnits = Hydro_UnitsType_Undefined) = 0;
    virtual HydroActivationHandle pump(float volume, Hydro_UnitsType volumeUnits = Hydro_UnitsType_Undefined) = 0;
    virtual bool canPump(millis_t time) = 0;
    virtual HydroActivationHandle pump(millis_t time) = 0;

    virtual void setFlowRateUnits(Hydro_UnitsType flowRateUnits) = 0;
    virtual Hydro_UnitsType getFlowRateUnits() const = 0;

    virtual HydroAttachment &getParentReservoir(bool resolve = true) = 0;
    template<class U> inline void setInputReservoir(U reservoir);
    template<class U = HydroReservoir> inline SharedPtr<U> getInputReservoir(bool resolve = true);

    virtual HydroAttachment &getDestinationReservoir(bool resolve = true) = 0;
    template<class U> inline void setOutputReservoir(U reservoir);
    template<class U = HydroReservoir> inline SharedPtr<U> getOutputReservoir(bool resolve = true);

    virtual void setContinuousFlowRate(HydroSingleMeasurement contFlowRate) = 0;
    virtual const HydroSingleMeasurement &getContinuousFlowRate() = 0;
    inline void setContinuousFlowRate(float contFlowRate, Hydro_UnitsType contFlowRateUnits = Hydro_UnitsType_Undefined);

protected:
    virtual void pollPumpingSensors() = 0;
    virtual void handlePumpTime(millis_t time) = 0;
};


// Actuator Attachment Interface
class HydroActuatorAttachmentInterface {
public:
    virtual HydroAttachment &getParentActuator(bool resolve = true) = 0;

    template<class U> inline void setActuator(U actuator);
    template<class U = HydroActuator> inline SharedPtr<U> getActuator(bool resolve = true);
};

// Sensor Attachment Interface
class HydroSensorAttachmentInterface {
public:
    virtual HydroAttachment &getParentSensor(bool resolve = true) = 0;

    template<class U> inline void setSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getSensor(bool resolve = true);
};

// Crop Attachment Interface
class HydroCropAttachmentInterface {
public:
    virtual HydroAttachment &getParentCrop(bool resolve = true) = 0;

    template<class U> inline void setCrop(U crop);
    template<class U = HydroCrop> inline SharedPtr<U> getCrop(bool resolve = true);
};

// Reservoir Attachment Interface
class HydroReservoirAttachmentInterface {
public:
    virtual HydroAttachment &getParentReservoir(bool resolve = true) = 0;

    template<class U> inline void setReservoir(U reservoir);
    template<class U = HydroReservoir> inline SharedPtr<U> getReservoir(bool resolve = true);
};

// Rail Attachment Interface
class HydroRailAttachmentInterface {
public:
    virtual HydroAttachment &getParentRail(bool resolve = true) = 0;

    template<class U> inline void setRail(U rail);
    template<class U = HydroRail> inline SharedPtr<U> getRail(bool resolve = true);
};


// Feed Reservoir Attachment Interface
class HydroFeedReservoirAttachmentInterface {
public:
    virtual HydroAttachment &getFeedingReservoir(bool resolve = true) = 0;

    template<class U> inline void setFeedReservoir(U reservoir);
    template<class U = HydroFeedReservoir> inline SharedPtr<U> getFeedReservoir(bool resolve = true);
};


// Flow Rate Sensor Attachment Interface
class HydroFlowSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getFlowRate(bool poll = false) = 0;

    template<class U> inline void setFlowRateSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getFlowRateSensor(bool poll = false);
};

// Liquid Volume Sensor Attachment Interface
class HydroVolumeSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getWaterVolume(bool poll = false) = 0;

    template<class U> inline void setWaterVolumeSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getWaterVolumeSensor(bool poll = false);
};

// Power Production Sensor Attachment Interface
class HydroPowerProductionSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getPowerProduction(bool poll = false) = 0;

    template<class U> inline void setPowerProductionSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getPowerProductionSensor(bool poll = false);
};

// Power Usage Sensor Attachment Interface
class HydroPowerUsageSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getPowerUsage(bool poll = false) = 0;

    template<class U> inline void setPowerUsageSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getPowerUsageSensor(bool poll = false);
};

// Water Temperature Sensor Attachment Interface
class HydroWaterTemperatureSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getWaterTemperature(bool poll = false) = 0;

    template<class U> inline void setWaterTemperatureSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getWaterTemperatureSensor(bool poll = false);
};

// Water pH/Alkalinity Sensor Attachment Interface
class HydroWaterPHSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getWaterPH(bool poll = false) = 0;

    template<class U> inline void setWaterPHSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getWaterPHSensor(bool poll = false);
};

// Water TDS/Concentration Sensor Attachment Interface
class HydroWaterTDSSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getWaterTDS(bool poll = false) = 0;

    template<class U> inline void setWaterTDSSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getWaterTDSSensor(bool poll = false);
};

// Soil Moisture Sensor Attachment Interface
class HydroSoilMoistureSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getSoilMoisture(bool poll = false) = 0;

    template<class U> inline void setSoilMoistureSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getSoilMoistureSensor(bool poll = false);
};

// Air Temperature Sensor Attachment Interface
class HydroAirTemperatureSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getAirTemperature(bool poll = false) = 0;

    template<class U> inline void setAirTemperatureSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getAirTemperatureSensor(bool poll = false);
};

// Air CO2 Sensor Attachment Interface
class HydroAirCO2SensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getAirCO2(bool poll = false) = 0;

    template<class U> inline void setAirCO2Sensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getAirCO2Sensor(bool poll = false);
};


// Full Trigger Attachment Interface
class HydroFullTriggerAttachmentInterface {
public:
    // todo
};

// Empty Trigger Attachment Interface
class HydroEmptyTriggerAttachmentInterface {
public:
    // todo
};

// Feed Trigger Attachment Interface
class HydroFeedTriggerAttachmentInterface {
public:
    // todo
};

// Fed Trigger Attachment Interface
class HydroFedTriggerAttachmentInterface {
public:
    // todo
};

// Limit Trigger Attachment Interface
class HydroLimitTriggerAttachmentInterface {
public:
    // todo
};

#endif // /ifndef HydroInterfaces_H
