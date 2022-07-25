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
    virtual void handleLowMemory() override;

    virtual bool canActivate(HydroponicsActuator *actuator);
    virtual bool isFilled() = 0;
    virtual bool isEmpty() = 0;

    virtual void setVolumeUnits(Hydroponics_UnitsType volumeUnits);
    virtual Hydroponics_UnitsType getVolumeUnits() const;

    virtual HydroponicsSensorAttachment &getWaterVolume() = 0;

    virtual bool addActuator(HydroponicsActuator *actuator) override;
    virtual bool removeActuator(HydroponicsActuator *actuator) override;
    virtual bool hasActuator(HydroponicsActuator *actuator) const override;

    virtual bool addSensor(HydroponicsSensor *sensor) override;
    virtual bool removeSensor(HydroponicsSensor *sensor) override;
    virtual bool hasSensor(HydroponicsSensor *sensor) const override;

    virtual bool addCrop(HydroponicsCrop *crop) override;
    virtual bool removeCrop(HydroponicsCrop *crop) override;
    virtual bool hasCrop(HydroponicsCrop *crop) const override;

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
class HydroponicsFluidReservoir : public HydroponicsReservoir, public HydroponicsVolumeSensorAttachmentInterface {
public:
    HydroponicsFluidReservoir(Hydroponics_ReservoirType reservoirType,
                              Hydroponics_PositionIndex reservoirIndex,
                              float maxVolume,
                              int classType = Fluid);
    HydroponicsFluidReservoir(const HydroponicsFluidReservoirData *dataIn);
    virtual ~HydroponicsFluidReservoir();

    virtual void update() override;
    virtual void handleLowMemory() override;

    virtual bool isFilled() override;
    virtual bool isEmpty() override;

    virtual void setVolumeUnits(Hydroponics_UnitsType volumeUnits) override;

    virtual HydroponicsSensorAttachment &getWaterVolume() override;

    void setFilledTrigger(HydroponicsTrigger *filledTrigger);
    const HydroponicsTrigger *getFilledTrigger() const;

    void setEmptyTrigger(HydroponicsTrigger *emptyTrigger);
    const HydroponicsTrigger *getEmptyTrigger() const;

    float getMaxVolume();

protected:
    float _maxVolume;                                       // Maximum volume
    HydroponicsSensorAttachment _waterVolume;               // Water volume sensor attachment
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
};


// Feed Water Reservoir
// The feed water reservoir can be thought of as an entire feeding channel hub, complete
// with sensors to automate the variety of tasks associated with feeding crops.
class HydroponicsFeedReservoir : public HydroponicsFluidReservoir, public HydroponicsWaterPHSensorAttachmentInterface, public HydroponicsWaterTDSSensorAttachmentInterface, public HydroponicsWaterTemperatureSensorAttachmentInterface,  public HydroponicsAirTemperatureSensorAttachmentInterface, public HydroponicsAirCO2SensorAttachmentInterface {
public:
    HydroponicsFeedReservoir(Hydroponics_PositionIndex reservoirIndex,
                             float maxVolume,
                             DateTime lastChangeDate = DateTime((uint32_t)unixNow()),
                             DateTime lastPruningDate = DateTime(),
                             int classType = Feed);
    HydroponicsFeedReservoir(const HydroponicsFeedReservoirData *dataIn);
    virtual ~HydroponicsFeedReservoir();

    virtual void update() override;
    virtual void handleLowMemory() override;

    void setTDSUnits(Hydroponics_UnitsType tdsUnits);
    Hydroponics_UnitsType getTDSUnits() const;

    void setTemperatureUnits(Hydroponics_UnitsType tempUnits);
    Hydroponics_UnitsType getTemperatureUnits() const;

    virtual HydroponicsSensorAttachment &getWaterPH() override;

    virtual HydroponicsSensorAttachment &getWaterTDS() override;

    virtual HydroponicsSensorAttachment &getWaterTemperature() override;

    virtual HydroponicsSensorAttachment &getAirTemperature() override;

    virtual HydroponicsSensorAttachment &getAirCO2() override;

    HydroponicsBalancer *setWaterPHBalancer(float phSetpoint, Hydroponics_UnitsType phSetpointUnits);
    void setWaterPHBalancer(HydroponicsBalancer *phBalancer);
    HydroponicsBalancer *getWaterPHBalancer() const;

    HydroponicsBalancer *setWaterTDSBalancer(float tdsSetpoint, Hydroponics_UnitsType tdsSetpointUnits);
    void setWaterTDSBalancer(HydroponicsBalancer *tdsBalancer);
    HydroponicsBalancer *getWaterTDSBalancer() const;

    HydroponicsBalancer *setWaterTemperatureBalancer(float tempSetpoint, Hydroponics_UnitsType tempSetpointUnits);
    void setWaterTemperatureBalancer(HydroponicsBalancer *waterTempBalancer);
    HydroponicsBalancer *getWaterTemperatureBalancer() const;

    HydroponicsBalancer *setAirTemperatureBalancer(float tempSetpoint, Hydroponics_UnitsType tempSetpointUnits);
    void setAirTemperatureBalancer(HydroponicsBalancer *airTempBalancer);
    HydroponicsBalancer *getAirTemperatureBalancer() const;

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
    time_t _lastChangeDate;                                 // Last water change date (recycling systems only, UTC)
    time_t _lastPruningDate;                                // Last pruning date (pruning crops only, UTC)
    time_t _lastFeedingDate;                                // Last feeding date (UTC)
    int _numFeedingsToday;                                  // Number of feedings performed today
    Hydroponics_UnitsType _tdsUnits;                        // TDS units preferred (else default)
    Hydroponics_UnitsType _tempUnits;                       // Temperature units preferred (else default)

    HydroponicsSensorAttachment _waterPH;                   // Water PH sensor attachment
    HydroponicsSensorAttachment _waterTDS;                  // Water TDS sensor attachment
    HydroponicsSensorAttachment _waterTemp;                 // Water temp sensor attachment
    HydroponicsSensorAttachment _airTemp;                   // Air temp sensor attachment
    HydroponicsSensorAttachment _airCO2;                    // Air CO2 sensor attachment

    HydroponicsBalancer *_waterPHBalancer;                  // Water pH balancer (assigned by scheduler when needed)
    HydroponicsBalancer *_waterTDSBalancer;                 // Water TDS balancer (assigned by scheduler when needed)
    HydroponicsBalancer *_waterTempBalancer;                // Water temperature balancer (assigned by scheduler when needed)
    HydroponicsBalancer *_airTempBalancer;                  // Air temperature balancer (assigned by user if desired)
    HydroponicsBalancer *_airCO2Balancer;                   // Air CO2 balancer (assigned by user if desired)

    virtual void saveToData(HydroponicsData *dataOut) override;
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

    virtual bool isFilled() override;
    virtual bool isEmpty() override;

    virtual HydroponicsSensorAttachment &getWaterVolume() override;

protected:
    HydroponicsSensorAttachment _waterVolume;               // Water volume sensor attachment (defunct)
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
    char volumeSensor[HYDRUINO_NAME_MAXSIZE];
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
    char waterPHSensor[HYDRUINO_NAME_MAXSIZE];
    char waterTDSSensor[HYDRUINO_NAME_MAXSIZE];
    char waterTempSensor[HYDRUINO_NAME_MAXSIZE];
    char airTempSensor[HYDRUINO_NAME_MAXSIZE];
    char airCO2Sensor[HYDRUINO_NAME_MAXSIZE];

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
