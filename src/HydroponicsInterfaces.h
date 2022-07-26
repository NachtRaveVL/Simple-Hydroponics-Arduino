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


// Actuator Attachment Interface
class HydroponicsActuatorAttachmentInterface {
public:
    virtual HydroponicsAttachment<HydroponicsActuator> &getParentActuator() = 0;

    template<class T> inline void setActuator(shared_ptr<T> actuator) { getParentActuator().setObject(actuator); }
    template<class T = HydroponicsActuator> inline shared_ptr<T> getActuator() { return static_pointer_cast<T>(getParentActuator().getObject()); }
};

// Sensor Attachment Interface
class HydroponicsSensorAttachmentInterface {
public:
    virtual HydroponicsAttachment<HydroponicsSensor> &getParentSensor() = 0;

    template<class T> inline void setSensor(shared_ptr<T> sensor) { getParentSensor().setObject(sensor); }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getSensor() { return static_pointer_cast<T>(getParentSensor().getObject()); }
};

// Crop Attachment Interface
class HydroponicsCropAttachmentInterface {
public:
    virtual HydroponicsAttachment<HydroponicsCrop> &getParentCrop() = 0;

    template<class T> inline void setCrop(shared_ptr<T> crop) { getParentCrop().setObject(crop); }
    template<class T = HydroponicsCrop> inline shared_ptr<T> getCrop() { return static_pointer_cast<T>(getParentCrop().getObject()); }
};

// Reservoir Attachment Interface
class HydroponicsReservoirAttachmentInterface {
public:
    virtual HydroponicsAttachment<HydroponicsReservoir> &getParentReservoir() = 0;

    template<class T> inline void setReservoir(shared_ptr<T> reservoir) { getParentReservoir().setObject(reservoir); }
    template<class T = HydroponicsReservoir> inline shared_ptr<T> getReservoir() { return static_pointer_cast<T>(getParentReservoir().getObject()); }
};

// Rail Attachment Interface
class HydroponicsRailAttachmentInterface {
public:
    virtual HydroponicsAttachment<HydroponicsRail> &getParentRail() = 0;

    template<class T> inline void setRail(shared_ptr<T> rail) { getParentRail().setObject(rail); }
    template<class T = HydroponicsRail> inline shared_ptr<T> getRail() { return static_pointer_cast<T>(getParentRail().getObject()); }
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
    virtual bool needsFeeding() const = 0;
    virtual void notifyFeedingBegan() = 0;
    virtual void notifyFeedingEnded() = 0;
};

// Reservoir Object Interface
class HydroponicsReservoirObjectInterface {
public:
    virtual bool canActivate(HydroponicsActuator *actuator) = 0;
    virtual bool isFilled() = 0;
    virtual bool isEmpty() = 0;

    virtual HydroponicsSensorAttachment &getWaterVolume() = 0;
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
};

// Trigger Object Interface
class HydroponicsTriggerObjectInterface {
public:
    virtual void attachTrigger() = 0;
    virtual void detachTrigger() = 0;
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

    virtual HydroponicsAttachment<HydroponicsReservoir> &getParentReservoir() = 0;
    template<class T> inline void setInputReservoir(shared_ptr<T> reservoir) { getParentReservoir().setObject(reservoir); }
    template<class T = HydroponicsReservoir> inline shared_ptr<T> getInputReservoir() { return static_pointer_cast<T>(getParentReservoir().getObject()); }

    virtual HydroponicsAttachment<HydroponicsReservoir> &getDestinationReservoir() = 0;
    template<class T> inline void setOutputReservoir(shared_ptr<T> reservoir) { getDestinationReservoir().setObject(reservoir); }
    template<class T = HydroponicsReservoir> inline shared_ptr<T> getOutputReservoir() { return static_pointer_cast<T>(getDestinationReservoir().getObject()); }

    virtual void setContinuousFlowRate(float contFlowRate, Hydroponics_UnitsType contFlowRateUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual void setContinuousFlowRate(HydroponicsSingleMeasurement contFlowRate) = 0;
    virtual const HydroponicsSingleMeasurement &getContinuousFlowRate() = 0;
};


// Flow Rate Aware Interface
class HydroponicsFeedReservoirAttachmentInterface {
public:
    virtual HydroponicsAttachment<HydroponicsFeedReservoir> &getFeedingReservoir() = 0;

    template<class T> inline void setFeedReservoir(shared_ptr<T> reservoir) { getFeedingReservoir().setObject(reservoir); }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getFeedReservoir() { return static_pointer_cast<T>(getFeedingReservoir().getObject()); }
};

// Flow Rate Aware Interface
class HydroponicsFlowSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getFlowRate() = 0;

    template<class T> inline void setFlowRateSensor(shared_ptr<T> sensor) { getFlowRate().setObject(sensor); }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getFlowRateSensor() { return static_pointer_cast<T>(getFlowRate().getObject()); }
};

// Liquid Volume Aware Interface
class HydroponicsVolumeSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getWaterVolume() = 0;

    template<class T> inline void setWaterVolumeSensor(shared_ptr<T> sensor) { getWaterVolume().setObject(sensor); }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getWaterVolumeSensor() { return static_pointer_cast<T>(getWaterVolume().getObject()); }
};

// Power Aware Interface
class HydroponicsPowerSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getPowerUsage() = 0;

    template<class T> inline void setPowerUsageSensor(shared_ptr<T> sensor) { getPowerUsage().setObject(sensor); }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getPowerUsageSensor() { return static_pointer_cast<T>(getPowerUsage().getObject()); }
};

// Water Temperature Aware Interface
class HydroponicsWaterTemperatureSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getWaterTemperature() = 0;

    template<class T> inline void setWaterTemperatureSensor(shared_ptr<T> sensor) { getWaterTemperature().setObject(sensor); }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getWaterTemperatureSensor() { return static_pointer_cast<T>(getWaterTemperature().getObject()); }
};

// Water pH/Alkalinity Aware Interface
class HydroponicsWaterPHSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getWaterPH() = 0;

    template<class T> inline void setWaterPHSensor(shared_ptr<T> sensor) { getWaterPH().setObject(sensor); }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getWaterPHSensor() { return static_pointer_cast<T>(getWaterPH().getObject()); }
};

// Water TDS/Concentration Aware Interface
class HydroponicsWaterTDSSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getWaterTDS() = 0;

    template<class T> inline void setWaterTDSSensor(shared_ptr<T> sensor) { getWaterTDS().setObject(sensor); }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getWaterTDSSensor() { return static_pointer_cast<T>(getWaterTDS().getObject()); }
};

// Soil Moisture Aware Interface
class HydroponicsSoilMoistureSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getSoilMoisture() = 0;

    template<class T> inline void setSoilMoistureSensor(shared_ptr<T> sensor) { getSoilMoisture().setObject(sensor); }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getSoilMoistureSensor() { return static_pointer_cast<T>(getSoilMoisture().getObject()); }
};

// Air Temperature Aware Interface
class HydroponicsAirTemperatureSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getAirTemperature() = 0;

    template<class T> inline void setAirTemperatureSensor(shared_ptr<T> sensor) { getAirTemperature().setObject(sensor); }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getAirTemperatureSensor() { return static_pointer_cast<T>(getAirTemperature().getObject()); }
};

// Air Humidity Aware Interface
class HydroponicsAirHumiditySensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getAirHumidity() = 0;

    template<class T> inline void setAirHumiditySensor(shared_ptr<T> sensor) { getAirHumidity().setObject(sensor); }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getAirHumiditySensor() { return static_pointer_cast<T>(getAirHumidity().getObject()); }
};

// Air CO2 Aware Interface
class HydroponicsAirCO2SensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getAirCO2() = 0;

    template<class T> inline void setAirCO2Sensor(shared_ptr<T> sensor) { getAirCO2().setObject(sensor); }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getAirCO2Sensor() { return static_pointer_cast<T>(getAirCO2().getObject()); }
};

#endif // /ifndef HydroponicsInterfaces_H
