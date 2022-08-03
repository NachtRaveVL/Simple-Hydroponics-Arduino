/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Interfaces
*/

#ifndef HydroponicsInterfaces_H
#define HydroponicsInterfaces_H

struct HydroponicsJSONSerializableInterface;

class HydroponicsActuatorAttachmentInterface;
class HydroponicsSensorAttachmentInterface;
class HydroponicsCropAttachmentInterface;
class HydroponicsRailAttachmentInterface;
class HydroponicsReservoirAttachmentInterface;

class HydroponicsActuatorObjectInterface;
class HydroponicsSensorObjectInterface;
class HydroponicsCropObjectInterface;
class HydroponicsRailObjectInterface;
class HydroponicsReservoirObjectInterface;

class HydroponicsTriggerObjectInterface;
class HydroponicsPumpObjectInterface;

class HydroponicsFeedReservoirAttachmentInterface;
class HydroponicsFlowSensorAttachmentInterface;
class HydroponicsVolumeSensorAttachmentInterface;
class HydroponicsPowerSensorAttachmentInterface;
class HydroponicsWaterTemperatureSensorAttachmentInterface;
class HydroponicsWaterPHSensorAttachmentInterface;
class HydroponicsWaterTDSSensorAttachmentInterface;
class HydroponicsSoilMoistureSensorAttachmentInterface;
class HydroponicsSoilTemperatureSensorAttachmentInterface;
class HydroponicsAirTemperatureSensorAttachmentInterface;
class HydroponicsAirHumiditySensorAttachmentInterface;
class HydroponicsAirCO2SensorAttachmentInterface;

#include "Hydroponics.h"

// JSON Serializable Interface
struct HydroponicsJSONSerializableInterface {
    // Given a JSON element to fill in, writes self to JSON format.
    virtual void toJSONObject(JsonObject &objectOut) const = 0;

    // Given a JSON element to read from, reads overtop self from JSON format.
    virtual void fromJSONObject(JsonObjectConst &objectIn) = 0;
};


// Hydroponics Object Interface
class HydroponicsObjInterface {
public:
    virtual HydroponicsIdentity getId() const = 0;
    virtual Hydroponics_KeyType getKey() const = 0;
    virtual String getKeyString() const = 0;
    virtual SharedPtr<HydroponicsObjInterface> getSharedPtr() const = 0;

    virtual bool addLinkage(HydroponicsObject *obj) = 0;
    virtual bool removeLinkage(HydroponicsObject *obj) = 0;
};


// Actuator Attachment Interface
class HydroponicsActuatorAttachmentInterface {
public:
    virtual HydroponicsAttachment &getParentActuator(bool resolve = true) = 0;

    template<class U> inline void setActuator(U actuator) { getParentActuator(false).setObject(actuator); }
    template<class U = HydroponicsActuator> inline SharedPtr<U> getActuator(bool resolve = true) { return getParentActuator(resolve).template getObject<U>(); }
};

// Sensor Attachment Interface
class HydroponicsSensorAttachmentInterface {
public:
    virtual HydroponicsAttachment &getParentSensor(bool resolve = true) = 0;

    template<class U> inline void setSensor(U sensor) { getParentSensor(false).setObject(sensor); }
    template<class U = HydroponicsSensor> inline SharedPtr<U> getSensor(bool resolve = true) { return getParentSensor(resolve).template getObject<U>(); }
};

// Crop Attachment Interface
class HydroponicsCropAttachmentInterface {
public:
    virtual HydroponicsAttachment &getParentCrop(bool resolve = true) = 0;

    template<class U> inline void setCrop(U crop) { getParentCrop(false).setObject(crop); }
    template<class U = HydroponicsCrop> inline SharedPtr<U> getCrop(bool resolve = true) { return getParentCrop(resolve).template getObject<U>(); }
};

// Reservoir Attachment Interface
class HydroponicsReservoirAttachmentInterface {
public:
    virtual HydroponicsAttachment &getParentReservoir(bool resolve = true) = 0;

    template<class U> inline void setReservoir(U reservoir) { getParentReservoir(false).setObject(reservoir); }
    template<class U = HydroponicsReservoir> inline SharedPtr<U> getReservoir(bool resolve = true) { return getParentReservoir(resolve).template getObject<U>(); }
};

// Rail Attachment Interface
class HydroponicsRailAttachmentInterface {
public:
    virtual HydroponicsAttachment &getParentRail(bool resolve = true) = 0;

    template<class U> inline void setRail(U rail) { getParentRail(false).setObject(rail); }
    template<class U = HydroponicsRail> inline SharedPtr<U> getRail(bool resolve = true) { return getParentRail(resolve).template getObject<U>(); }
};


// Actuator Object Interface
class HydroponicsActuatorObjectInterface {
public:
    virtual bool enableActuator(float intensity = 1.0f, bool force = false) = 0;
    virtual void disableActuator() = 0;
    virtual bool getCanEnable() = 0;
    virtual bool isEnabled(float tolerance = 0.0f) const = 0;

    virtual void setContinuousPowerUsage(float contPowerUsage, Hydroponics_UnitsType contPowerUsageUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual void setContinuousPowerUsage(HydroponicsSingleMeasurement contPowerUsage) = 0;
    virtual const HydroponicsSingleMeasurement &getContinuousPowerUsage() = 0;
};

// Sensor Object Interface
class HydroponicsSensorObjectInterface {
public:
    virtual bool takeMeasurement(bool force = false) = 0;
    virtual const HydroponicsMeasurement *getLatestMeasurement() const = 0;
    virtual bool isTakingMeasurement() const = 0;
    virtual bool needsPolling(uint32_t allowance = 0) const = 0;
};

// Crop Object Interface
class HydroponicsCropObjectInterface {
public:
    virtual bool needsFeeding() = 0;
    virtual void notifyFeedingBegan() = 0;
    virtual void notifyFeedingEnded() = 0;
};

// Reservoir Object Interface
class HydroponicsReservoirObjectInterface {
public:
    virtual bool canActivate(HydroponicsActuator *actuator) = 0;
    virtual bool isFilled() = 0;
    virtual bool isEmpty() = 0;

    virtual HydroponicsSensorAttachment &getWaterVolume(bool poll = false) = 0;
};

// Rail Object Interface
class HydroponicsRailObjectInterface {
public:
    virtual bool canActivate(HydroponicsActuator *actuator) = 0;
    virtual float getCapacity() = 0;

    virtual void setPowerUnits(Hydroponics_UnitsType powerUnits) = 0;
    virtual Hydroponics_UnitsType getPowerUnits() const = 0;

    virtual float getRailVoltage() const = 0;
};


// Balancer Object Interface
class HydroponicsBalancerObjectInterface {
public:
    virtual void setTargetSetpoint(float targetSetpoint) = 0;
    virtual Hydroponics_BalancerState getBalancerState() const = 0;
    inline bool isBalanced() const { return getBalancerState() == Hydroponics_BalancerState_Balanced; }
};

// Trigger Object Interface
class HydroponicsTriggerObjectInterface {
public:
    virtual Hydroponics_TriggerState getTriggerState() const = 0;
};

// Pump Object Interface
class HydroponicsPumpObjectInterface {
public:
    virtual bool canPump(float volume, Hydroponics_UnitsType volumeUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual bool pump(float volume, Hydroponics_UnitsType volumeUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual bool canPump(time_t timeMillis) = 0;
    virtual bool pump(time_t timeMillis) = 0;

    virtual void setFlowRateUnits(Hydroponics_UnitsType flowRateUnits) = 0;
    virtual Hydroponics_UnitsType getFlowRateUnits() const = 0;

    virtual HydroponicsAttachment &getParentReservoir(bool resolve = true) = 0;
    template<class U> inline void setInputReservoir(U reservoir) { getParentReservoir(false).setObject(reservoir); }
    template<class U = HydroponicsReservoir> inline SharedPtr<U> getInputReservoir(bool resolve = true) { return getParentReservoir(resolve).template getObject<U>(); }

    virtual HydroponicsAttachment &getDestinationReservoir(bool resolve = true) = 0;
    template<class U> inline void setOutputReservoir(U reservoir) { getDestinationReservoir(false).setObject(reservoir); }
    template<class U = HydroponicsReservoir> inline SharedPtr<U> getOutputReservoir(bool resolve = true) { return getDestinationReservoir(resolve).template getObject<U>(); }

    virtual void setContinuousFlowRate(float contFlowRate, Hydroponics_UnitsType contFlowRateUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual void setContinuousFlowRate(HydroponicsSingleMeasurement contFlowRate) = 0;
    virtual const HydroponicsSingleMeasurement &getContinuousFlowRate() = 0;
};


// Flow Rate Aware Interface
class HydroponicsFeedReservoirAttachmentInterface {
public:
    virtual HydroponicsAttachment &getFeedingReservoir(bool resolve = true) = 0;

    template<class U> inline void setFeedReservoir(U reservoir) { getFeedingReservoir(false).setObject(reservoir); }
    template<class U = HydroponicsFeedReservoir> inline SharedPtr<U> getFeedReservoir(bool resolve = true) { return getFeedingReservoir(resolve).template getObject<U>(); }
};

// Flow Rate Aware Interface
class HydroponicsFlowSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getFlowRate(bool poll = false) = 0;

    template<class U> inline void setFlowRateSensor(U sensor) { getFlowRate(false).setObject(sensor); }
    template<class U = HydroponicsSensor> inline SharedPtr<U> getFlowRateSensor(bool poll = false) { return static_pointer_cast<U>(getFlowRate(poll).getObject()); }
};

// Liquid Volume Aware Interface
class HydroponicsVolumeSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getWaterVolume(bool poll = false) = 0;

    template<class U> inline void setWaterVolumeSensor(U sensor) { getWaterVolume(false).setObject(sensor); }
    template<class U = HydroponicsSensor> inline SharedPtr<U> getWaterVolumeSensor(bool poll = false) { return static_pointer_cast<U>(getWaterVolume(poll).getObject()); }
};

// Power Aware Interface
class HydroponicsPowerSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getPowerUsage(bool poll = false) = 0;

    template<class U> inline void setPowerUsageSensor(U sensor) { getPowerUsage(false).setObject(sensor); }
    template<class U = HydroponicsSensor> inline SharedPtr<U> getPowerUsageSensor(bool poll = false) { return static_pointer_cast<U>(getPowerUsage(poll).getObject()); }
};

// Water Temperature Aware Interface
class HydroponicsWaterTemperatureSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getWaterTemperature(bool poll = false) = 0;

    template<class U> inline void setWaterTemperatureSensor(U sensor) { getWaterTemperature(false).setObject(sensor); }
    template<class U = HydroponicsSensor> inline SharedPtr<U> getWaterTemperatureSensor(bool poll = false) { return static_pointer_cast<U>(getWaterTemperature(poll).getObject()); }
};

// Water pH/Alkalinity Aware Interface
class HydroponicsWaterPHSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getWaterPH(bool poll = false) = 0;

    template<class U> inline void setWaterPHSensor(U sensor) { getWaterPH(false).setObject(sensor); }
    template<class U = HydroponicsSensor> inline SharedPtr<U> getWaterPHSensor(bool poll = false) { return static_pointer_cast<U>(getWaterPH(poll).getObject()); }
};

// Water TDS/Concentration Aware Interface
class HydroponicsWaterTDSSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getWaterTDS(bool poll = false) = 0;

    template<class U> inline void setWaterTDSSensor(U sensor) { getWaterTDS(false).setObject(sensor); }
    template<class U = HydroponicsSensor> inline SharedPtr<U> getWaterTDSSensor(bool poll = false) { return static_pointer_cast<U>(getWaterTDS(poll).getObject()); }
};

// Soil Moisture Aware Interface
class HydroponicsSoilMoistureSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getSoilMoisture(bool poll = false) = 0;

    template<class U> inline void setSoilMoistureSensor(U sensor) { getSoilMoisture(false).setObject(sensor); }
    template<class U = HydroponicsSensor> inline SharedPtr<U> getSoilMoistureSensor(bool poll = false) { return static_pointer_cast<U>(getSoilMoisture(poll).getObject()); }
};

// Air Temperature Aware Interface
class HydroponicsAirTemperatureSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getAirTemperature(bool poll = false) = 0;

    template<class U> inline void setAirTemperatureSensor(U sensor) { getAirTemperature(false).setObject(sensor); }
    template<class U = HydroponicsSensor> inline SharedPtr<U> getAirTemperatureSensor(bool poll = false) { return static_pointer_cast<U>(getAirTemperature(poll).getObject()); }
};

// Air Humidity Aware Interface
class HydroponicsAirHumiditySensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getAirHumidity(bool poll = false) = 0;

    template<class U> inline void setAirHumiditySensor(U sensor) { getAirHumidity(false).setObject(sensor); }
    template<class U = HydroponicsSensor> inline SharedPtr<U> getAirHumiditySensor(bool poll = false) { return static_pointer_cast<U>(getAirHumidity(poll).getObject()); }
};

// Air CO2 Aware Interface
class HydroponicsAirCO2SensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getAirCO2(bool poll = false) = 0;

    template<class U> inline void setAirCO2Sensor(U sensor) { getAirCO2(false).setObject(sensor); }
    template<class U = HydroponicsSensor> inline SharedPtr<U> getAirCO2Sensor(bool poll = false) { return static_pointer_cast<U>(getAirCO2(poll).getObject()); }
};

#endif // /ifndef HydroponicsInterfaces_H
