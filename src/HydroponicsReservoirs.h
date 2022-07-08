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
    inline bool isUnknownClass() const { return classType <= Unknown; }

    HydroponicsReservoir(Hydroponics_ReservoirType reservoirType,
                         Hydroponics_PositionIndex reservoirIndex,
                         int classType = Unknown);
    HydroponicsReservoir(const HydroponicsReservoirData *dataIn);
    virtual ~HydroponicsReservoir();

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    virtual bool canActivate(HydroponicsActuator *actuator) const = 0;
    virtual bool getIsFull() const = 0;
    virtual bool getIsEmpty() const = 0;

    virtual bool addActuator(HydroponicsActuator *actuator) override;
    virtual bool removeActuator(HydroponicsActuator *actuator) override;
    virtual bool hasActuator(HydroponicsActuator *actuator) const override;
    virtual arx::map<Hydroponics_KeyType, HydroponicsObject *, HYDRUINO_OBJ_LINKS_MAXSIZE> getActuators() const override;

    virtual bool addSensor(HydroponicsSensor *sensor) override;
    virtual bool removeSensor(HydroponicsSensor *sensor) override;
    virtual bool hasSensor(HydroponicsSensor *sensor) const override;
    virtual arx::map<Hydroponics_KeyType, HydroponicsObject *, HYDRUINO_OBJ_LINKS_MAXSIZE> getSensors() const override;

    virtual bool addCrop(HydroponicsCrop *crop) override;
    virtual bool removeCrop(HydroponicsCrop *crop) override;
    virtual bool hasCrop(HydroponicsCrop *crop) const override;
    virtual arx::map<Hydroponics_KeyType, HydroponicsObject *, HYDRUINO_OBJ_LINKS_MAXSIZE> getCrops() const override;

    Hydroponics_ReservoirType getReservoirType() const;
    Hydroponics_PositionIndex getReservoirIndex() const;

    Signal<HydroponicsReservoir *> &getFilledSignal();
    Signal<HydroponicsReservoir *> &getEmptySignal();

protected:
    Signal<HydroponicsReservoir *> _filledSignal;           // Filled state signal
    Signal<HydroponicsReservoir *> _emptySignal;            // Empty state signal

    virtual HydroponicsData *allocateData() const override;
    virtual void saveToData(HydroponicsData *dataOut) override;
};


// Simple Fluid Reservoir
// Basic fluid reservoir that contains a volume of liquid and the ability to track
// such. Crude, but effective.
// Optional channel number is used mainly to track specific feed water reservoirs
// but is usually (but not always) aliasing reservoir type position index.
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

    virtual bool canActivate(HydroponicsActuator *actuator) const override;
    virtual bool getIsFull() const override;
    virtual bool getIsEmpty() const override;

    void setVolumeUnits(Hydroponics_UnitsType volumeUnits);
    Hydroponics_UnitsType getVolumeUnits() const;

    virtual void setVolumeSensor(HydroponicsIdentity volumeSensorId) override;
    virtual void setVolumeSensor(shared_ptr<HydroponicsSensor> volumeSensor) override;
    virtual shared_ptr<HydroponicsSensor> getVolumeSensor() override;

    virtual void setWaterVolume(float waterVolume, Hydroponics_UnitsType waterVolumeUnits = Hydroponics_UnitsType_Undefined) override;
    virtual void setWaterVolume(HydroponicsSingleMeasurement waterVolume) override;
    virtual const HydroponicsSingleMeasurement &getWaterVolume() const override;

    void setFilledTrigger(HydroponicsTrigger *filledTrigger);
    const HydroponicsTrigger *getFilledTrigger() const;

    void setEmptyTrigger(HydroponicsTrigger *emptyTrigger);
    const HydroponicsTrigger *getEmptyTrigger() const;

    float getMaxVolume();

protected:
    float _maxVolume;                                       // Maximum volume
    Hydroponics_UnitsType _volumeUnits;                     // Volume units preferred (else default)
    HydroponicsDLinkObject<HydroponicsSensor> _volumeSensor; // Volume sensor linkage
    HydroponicsSingleMeasurement _waterVolume;              // Current water volume measure
    HydroponicsTrigger *_filledTrigger;                     // Filled trigger (owned)
    HydroponicsTrigger *_emptyTrigger;                      // Empty trigger (owned)

    virtual void saveToData(HydroponicsData *dataOut) override;

    void attachWaterVolumeSensor();
    void detachWaterVolumeSensor();
    void handleWaterVolumeMeasure(const HydroponicsMeasurement *measurement);
};


// Feed Water Reservoir
// TODO
class HydroponicsFeedReservoir : public HydroponicsFluidReservoir, public HydroponicsWaterPHAwareInterface, public HydroponicsWaterTDSAwareInterface, public HydroponicsWaterTemperatureAwareInterface {
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

    virtual void setWaterPHSensor(HydroponicsIdentity phSensorId) override;
    virtual void setWaterPHSensor(shared_ptr<HydroponicsSensor> phSensor) override;
    virtual shared_ptr<HydroponicsSensor> getWaterPHSensor() override;

    virtual void setWaterPH(float waterPH, Hydroponics_UnitsType waterPHUnits = Hydroponics_UnitsType_pHScale_0_14) override;
    virtual void setWaterPH(HydroponicsSingleMeasurement waterPH) override;
    virtual const HydroponicsSingleMeasurement &getWaterPH() const override;

    virtual void setWaterTDSSensor(HydroponicsIdentity tdsSensorId) override;
    virtual void setWaterTDSSensor(shared_ptr<HydroponicsSensor> tdsSensor) override;
    virtual shared_ptr<HydroponicsSensor> getWaterTDSSensor() override;

    virtual void setWaterTDS(float waterTDS, Hydroponics_UnitsType waterTDSUnits = Hydroponics_UnitsType_Undefined) override;
    virtual void setWaterTDS(HydroponicsSingleMeasurement waterTDS) override;
    virtual const HydroponicsSingleMeasurement &getWaterTDS() const override;

    virtual void setWaterTempSensor(HydroponicsIdentity waterTempSensorId) override;
    virtual void setWaterTempSensor(shared_ptr<HydroponicsSensor> waterTempSensor) override;
    virtual shared_ptr<HydroponicsSensor> getWaterTempSensor() override;

    virtual void setWaterTemperature(float waterTemperature, Hydroponics_UnitsType waterTempUnits = Hydroponics_UnitsType_Undefined) override;
    virtual void setWaterTemperature(HydroponicsSingleMeasurement waterTemperature) override;
    virtual const HydroponicsSingleMeasurement &getWaterTemperature() const override;

    HydroponicsBalancer *setWaterPHBalancer(float phSetpoint, Hydroponics_UnitsType phSetpointUnits);
    void setWaterPHBalancer(HydroponicsBalancer *phBalancer);
    HydroponicsBalancer *getWaterPHBalancer() const;

    HydroponicsBalancer *setWaterTDSBalancer(float tdsSetpoint, Hydroponics_UnitsType tdsSetpointUnits);
    void setWaterTDSBalancer(HydroponicsBalancer *tdsBalancer);
    HydroponicsBalancer *getWaterTDSBalancer() const;

    HydroponicsBalancer *setWaterTempBalancer(float tempSetpoint, Hydroponics_UnitsType tempSetpointUnits);
    void setWaterTempBalancer(HydroponicsBalancer *tempBalancer);
    HydroponicsBalancer *getWaterTempBalancer() const;

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
    HydroponicsDLinkObject<HydroponicsSensor> _phSensor;    // PH sensor
    HydroponicsDLinkObject<HydroponicsSensor> _tdsSensor;   // TDS sensor
    HydroponicsDLinkObject<HydroponicsSensor> _tempSensor;  // Temperature sensor
    HydroponicsSingleMeasurement _waterPH;                  // Current PH alkalinity measure
    HydroponicsSingleMeasurement _waterTDS;                 // Current TDS concentration measure
    HydroponicsSingleMeasurement _waterTemp;                // Current water temperature measure
    HydroponicsBalancer *_phBalancer;                       // PH balancer (assigned by scheduler when needed)
    HydroponicsBalancer *_tdsBalancer;                      // TDS balancer (assigned by scheduler when needed)
    HydroponicsBalancer *_tempBalancer;                     // Temperature balancer (assigned by scheduler when needed)

    virtual void saveToData(HydroponicsData *dataOut) override;

    void setupPHBalancer();
    void setupTDSBalancer();
    void setupTempBalancer();

    void attachPHSensor();
    void detachPHSensor();
    void handlePHMeasure(const HydroponicsMeasurement *measurement);
    void attachTDSSensor();
    void detachTDSSensor();
    void handleTDSMeasure(const HydroponicsMeasurement *measurement);
    void attachWaterTempSensor();
    void detachWaterTempSensor();
    void handleWaterTempMeasure(const HydroponicsMeasurement *measurement);
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

    virtual bool canActivate(HydroponicsActuator *actuator) const override;
    virtual bool getIsFull() const override;
    virtual bool getIsEmpty() const override;

protected:
    bool _alwaysFilled;                                     // Always filled flag

    virtual void saveToData(HydroponicsData *dataOut) override;
};


// Reservoir Serialization Data
struct HydroponicsReservoirData : public HydroponicsObjectData {
    HydroponicsReservoirData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Fluid Reservoir Serialization Data
struct HydroponicsFluidReservoirData : public HydroponicsReservoirData {
    float maxVolume;
    Hydroponics_UnitsType volumeUnits;
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
    char phSensorName[HYDRUINO_NAME_MAXSIZE];
    char tdsSensorName[HYDRUINO_NAME_MAXSIZE];
    char tempSensorName[HYDRUINO_NAME_MAXSIZE];

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
