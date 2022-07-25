/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Interfaces
*/

#ifndef HydroponicsInterfaces_H
#define HydroponicsInterfaces_H

struct HydroponicsJSONSerializableInterface;

class HydroponicsActuatorAttachmentsInterface;
class HydroponicsSensorAttachmentsInterface;
class HydroponicsCropAttachmentsInterface;
class HydroponicsRailAttachmentsInterface;
class HydroponicsReservoirAttachmentsInterface;

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


// Actuator Attachments Interface
class HydroponicsActuatorAttachmentsInterface {
public:
    virtual bool addActuator(HydroponicsActuator *actuator) = 0;
    virtual bool removeActuator(HydroponicsActuator *actuator) = 0;
    virtual bool hasActuator(HydroponicsActuator *actuator) const = 0;
};

// Sensor Attachments Interface
class HydroponicsSensorAttachmentsInterface {
public:
    virtual bool addSensor(HydroponicsSensor *sensor) = 0;
    virtual bool removeSensor(HydroponicsSensor *sensor) = 0;
    virtual bool hasSensor(HydroponicsSensor *sensor) const = 0;
};

// Crop Attachments Interface
class HydroponicsCropAttachmentsInterface {
public:
    virtual bool addCrop(HydroponicsCrop *crop) = 0;
    virtual bool removeCrop(HydroponicsCrop *crop) = 0;
    virtual bool hasCrop(HydroponicsCrop *crop) const = 0;
};

// Reservoir Attachments Interface
class HydroponicsReservoirAttachmentsInterface {
public:
    virtual bool addReservoir(HydroponicsReservoir *reservoir) = 0;
    virtual bool removeReservoir(HydroponicsReservoir *reservoir) = 0;
    virtual bool hasReservoir(HydroponicsReservoir *reservoir) const = 0;
};

// Rail Attachments Interface
class HydroponicsRailAttachmentsInterface {
public:
    virtual bool addRail(HydroponicsRail *rail) = 0;
    virtual bool removeRail(HydroponicsRail *rail) = 0;
    virtual bool hasRail(HydroponicsRail *rail) const = 0;
};


// Actuator Attachment Interface
class HydroponicsActuatorAttachmentInterface {
public:
    virtual void setActuator(HydroponicsIdentity actuatorId) = 0;
    virtual void setActuator(shared_ptr<HydroponicsActuator> actuator) = 0;
    virtual shared_ptr<HydroponicsActuator> getActuator() = 0;
};

// Sensor Attachment Interface
class HydroponicsSensorAttachmentInterface {
public:
    virtual void setSensor(HydroponicsIdentity sensorId) = 0;
    virtual void setSensor(shared_ptr<HydroponicsSensor> sensor) = 0;
    virtual shared_ptr<HydroponicsSensor> getSensor() = 0;
};

// Crop Attachment Interface
class HydroponicsCropAttachmentInterface {
public:
    virtual void setCrop(HydroponicsIdentity cropId) = 0;
    virtual void setCrop(shared_ptr<HydroponicsCrop> crop) = 0;
    virtual shared_ptr<HydroponicsCrop> getCrop() = 0;
};

// Reservoir Attachment Interface
class HydroponicsReservoirAttachmentInterface {
public:
    virtual void setReservoir(HydroponicsIdentity reservoirId) = 0;
    virtual void setReservoir(shared_ptr<HydroponicsReservoir> reservoir) = 0;
    virtual shared_ptr<HydroponicsReservoir> getReservoir() = 0;
};

// Rail Attachment Interface
class HydroponicsRailAttachmentInterface {
public:
    virtual void setRail(HydroponicsIdentity powerRailId) = 0;
    virtual void setRail(shared_ptr<HydroponicsRail> powerRail) = 0;
    virtual shared_ptr<HydroponicsRail> getRail() = 0;
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
class HydroponicsPumpObjectInterface : public HydroponicsReservoirAttachmentInterface {
public:
    virtual bool canPump(float volume, Hydroponics_UnitsType volumeUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual bool pump(float volume, Hydroponics_UnitsType volumeUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual bool canPump(time_t timeMillis) = 0;
    virtual bool pump(time_t timeMillis) = 0;

    virtual void setFlowRateUnits(Hydroponics_UnitsType flowRateUnits) = 0;
    virtual Hydroponics_UnitsType getFlowRateUnits() const = 0;

    virtual void setOutputReservoir(HydroponicsIdentity destReservoirId) = 0;
    virtual void setOutputReservoir(shared_ptr<HydroponicsReservoir> destReservoir) = 0;
    virtual shared_ptr<HydroponicsReservoir> getOutputReservoir() = 0;

    virtual void setContinuousFlowRate(float contFlowRate, Hydroponics_UnitsType contFlowRateUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual void setContinuousFlowRate(HydroponicsSingleMeasurement contFlowRate) = 0;
    virtual const HydroponicsSingleMeasurement &getContinuousFlowRate() = 0;
};


// Flow Rate Aware Interface
class HydroponicsFlowSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getFlowRate() = 0;

    template<class T> inline void setFlowRateSensor(shared_ptr<T> sensor) { getFlowRate() = sensor; }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getFlowRateSensor() { static_pointer_cast<T>(getFlowRate().getObject()); }
};

// Liquid Volume Aware Interface
class HydroponicsVolumeSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getWaterVolume() = 0;

    template<class T> inline void setWaterVolumeSensor(shared_ptr<T> sensor) { getWaterVolume() = sensor; }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getWaterVolumeSensor() { static_pointer_cast<T>(getWaterVolume().getObject()); }
};

// Power Aware Interface
class HydroponicsPowerSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getPowerUsage() = 0;

    template<class T> inline void setPowerUsageSensor(shared_ptr<T> sensor) { getPowerUsage() = sensor; }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getPowerUsageSensor() { static_pointer_cast<T>(getPowerUsage().getObject()); }
};

// Water Temperature Aware Interface
class HydroponicsWaterTemperatureSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getWaterTemperature() = 0;

    template<class T> inline void setWaterTemperatureSensor(shared_ptr<T> sensor) { getWaterTemperature() = sensor; }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getWaterTemperatureSensor() { static_pointer_cast<T>(getWaterTemperature().getObject()); }
};

// Water pH/Alkalinity Aware Interface
class HydroponicsWaterPHSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getWaterPH() = 0;

    template<class T> inline void setWaterPHSensor(shared_ptr<T> sensor) { getWaterPH() = sensor; }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getWaterPHSensor() { static_pointer_cast<T>(getWaterPH().getObject()); }
};

// Water TDS/Concentration Aware Interface
class HydroponicsWaterTDSSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getWaterTDS() = 0;

    template<class T> inline void setWaterTDSSensor(shared_ptr<T> sensor) { getWaterTDS() = sensor; }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getWaterTDSSensor() { static_pointer_cast<T>(getWaterTDS().getObject()); }
};

// Soil Moisture Aware Interface
class HydroponicsSoilMoistureSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getSoilMoisture() = 0;

    template<class T> inline void setSoilMoistureSensor(shared_ptr<T> sensor) { getSoilMoisture() = sensor; }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getSoilMoistureSensor() { static_pointer_cast<T>(getSoilMoisture().getObject()); }
};

// Air Temperature Aware Interface
class HydroponicsAirTemperatureSensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getAirTemperature() = 0;

    template<class T> inline void setAirTemperatureSensor(shared_ptr<T> sensor) { getAirTemperature() = sensor; }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getAirTemperatureSensor() { static_pointer_cast<T>(getAirTemperature().getObject()); }
};

// Air Humidity Aware Interface
class HydroponicsAirHumiditySensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getAirHumidity() = 0;

    template<class T> inline void setAirHumiditySensor(shared_ptr<T> sensor) { getAirHumidity() = sensor; }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getAirHumiditySensor() { static_pointer_cast<T>(getAirHumidity().getObject()); }
};

// Air CO2 Aware Interface
class HydroponicsAirCO2SensorAttachmentInterface {
public:
    virtual HydroponicsSensorAttachment &getAirCO2() = 0;

    template<class T> inline void setAirCO2Sensor(shared_ptr<T> sensor) { getAirCO2() = sensor; }
    template<class T = HydroponicsSensor> inline shared_ptr<T> getAirCO2Sensor() { static_pointer_cast<T>(getAirCO2().getObject()); }
};

#endif // /ifndef HydroponicsInterfaces_H
