/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Reservoirs
*/

#ifndef HydroponicsReservoirs_H
#define HydroponicsReservoirs_H

class HydroponicsReservoir;
class HydroponicsFluidReservoir;
class HydroponicsFeedReservoir;
class HydroponicsInfiniteReservoir;

struct HydroponicsReservoirData;
struct HydroponicsFluidReservoirData;
struct HydroponicsFeedReservoirData;
struct HydroponicsInfiniteReservoirData;

#include "Hydroponics.h"
#include "HydroponicsTriggers.h"

// Creates reservoir object from passed reservoir data (return ownership transfer - user code *must* delete returned object)
extern HydroponicsReservoir *newReservoirObjectFromData(const HydroponicsReservoirData *dataIn);


// Hydroponics Reservoir Base
// This is the base class for all reservoirs, which defines how the reservoir is
// identified, where it lives, what's attached to it, if it is full or empty, and
// who can activate under it.
class HydroponicsReservoir : public HydroponicsObject, public HydroponicsReservoirObjectInterface, public HydroponicsActuatorAttachmentsInterface, public HydroponicsSensorAttachmentsInterface, public HydroponicsCropAttachmentsInterface {
public:
    const enum { Fluid, Feed, Pipe, Unknown = -1 } classType; // Reservoir class type (custom RTTI)
    inline bool isFluidClass() const { return classType == Fluid; }
    inline bool isFeedClass() const { return classType == Feed; }
    inline bool isPipeClass() const { return classType == Pipe; }
    inline bool isAnyFluidClass() const { return isFluidClass() || isFeedClass(); }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    HydroponicsReservoir(Hydroponics_ReservoirType reservoirType,
                         Hydroponics_PositionIndex reservoirIndex,
                         int classType = Unknown);
    HydroponicsReservoir(const HydroponicsReservoirData *dataIn);
    virtual ~HydroponicsReservoir();

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    virtual bool canActivate(HydroponicsActuator *actuator);
    virtual bool getIsFilled() const = 0;
    virtual bool getIsEmpty() const = 0;

    virtual void setVolumeUnits(Hydroponics_UnitsType volumeUnits);
    virtual Hydroponics_UnitsType getVolumeUnits() const;

    virtual void setWaterVolume(float waterVolume, Hydroponics_UnitsType waterVolumeUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual void setWaterVolume(HydroponicsSingleMeasurement waterVolume) = 0;
    virtual const HydroponicsSingleMeasurement &getWaterVolume() = 0;

    virtual bool addActuator(HydroponicsActuator *actuator) override;
    virtual bool removeActuator(HydroponicsActuator *actuator) override;
    virtual bool hasActuator(HydroponicsActuator *actuator) const override;
    virtual Map<Hydroponics_KeyType, HydroponicsObject *, HYDRUINO_OBJ_LINKS_MAXSIZE>::type getActuators() const override;

    virtual bool addSensor(HydroponicsSensor *sensor) override;
    virtual bool removeSensor(HydroponicsSensor *sensor) override;
    virtual bool hasSensor(HydroponicsSensor *sensor) const override;
    virtual Map<Hydroponics_KeyType, HydroponicsObject *, HYDRUINO_OBJ_LINKS_MAXSIZE>::type getSensors() const override;

    virtual bool addCrop(HydroponicsCrop *crop) override;
    virtual bool removeCrop(HydroponicsCrop *crop) override;
    virtual bool hasCrop(HydroponicsCrop *crop) const override;
    virtual Map<Hydroponics_KeyType, HydroponicsObject *, HYDRUINO_OBJ_LINKS_MAXSIZE>::type getCrops() const override;

    Hydroponics_ReservoirType getReservoirType() const;
    Hydroponics_PositionIndex getReservoirIndex() const;

    Signal<HydroponicsReservoir *> &getFilledSignal();
    Signal<HydroponicsReservoir *> &getEmptySignal();

protected:
    Hydroponics_UnitsType _volumeUnits;                     // Volume units preferred (else default)
    Hydroponics_TriggerState _filledState;                  // Current filled state
    Hydroponics_TriggerState _emptyState;                   // Current empty state
    Signal<HydroponicsReservoir *> _filledSignal;           // Filled state signal
    Signal<HydroponicsReservoir *> _emptySignal;            // Empty state signal

    virtual HydroponicsData *allocateData() const override;
    virtual void saveToData(HydroponicsData *dataOut) override;

    virtual void handleFilledState();
    virtual void handleEmptyState();
};


// Simple Fluid Reservoir
// Basic fluid reservoir that contains a volume of liquid and the ability to track such.
// Crude, but effective.
class HydroponicsFluidReservoir : public HydroponicsReservoir, public HydroponicsVolumeAwareInterface {
public:
    HydroponicsFluidReservoir(Hydroponics_ReservoirType reservoirType,
                              Hydroponics_PositionIndex reservoirIndex,
                              float maxVolume,
                              int classType = Fluid);
    HydroponicsFluidReservoir(const HydroponicsFluidReservoirData *dataIn);
    virtual ~HydroponicsFluidReservoir();

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    virtual bool getIsFilled() const override;
    virtual bool getIsEmpty() const override;

    virtual void setVolumeUnits(Hydroponics_UnitsType volumeUnits) override;

    virtual void setVolumeSensor(HydroponicsIdentity volumeSensorId) override;
    virtual void setVolumeSensor(shared_ptr<HydroponicsSensor> volumeSensor) override;
    virtual shared_ptr<HydroponicsSensor> getVolumeSensor() override;

    virtual void setWaterVolume(float waterVolume, Hydroponics_UnitsType waterVolumeUnits = Hydroponics_UnitsType_Undefined) override;
    virtual void setWaterVolume(HydroponicsSingleMeasurement waterVolume) override;
    virtual const HydroponicsSingleMeasurement &getWaterVolume() override;

    void setFilledTrigger(HydroponicsTrigger *filledTrigger);
    const HydroponicsTrigger *getFilledTrigger() const;

    void setEmptyTrigger(HydroponicsTrigger *emptyTrigger);
    const HydroponicsTrigger *getEmptyTrigger() const;

    float getMaxVolume();

protected:
    float _maxVolume;                                       // Maximum volume
    HydroponicsSingleMeasurement _waterVolume;              // Current water volume measure
    bool _needsVolumeUpdate;                                // Needs water volume update tracking flag
    HydroponicsDLinkObject<HydroponicsSensor> _volumeSensor; // Volume sensor linkage
    HydroponicsTrigger *_filledTrigger;                     // Filled trigger (owned)
    HydroponicsTrigger *_emptyTrigger;                      // Empty trigger (owned)

    virtual void saveToData(HydroponicsData *dataOut) override;

    virtual void handleFilledState() override;
    virtual void handleEmptyState() override;

    void attachFilledTrigger();
    void detachFilledTrigger();
    void handleFilledTrigger(Hydroponics_TriggerState triggerState);
    void attachEmptyTrigger();
    void detachEmptyTrigger();
    void handleEmptyTrigger(Hydroponics_TriggerState triggerState);
    void attachWaterVolumeSensor();
    void detachWaterVolumeSensor();
    void handleWaterVolumeMeasure(const HydroponicsMeasurement *measurement);
};


// Feed Water Reservoir
// The feed water reservoir can be thought of as an entire feeding channel hub, complete
// with sensors to automate the variety of tasks associated with feeding crops.
class HydroponicsFeedReservoir : public HydroponicsFluidReservoir, public HydroponicsWaterPHAwareInterface, public HydroponicsWaterTDSAwareInterface, public HydroponicsWaterTemperatureAwareInterface,  public HydroponicsAirTemperatureAwareInterface, public HydroponicsAirCO2AwareInterface {
public:
    HydroponicsFeedReservoir(Hydroponics_PositionIndex reservoirIndex,
                             float maxVolume,
                             DateTime lastChangeDate = DateTime((uint32_t)now()),
                             DateTime lastPruningDate = DateTime((uint32_t)0),
                             int classType = Feed);
    HydroponicsFeedReservoir(const HydroponicsFeedReservoirData *dataIn);
    virtual ~HydroponicsFeedReservoir();

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    void setTDSUnits(Hydroponics_UnitsType tdsUnits);
    Hydroponics_UnitsType getTDSUnits() const;

    void setTemperatureUnits(Hydroponics_UnitsType tempUnits);
    Hydroponics_UnitsType getTemperatureUnits() const;

    virtual void setWaterPHSensor(HydroponicsIdentity waterPHSensorId) override;
    virtual void setWaterPHSensor(shared_ptr<HydroponicsSensor> waterPHSensor) override;
    virtual shared_ptr<HydroponicsSensor> getWaterPHSensor() override;

    virtual void setWaterPH(float waterPH, Hydroponics_UnitsType waterPHUnits = Hydroponics_UnitsType_pHScale_0_14) override;
    virtual void setWaterPH(HydroponicsSingleMeasurement waterPH) override;
    virtual const HydroponicsSingleMeasurement &getWaterPH() override;

    virtual void setWaterTDSSensor(HydroponicsIdentity waterTDSSensorId) override;
    virtual void setWaterTDSSensor(shared_ptr<HydroponicsSensor> waterTDSSensor) override;
    virtual shared_ptr<HydroponicsSensor> getWaterTDSSensor() override;

    virtual void setWaterTDS(float waterTDS, Hydroponics_UnitsType waterTDSUnits = Hydroponics_UnitsType_Undefined) override;
    virtual void setWaterTDS(HydroponicsSingleMeasurement waterTDS) override;
    virtual const HydroponicsSingleMeasurement &getWaterTDS() override;

    virtual void setWaterTempSensor(HydroponicsIdentity waterTempSensorId) override;
    virtual void setWaterTempSensor(shared_ptr<HydroponicsSensor> waterTempSensor) override;
    virtual shared_ptr<HydroponicsSensor> getWaterTempSensor() override;

    virtual void setWaterTemperature(float waterTemperature, Hydroponics_UnitsType waterTempUnits = Hydroponics_UnitsType_Undefined) override;
    virtual void setWaterTemperature(HydroponicsSingleMeasurement waterTemperature) override;
    virtual const HydroponicsSingleMeasurement &getWaterTemperature() override;

    virtual void setAirTempSensor(HydroponicsIdentity airTempSensorId) override;
    virtual void setAirTempSensor(shared_ptr<HydroponicsSensor> airTempSensor) override;
    virtual shared_ptr<HydroponicsSensor> getAirTempSensor() override;

    virtual void setAirTemperature(float airTemperature, Hydroponics_UnitsType airTempUnits = Hydroponics_UnitsType_Undefined) override;
    virtual void setAirTemperature(HydroponicsSingleMeasurement airTemperature) override;
    virtual const HydroponicsSingleMeasurement &getAirTemperature() override;

    virtual void setAirCO2Sensor(HydroponicsIdentity airCO2SensorId) override;
    virtual void setAirCO2Sensor(shared_ptr<HydroponicsSensor> airCO2Sensor) override;
    virtual shared_ptr<HydroponicsSensor> getAirCO2Sensor() override;

    virtual void setAirCO2(float airCO2, Hydroponics_UnitsType airCO2Units = Hydroponics_UnitsType_Concentration_PPM) override;
    virtual void setAirCO2(HydroponicsSingleMeasurement airCO2) override;
    virtual const HydroponicsSingleMeasurement &getAirCO2() override;

    HydroponicsBalancer *setWaterPHBalancer(float phSetpoint, Hydroponics_UnitsType phSetpointUnits);
    void setWaterPHBalancer(HydroponicsBalancer *phBalancer);
    HydroponicsBalancer *getWaterPHBalancer() const;

    HydroponicsBalancer *setWaterTDSBalancer(float tdsSetpoint, Hydroponics_UnitsType tdsSetpointUnits);
    void setWaterTDSBalancer(HydroponicsBalancer *tdsBalancer);
    HydroponicsBalancer *getWaterTDSBalancer() const;

    HydroponicsBalancer *setWaterTempBalancer(float tempSetpoint, Hydroponics_UnitsType tempSetpointUnits);
    void setWaterTempBalancer(HydroponicsBalancer *waterTempBalancer);
    HydroponicsBalancer *getWaterTempBalancer() const;

    HydroponicsBalancer *setAirTempBalancer(float tempSetpoint, Hydroponics_UnitsType tempSetpointUnits);
    void setAirTempBalancer(HydroponicsBalancer *airTempBalancer);
    HydroponicsBalancer *getAirTempBalancer() const;

    HydroponicsBalancer *setAirCO2Balancer(float co2Setpoint, Hydroponics_UnitsType co2SetpointUnits);
    void setAirCO2Balancer(HydroponicsBalancer *co2Balancer);
    HydroponicsBalancer *getAirCO2Balancer() const;

    Hydroponics_PositionIndex getChannelNumber() const;

    DateTime getLastWaterChangeDate() const;
    void notifyWaterChanged();

    DateTime getLastPruningDate() const;
    void notifyPruningCompleted();

    DateTime getLastFeeding() const;
    int getFeedingsToday() const;
    void notifyFeedingBegan();
    void notifyFeedingEnded();
    void notifyDayChanged();

protected:
    time_t _lastChangeDate;                                 // Last water change date (recycling systems only)
    time_t _lastPruningDate;                                // Last pruning date (pruning crops only)
    time_t _lastFeedingDate;                                // Last feeding date
    int _numFeedingsToday;                                  // Number of feedings performed today
    Hydroponics_UnitsType _tdsUnits;                        // TDS units preferred (else default)
    Hydroponics_UnitsType _tempUnits;                       // Temperature units preferred (else default)
    HydroponicsDLinkObject<HydroponicsSensor> _waterPHSensor; // Water pH sensor
    HydroponicsDLinkObject<HydroponicsSensor> _waterTDSSensor; // Water TDS sensor
    HydroponicsDLinkObject<HydroponicsSensor> _waterTempSensor; // Water temperature sensor
    HydroponicsDLinkObject<HydroponicsSensor> _airTempSensor; // Air temperature sensor
    HydroponicsDLinkObject<HydroponicsSensor> _airCO2Sensor; // Air CO2 sensor
    bool _needsWaterPHUpdate;                               // Needs water pH update tracking flag
    bool _needsWaterTDSUpdate;                              // Needs water TDS update tracking flag
    bool _needsWaterTempUpdate;                             // Needs water temperature update tracking flag
    bool _needsAirTempUpdate;                               // Needs air temperature update tracking flag
    bool _needsAirCO2Update;                                // Needs air CO2 level update tracking flag
    HydroponicsSingleMeasurement _waterPH;                  // Current pH alkalinity measure
    HydroponicsSingleMeasurement _waterTDS;                 // Current TDS concentration measure
    HydroponicsSingleMeasurement _waterTemp;                // Current water temperature measure
    HydroponicsSingleMeasurement _airTemp;                  // Current air temperature measure
    HydroponicsSingleMeasurement _airCO2;                   // Current air CO2 level measure
    HydroponicsBalancer *_waterPHBalancer;                  // Water pH balancer (assigned by scheduler when needed)
    HydroponicsBalancer *_waterTDSBalancer;                 // Water TDS balancer (assigned by scheduler when needed)
    HydroponicsBalancer *_waterTempBalancer;                // Water temperature balancer (assigned by scheduler when needed)
    HydroponicsBalancer *_airTempBalancer;                  // Air temperature balancer (assigned by user if desired)
    HydroponicsBalancer *_airCO2Balancer;                   // Air CO2 balancer (assigned by user if desired)

    virtual void saveToData(HydroponicsData *dataOut) override;

    void attachWaterPHSensor();
    void detachWaterPHSensor();
    void handleWaterPHMeasure(const HydroponicsMeasurement *measurement);
    void attachWaterTDSSensor();
    void detachWaterTDSSensor();
    void handleWaterTDSMeasure(const HydroponicsMeasurement *measurement);
    void attachWaterTempSensor();
    void detachWaterTempSensor();
    void handleWaterTempMeasure(const HydroponicsMeasurement *measurement);
    void attachAirTempSensor();
    void detachAirTempSensor();
    void handleAirTempMeasure(const HydroponicsMeasurement *measurement);
    void attachAirCO2Sensor();
    void detachAirCO2Sensor();
    void handleAirCO2Measure(const HydroponicsMeasurement *measurement);
};


// Infinite Pipe Reservoir
// An infinite pipe reservoir is like your standard water main - it's not technically
// unlimited, but you can act like it is. Used for reservoirs that should behave as
// alwaysFilled (e.g. water mains) or not (e.g. drainage pipes).
class HydroponicsInfiniteReservoir : public HydroponicsReservoir {
public:
    HydroponicsInfiniteReservoir(Hydroponics_ReservoirType reservoirType,
                                 Hydroponics_PositionIndex reservoirIndex,
                                 bool alwaysFilled = true,
                                 int classType = Pipe);
    HydroponicsInfiniteReservoir(const HydroponicsInfiniteReservoirData *dataIn);
    virtual ~HydroponicsInfiniteReservoir();

    virtual bool getIsFilled() const override;
    virtual bool getIsEmpty() const override;

    virtual void setWaterVolume(float waterVolume, Hydroponics_UnitsType waterVolumeUnits = Hydroponics_UnitsType_Undefined) override;
    virtual void setWaterVolume(HydroponicsSingleMeasurement waterVolume) override;
    virtual const HydroponicsSingleMeasurement &getWaterVolume() override;

protected:
    bool _alwaysFilled;                                     // Always filled flag

    virtual void saveToData(HydroponicsData *dataOut) override;
};


// Reservoir Serialization Data
struct HydroponicsReservoirData : public HydroponicsObjectData {
    Hydroponics_UnitsType volumeUnits;

    HydroponicsReservoirData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Fluid Reservoir Serialization Data
struct HydroponicsFluidReservoirData : public HydroponicsReservoirData {
    float maxVolume;
    char volumeSensorName[HYDRUINO_NAME_MAXSIZE];
    HydroponicsTriggerSubData filledTrigger;
    HydroponicsTriggerSubData emptyTrigger;

    HydroponicsFluidReservoirData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Feed Water Reservoir Serialization Data
struct HydroponicsFeedReservoirData : public HydroponicsFluidReservoirData {
    time_t lastChangeDate;
    time_t lastPruningDate;
    time_t lastFeedingDate;
    uint8_t numFeedingsToday;
    Hydroponics_UnitsType tdsUnits;
    Hydroponics_UnitsType tempUnits;
    char waterPHSensorName[HYDRUINO_NAME_MAXSIZE];
    char waterTDSSensorName[HYDRUINO_NAME_MAXSIZE];
    char waterTempSensorName[HYDRUINO_NAME_MAXSIZE];
    char airTempSensorName[HYDRUINO_NAME_MAXSIZE];
    char airCO2SensorName[HYDRUINO_NAME_MAXSIZE];

    HydroponicsFeedReservoirData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Infinite Pipe Reservoir Serialization Data
struct HydroponicsInfiniteReservoirData : public HydroponicsReservoirData {
    bool alwaysFilled;

    HydroponicsInfiniteReservoirData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroponicsReservoirs_H
