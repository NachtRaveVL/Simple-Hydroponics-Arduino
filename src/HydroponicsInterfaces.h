/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydroponics Interfaces
*/

#ifndef HydroponicsInterfaces_H
#define HydroponicsInterfaces_H

struct HydroponicsJSONSerializableInterface;

class HydroponicsObjInterface;
class HydroponicsUIInterface;

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

// Hydroponics UI Interface
class HydroponicsUIInterface {
public:
    virtual void begin() = 0;

    virtual void setNeedsLayout() = 0;
};


// Actuator Attachment Interface
class HydroponicsActuatorAttachmentInterface {
public:
    virtual HydroponicsAttachment &getParentActuator(bool resolve = true) = 0;

    template<class U> inline void setActuator(U actuator);
    template<class U = HydroponicsActuator> inline SharedPtr<U> getActuator(bool resolve = true);
};

// Sensor Attachment Interface
class HydroponicsSensorAttachmentInterface {
public:
    virtual HydroponicsAttachment &getParentSensor(bool resolve = true) = 0;

    template<class U> inline void setSensor(U sensor);
    template<class U = HydroponicsSensor> inline SharedPtr<U> getSensor(bool resolve = true);
};

// Crop Attachment Interface
class HydroponicsCropAttachmentInterface {
public:
    virtual HydroponicsAttachment &getParentCrop(bool resolve = true) = 0;

    template<class U> inline void setCrop(U crop);
    template<class U = HydroponicsCrop> inline SharedPtr<U> getCrop(bool resolve = true);
};

// Reservoir Attachment Interface
class HydroponicsReservoirAttachmentInterface {
public:
    virtual HydroponicsAttachment &getParentReservoir(bool resolve = true) = 0;

    template<class U> inline void setReservoir(U reservoir);
    template<class U = HydroponicsReservoir> inline SharedPtr<U> getReservoir(bool resolve = true);
};

// Rail Attachment Interface
class HydroponicsRailAttachmentInterface {
public:
    virtual HydroponicsAttachment &getParentRail(bool resolve = true) = 0;

    template<class U> inline void setRail(U rail);
    template<class U = HydroponicsRail> inline SharedPtr<U> getRail(bool resolve = true);
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
    inline bool isBalanced() const;
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
    template<class U> inline void setInputReservoir(U reservoir);
    template<class U = HydroponicsReservoir> inline SharedPtr<U> getInputReservoir(bool resolve = true);

    virtual HydroponicsAttachment &getDestinationReservoir(bool resolve = true) = 0;
    template<class U> inline void setOutputReservoir(U reservoir);
    template<class U = HydroponicsReservoir> inline SharedPtr<U> getOutputReservoir(bool resolve = true);

    virtual void setContinuousFlowRate(float contFlowRate, Hydroponics_UnitsType contFlowRateUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual void setContinuousFlowRate(HydroponicsSingleMeasurement contFlowRate) = 0;
    virtual const HydroponicsSingleMeasurement &getContinuousFlowRate() = 0;
};


// Flow Rate Aware Interface
class HydroponicsFeedReservoirAttachmentInterface {
public:
    virtual HydroponicsAttachment &getFeedingReservoir(bool resolve = true) = 0;

    template<class U> inline void setFeedReservoir(U reservoir);
    template<class U = HydroponicsFeedReservoir> inline SharedPtr<U> getFeedReservoir(bool resolve = true);
};

// Flow Rate Aware Interface
class HydroponicsFlowSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getFlowRate(bool poll = false) = 0;

    template<class U> inline void setFlowRateSensor(U sensor);
    template<class U = HydroponicsSensor> inline SharedPtr<U> getFlowRateSensor(bool poll = false);
};

// Liquid Volume Aware Interface
class HydroponicsVolumeSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getWaterVolume(bool poll = false) = 0;

    template<class U> inline void setWaterVolumeSensor(U sensor);
    template<class U = HydroponicsSensor> inline SharedPtr<U> getWaterVolumeSensor(bool poll = false);
};

// Power Aware Interface
class HydroponicsPowerSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getPowerUsage(bool poll = false) = 0;

    template<class U> inline void setPowerUsageSensor(U sensor);
    template<class U = HydroponicsSensor> inline SharedPtr<U> getPowerUsageSensor(bool poll = false);
};

// Water Temperature Aware Interface
class HydroponicsWaterTemperatureSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getWaterTemperature(bool poll = false) = 0;

    template<class U> inline void setWaterTemperatureSensor(U sensor);
    template<class U = HydroponicsSensor> inline SharedPtr<U> getWaterTemperatureSensor(bool poll = false);
};

// Water pH/Alkalinity Aware Interface
class HydroponicsWaterPHSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getWaterPH(bool poll = false) = 0;

    template<class U> inline void setWaterPHSensor(U sensor);
    template<class U = HydroponicsSensor> inline SharedPtr<U> getWaterPHSensor(bool poll = false);
};

// Water TDS/Concentration Aware Interface
class HydroponicsWaterTDSSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getWaterTDS(bool poll = false) = 0;

    template<class U> inline void setWaterTDSSensor(U sensor);
    template<class U = HydroponicsSensor> inline SharedPtr<U> getWaterTDSSensor(bool poll = false);
};

// Soil Moisture Aware Interface
class HydroponicsSoilMoistureSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getSoilMoisture(bool poll = false) = 0;

    template<class U> inline void setSoilMoistureSensor(U sensor);
    template<class U = HydroponicsSensor> inline SharedPtr<U> getSoilMoistureSensor(bool poll = false);
};

// Air Temperature Aware Interface
class HydroponicsAirTemperatureSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getAirTemperature(bool poll = false) = 0;

    template<class U> inline void setAirTemperatureSensor(U sensor);
    template<class U = HydroponicsSensor> inline SharedPtr<U> getAirTemperatureSensor(bool poll = false);
};

// Air Humidity Aware Interface
class HydroponicsAirHumiditySensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getAirHumidity(bool poll = false) = 0;

    template<class U> inline void setAirHumiditySensor(U sensor);
    template<class U = HydroponicsSensor> inline SharedPtr<U> getAirHumiditySensor(bool poll = false);
};

// Air CO2 Aware Interface
class HydroponicsAirCO2SensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getAirCO2(bool poll = false) = 0;

    template<class U> inline void setAirCO2Sensor(U sensor);
    template<class U = HydroponicsSensor> inline SharedPtr<U> getAirCO2Sensor(bool poll = false);
};

#endif // /ifndef HydroponicsInterfaces_H
