/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Reservoirs
*/

#ifndef HydroReservoirs_H
#define HydroReservoirs_H

class HydroReservoir;
class HydroFluidReservoir;
class HydroFeedReservoir;
class HydroInfiniteReservoir;

struct HydroReservoirData;
struct HydroFluidReservoirData;
struct HydroFeedReservoirData;
struct HydroInfiniteReservoirData;

#include "Hydruino.h"
#include "HydroTriggers.h"

// Creates reservoir object from passed reservoir data (return ownership transfer - user code *must* delete returned object)
extern HydroReservoir *newReservoirObjectFromData(const HydroReservoirData *dataIn);


// Reservoir Base
// This is the base class for all reservoirs, which defines how the reservoir is
// identified, where it lives, what's attached to it, if it is full or empty, and
// who can activate under it.
class HydroReservoir : public HydroObject,
                       public HydroReservoirObjectInterface,
                       public HydroVolumeUnitsInterfaceStorage {
public:
    const enum : signed char { Fluid, Feed, Pipe, Unknown = -1 } classType; // Reservoir class type (custom RTTI)
    inline bool isFluidClass() const { return classType == Fluid; }
    inline bool isFeedClass() const { return classType == Feed; }
    inline bool isPipeClass() const { return classType == Pipe; }
    inline bool isAnyFluidClass() const { return isFluidClass() || isFeedClass(); }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    HydroReservoir(Hydro_ReservoirType reservoirType,
                   hposi_t reservoirIndex,
                   int classType = Unknown);
    HydroReservoir(const HydroReservoirData *dataIn);

    virtual void update() override;

    virtual bool canActivate(HydroActuator *actuator) override;

    virtual void setVolumeUnits(Hydro_UnitsType volumeUnits) override;

    virtual HydroSensorAttachment &getWaterVolumeSensorAttachment() = 0;

    inline Hydro_ReservoirType getReservoirType() const { return _id.objTypeAs.reservoirType; }
    inline hposi_t getReservoirIndex() const { return _id.posIndex; }

    Signal<HydroReservoir *, HYDRO_RESERVOIR_SIGNAL_SLOTS> &getFilledSignal();
    Signal<HydroReservoir *, HYDRO_RESERVOIR_SIGNAL_SLOTS> &getEmptySignal();

protected:
    Hydro_TriggerState _filledState;                        // Filled state (last handled)
    Hydro_TriggerState _emptyState;                         // Empty state (last handled)

    Signal<HydroReservoir *, HYDRO_RESERVOIR_SIGNAL_SLOTS> _filledSignal; // Filled state signal
    Signal<HydroReservoir *, HYDRO_RESERVOIR_SIGNAL_SLOTS> _emptySignal; // Empty state signal

    virtual HydroData *allocateData() const override;
    virtual void saveToData(HydroData *dataOut) override;

    virtual void handleFilled(Hydro_TriggerState filledState);
    virtual void handleEmpty(Hydro_TriggerState emptyState);
    friend class HydroFluidReservoir;
};


// Simple Fluid Reservoir
// Basic fluid reservoir that contains a volume of liquid and the ability to track such.
// Crude, but effective.
class HydroFluidReservoir : public HydroReservoir,
                            public HydroWaterVolumeSensorAttachmentInterface,
                            public HydroFilledTriggerAttachmentInterface,
                            public HydroEmptyTriggerAttachmentInterface {
public:
    HydroFluidReservoir(Hydro_ReservoirType reservoirType,
                        hposi_t reservoirIndex,
                        float maxVolume,
                        int classType = Fluid);
    HydroFluidReservoir(const HydroFluidReservoirData *dataIn);

    virtual void update() override;
    virtual SharedPtr<HydroObjInterface> getSharedPtrFor(const HydroObjInterface *obj) const override;

    virtual bool isFilled(bool poll = false) override;
    virtual bool isEmpty(bool poll = false) override;

    virtual void setVolumeUnits(Hydro_UnitsType volumeUnits) override;

    virtual HydroSensorAttachment &getWaterVolumeSensorAttachment() override;

    virtual HydroTriggerAttachment &getFilledTriggerAttachment() override;

    virtual HydroTriggerAttachment &getEmptyTriggerAttachment() override;

    inline float getMaxVolume() const { return _maxVolume; }

protected:
    float _maxVolume;                                       // Maximum volume
    HydroSensorAttachment _waterVolume;                     // Water volume sensor attachment
    HydroTriggerAttachment _filledTrigger;                  // Filled trigger attachment
    HydroTriggerAttachment _emptyTrigger;                   // Empty trigger attachment

    virtual void saveToData(HydroData *dataOut) override;

    virtual void handleFilled(Hydro_TriggerState filledState) override;
    virtual void handleEmpty(Hydro_TriggerState emptyState) override;
};


// Feed Water Reservoir
// The feed water reservoir can be thought of as an entire feeding channel hub, complete
// with sensors to automate the variety of tasks associated with feeding crops.
// It is recommended to keep crop types and their sow-times similar to one another in any
// one feeding reservoir, as feeding parameters for attached crops are calculated on a per
// feeding reservoir basis, and averaged over the attached crops (taking into account any
// crop weighting offsets).
class HydroFeedReservoir : public HydroFluidReservoir,
                           public HydroAirConcentrateUnitsInterfaceStorage,
                           public HydroTemperatureUnitsInterfaceStorage,
                           public HydroWaterConcentrateUnitsInterfaceStorage,
                           public HydroWaterPHSensorAttachmentInterface,
                           public HydroWaterTDSSensorAttachmentInterface,
                           public HydroWaterTemperatureSensorAttachmentInterface,
                           public HydroAirTemperatureSensorAttachmentInterface,
                           public HydroAirCO2SensorAttachmentInterface {
public:
    HydroFeedReservoir(hposi_t reservoirIndex,
                       float maxVolume,
                       DateTime lastChangeTime = localNow(),
                       DateTime lastPruningTime = localNow(),
                       int classType = Feed);
    HydroFeedReservoir(const HydroFeedReservoirData *dataIn);

    virtual void update() override;
    virtual void handleLowMemory() override;
    virtual SharedPtr<HydroObjInterface> getSharedPtrFor(const HydroObjInterface *obj) const override;

    virtual void setAirConcentrateUnits(Hydro_UnitsType airConcentrateUnits) override;
    virtual void setTemperatureUnits(Hydro_UnitsType temperatureUnits) override;
    virtual void setWaterConcentrateUnits(Hydro_UnitsType waterConcentrateUnits) override;

    virtual HydroSensorAttachment &getWaterPHSensorAttachment() override;
    virtual HydroSensorAttachment &getWaterTDSSensorAttachment() override;
    virtual HydroSensorAttachment &getWaterTemperatureSensorAttachment() override;
    virtual HydroSensorAttachment &getAirTemperatureSensorAttachment() override;
    virtual HydroSensorAttachment &getAirCO2SensorAttachment() override;

    template<typename T> inline void setWaterPHBalancer(T phBalancer) { _waterPHBalancer.setObject(phBalancer); }
    inline SharedPtr<HydroBalancer> getWaterPHBalancer() { return _waterPHBalancer.getObject(); }

    template<typename T> inline void setWaterTDSBalancer(T tdsBalancer) { _waterTDSBalancer.setObject(tdsBalancer); }
    inline SharedPtr<HydroBalancer> getWaterTDSBalancer() { return _waterTDSBalancer.getObject(); }

    template<typename T> inline void setWaterTemperatureBalancer(T waterTempBalancer) { _waterTempBalancer.setObject(waterTempBalancer); }
    inline SharedPtr<HydroBalancer> getWaterTemperatureBalancer() { return _waterTempBalancer.getObject(); }

    template<typename T> inline void setAirTemperatureBalancer(T airTempBalancer) { _airTempBalancer.setObject(airTempBalancer); }
    inline SharedPtr<HydroBalancer> getAirTemperatureBalancer() { return _airTempBalancer.getObject(); }

    template<typename T> inline void setAirCO2Balancer(T co2Balancer) { _airCO2Balancer.setObject(co2Balancer); }
    inline SharedPtr<HydroBalancer> getAirCO2Balancer() { return _airCO2Balancer.getObject(); }

    inline hposi_t getChannelNumber() const { return _id.posIndex; }

    inline DateTime getLastWaterChangeTime() const { return localTime(_lastChangeTime); }
    inline void notifyWaterChanged() { _lastChangeTime = unixTime(localDayStart()); }

    inline DateTime getLastPruningTime() const { return localTime(_lastPruningTime); }
    inline void notifyPruningCompleted() { _lastPruningTime = unixTime(localDayStart()); }

    inline DateTime getLastFeedingTime() const { return localTime(_lastFeedingTime); }
    inline int8_t getFeedingsToday() const { return _numFeedingsToday; }

    inline void notifyFeedingBegan() { _numFeedingsToday++; _lastFeedingTime = unixNow(); }
    inline void notifyFeedingEnded() { ; }
    inline void notifyDayChanged() { _numFeedingsToday = 0; }

protected:
    time_t _lastChangeTime;                                 // Last water change/maintenance date (recycling systems only, UTC)
    time_t _lastPruningTime;                                // Last pruning date (pruning crops only, UTC)
    time_t _lastFeedingTime;                                // Last feeding date (UTC)
    int8_t _numFeedingsToday;                               // Number of feedings performed today

    HydroSensorAttachment _waterPH;                         // Water PH sensor attachment
    HydroSensorAttachment _waterTDS;                        // Water TDS sensor attachment
    HydroSensorAttachment _waterTemp;                       // Water temp sensor attachment
    HydroSensorAttachment _airTemp;                         // Air temp sensor attachment
    HydroSensorAttachment _airCO2;                          // Air CO2 sensor attachment

    HydroBalancerAttachment _waterPHBalancer;               // Water pH balancer (assigned by scheduler when needed)
    HydroBalancerAttachment _waterTDSBalancer;              // Water TDS balancer (assigned by scheduler when needed)
    HydroBalancerAttachment _waterTempBalancer;             // Water temperature balancer (assigned by scheduler when needed)
    HydroBalancerAttachment _airTempBalancer;               // Air temperature balancer (assigned by user if desired)
    HydroBalancerAttachment _airCO2Balancer;                // Air CO2 balancer (assigned by user if desired)

    virtual void saveToData(HydroData *dataOut) override;
};


// Infinite Pipe Reservoir
// An infinite pipe reservoir is like your standard water main - it's not technically
// unlimited, but you can act like it is. Used for reservoirs that should behave as
// alwaysFilled (e.g. water mains) or not (e.g. drainage pipes).
class HydroInfiniteReservoir : public HydroReservoir {
public:
    HydroInfiniteReservoir(Hydro_ReservoirType reservoirType,
                           hposi_t reservoirIndex,
                           bool alwaysFilled = true,
                           int classType = Pipe);
    HydroInfiniteReservoir(const HydroInfiniteReservoirData *dataIn);

    virtual bool isFilled(bool poll = false) override;
    virtual bool isEmpty(bool poll = false) override;

    virtual HydroSensorAttachment &getWaterVolumeSensorAttachment() override;

protected:
    HydroSensorAttachment _waterVolume;                     // Water volume sensor attachment (defunct)
    bool _alwaysFilled;                                     // Always filled flag

    virtual void saveToData(HydroData *dataOut) override;
};


// Reservoir Serialization Data
struct HydroReservoirData : public HydroObjectData {
    Hydro_UnitsType volumeUnits;                            // Volume units

    HydroReservoirData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Fluid Reservoir Serialization Data
struct HydroFluidReservoirData : public HydroReservoirData {
    float maxVolume;                                        // Maximum volume
    char volumeSensor[HYDRO_NAME_MAXSIZE];                  // Volume sensor
    HydroTriggerSubData filledTrigger;                      // Filled trigger
    HydroTriggerSubData emptyTrigger;                       // Empty trigger

    HydroFluidReservoirData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Feed Water Reservoir Serialization Data
struct HydroFeedReservoirData : public HydroFluidReservoirData {
    time_t lastChangeTime;                                  // Last water change/maintenance time (UTC)
    time_t lastPruningTime;                                 // Last pruning time (UTC)
    time_t lastFeedingTime;                                 // Last feeding time
    uint8_t numFeedingsToday;                               // Number feedings on the day
    Hydro_UnitsType airConcentrateUnits;                    // Air concentration units
    Hydro_UnitsType temperatureUnits;                       // Temperature units
    Hydro_UnitsType waterConcentrateUnits;                  // Water concentration units
    char waterPHSensor[HYDRO_NAME_MAXSIZE];                 // pH sensor
    char waterTDSSensor[HYDRO_NAME_MAXSIZE];                // TDS sensor
    char waterTempSensor[HYDRO_NAME_MAXSIZE];               // Water temp sensor
    char airTempSensor[HYDRO_NAME_MAXSIZE];                 // Air temp sensor
    char airCO2Sensor[HYDRO_NAME_MAXSIZE];                  // Air CO2 sensor

    HydroFeedReservoirData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Infinite Pipe Reservoir Serialization Data
struct HydroInfiniteReservoirData : public HydroReservoirData {
    bool alwaysFilled;                                      // Always filled flag

    HydroInfiniteReservoirData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroReservoirs_H
