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

class HydroponicsFlowAwareInterface;
class HydroponicsVolumeAwareInterface;
class HydroponicsPowerAwareInterface;
class HydroponicsWaterTemperatureAwareInterface;
class HydroponicsWaterPHAwareInterface;
class HydroponicsWaterTDSAwareInterface;
class HydroponicsSoilMoistureAwareInterface;
class HydroponicsSoilTemperatureAwareInterface;
class HydroponicsAirTemperatureAwareInterface;
class HydroponicsAirHumidityAwareInterface;
class HydroponicsAirCO2AwareInterface;

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

    virtual void setContinuousPowerDraw(float contPowerDraw, Hydroponics_UnitsType contPowerDrawUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual void setContinuousPowerDraw(HydroponicsSingleMeasurement contPowerDraw) = 0;
    virtual const HydroponicsSingleMeasurement &getContinuousPowerDraw() = 0;
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
    virtual bool isFilled() const = 0;
    virtual bool isEmpty() const = 0;

    virtual void setVolumeUnits(Hydroponics_UnitsType volumeUnits) = 0;
    virtual Hydroponics_UnitsType getVolumeUnits() const = 0;

    virtual void setWaterVolume(float waterVolume, Hydroponics_UnitsType waterVolumeUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual void setWaterVolume(HydroponicsSingleMeasurement waterVolume) = 0;
    virtual const HydroponicsSingleMeasurement &getWaterVolume() = 0;
};

// Rail Object Interface
class HydroponicsRailObjectInterface {
public:
    virtual bool canActivate(HydroponicsActuator *actuator) = 0;
    virtual float getCapacity() const = 0;

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
class HydroponicsFlowAwareInterface {
public:
    virtual void setFlowRateSensor(HydroponicsIdentity flowRateSensorId) = 0;
    virtual void setFlowRateSensor(shared_ptr<HydroponicsSensor> flowRateSensor) = 0;
    virtual shared_ptr<HydroponicsSensor> getFlowRateSensor() = 0;

    virtual void setFlowRate(float flowRate, Hydroponics_UnitsType flowRateUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual void setFlowRate(HydroponicsSingleMeasurement flowRate) = 0;
    virtual const HydroponicsSingleMeasurement &getFlowRate() = 0;    
};

// Liquid Volume Aware Interface
class HydroponicsVolumeAwareInterface {
public:
    virtual void setVolumeSensor(HydroponicsIdentity volumeSensorId) = 0;
    virtual void setVolumeSensor(shared_ptr<HydroponicsSensor> volumeSensor) = 0;
    virtual shared_ptr<HydroponicsSensor> getVolumeSensor() = 0;

    virtual void setWaterVolume(float waterVolume, Hydroponics_UnitsType waterVolumeUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual void setWaterVolume(HydroponicsSingleMeasurement waterVolume) = 0;
    virtual const HydroponicsSingleMeasurement &getWaterVolume() = 0;
};

// Power Aware Interface
class HydroponicsPowerAwareInterface {
public:
    virtual void setPowerSensor(HydroponicsIdentity powerSensorId) = 0;
    virtual void setPowerSensor(shared_ptr<HydroponicsSensor> powerSensor) = 0;
    virtual shared_ptr<HydroponicsSensor> getPowerSensor() = 0;

    virtual void setPowerDraw(float powerDraw, Hydroponics_UnitsType powerDrawUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual void setPowerDraw(HydroponicsSingleMeasurement powerDraw) = 0;
    virtual const HydroponicsSingleMeasurement &getPowerDraw() = 0;
};

// Water Temperature Aware Interface
class HydroponicsWaterTemperatureAwareInterface {
public:
    virtual void setWaterTempSensor(HydroponicsIdentity waterTempSensorId) = 0;
    virtual void setWaterTempSensor(shared_ptr<HydroponicsSensor> waterTempSensor) = 0;
    virtual shared_ptr<HydroponicsSensor> getWaterTempSensor() = 0;

    virtual void setWaterTemperature(float waterTemperature, Hydroponics_UnitsType waterTempUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual void setWaterTemperature(HydroponicsSingleMeasurement waterTemperature) = 0;
    virtual const HydroponicsSingleMeasurement &getWaterTemperature() = 0;
};

// Water pH/Alkalinity Aware Interface
class HydroponicsWaterPHAwareInterface {
public:
    virtual void setWaterPHSensor(HydroponicsIdentity waterPHSensorId) = 0;
    virtual void setWaterPHSensor(shared_ptr<HydroponicsSensor> waterPHSensor) = 0;
    virtual shared_ptr<HydroponicsSensor> getWaterPHSensor() = 0;

    virtual void setWaterPH(float waterPH, Hydroponics_UnitsType waterPHUnits = Hydroponics_UnitsType_Alkalinity_pH_0_14) = 0;
    virtual void setWaterPH(HydroponicsSingleMeasurement waterPH) = 0;
    virtual const HydroponicsSingleMeasurement &getWaterPH() = 0;
};

// Water TDS/Concentration Aware Interface
class HydroponicsWaterTDSAwareInterface {
public:
    virtual void setWaterTDSSensor(HydroponicsIdentity waterTDSSensorId) = 0;
    virtual void setWaterTDSSensor(shared_ptr<HydroponicsSensor> waterTDSSensor) = 0;
    virtual shared_ptr<HydroponicsSensor> getWaterTDSSensor() = 0;

    virtual void setWaterTDS(float waterTDS, Hydroponics_UnitsType waterTDSUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual void setWaterTDS(HydroponicsSingleMeasurement waterTDS) = 0;
    virtual const HydroponicsSingleMeasurement &getWaterTDS() = 0;
};

// Soil Moisture Aware Interface
class HydroponicsSoilMoistureAwareInterface {
public:
    virtual void setMoistureSensor(HydroponicsIdentity moistureSensorId) = 0;
    virtual void setMoistureSensor(shared_ptr<HydroponicsSensor> moistureSensor) = 0;
    virtual shared_ptr<HydroponicsSensor> getMoistureSensor() = 0;

    virtual void setSoilMoisture(float soilMoisture, Hydroponics_UnitsType soilMoistureUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual void setSoilMoisture(HydroponicsSingleMeasurement soilMoisture) = 0;
    virtual const HydroponicsSingleMeasurement &getSoilMoisture() = 0;
};

// Air Temperature Aware Interface
class HydroponicsAirTemperatureAwareInterface {
public:
    virtual void setAirTempSensor(HydroponicsIdentity airTempSensorId) = 0;
    virtual void setAirTempSensor(shared_ptr<HydroponicsSensor> airTempSensor) = 0;
    virtual shared_ptr<HydroponicsSensor> getAirTempSensor() = 0;

    virtual void setAirTemperature(float airTemperature, Hydroponics_UnitsType airTempUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual void setAirTemperature(HydroponicsSingleMeasurement airTemperature) = 0;
    virtual const HydroponicsSingleMeasurement &getAirTemperature() = 0;
};

// Air Humidity Aware Interface
class HydroponicsAirHumidityAwareInterface {
public:
    virtual void setHumiditySensor(HydroponicsIdentity airHumiditySensorId) = 0;
    virtual void setHumiditySensor(shared_ptr<HydroponicsSensor> airHumiditySensor) = 0;
    virtual shared_ptr<HydroponicsSensor> getHumiditySensor() = 0;

    virtual void setAirHumidity(float airHumidity, Hydroponics_UnitsType airHumidityUnits = Hydroponics_UnitsType_Percentile_0_100) = 0;
    virtual void setAirHumidity(HydroponicsSingleMeasurement airHumidity) = 0;
    virtual const HydroponicsSingleMeasurement &getAirHumidity() = 0;
};

// Air CO2 Aware Interface
class HydroponicsAirCO2AwareInterface {
public:
    virtual void setAirCO2Sensor(HydroponicsIdentity airCO2SensorId) = 0;
    virtual void setAirCO2Sensor(shared_ptr<HydroponicsSensor> airCO2Sensor) = 0;
    virtual shared_ptr<HydroponicsSensor> getAirCO2Sensor() = 0;

    virtual void setAirCO2(float airCO2, Hydroponics_UnitsType airCO2Units = Hydroponics_UnitsType_Concentration_PPM) = 0;
    virtual void setAirCO2(HydroponicsSingleMeasurement airCO2) = 0;
    virtual const HydroponicsSingleMeasurement &getAirCO2() = 0;
};

#endif // /ifndef HydroponicsInterfaces_H
