/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Crops
*/

#ifndef HydroCrops_H
#define HydroCrops_H

class HydroCrop;
class HydroTimedCrop;
class HydroAdaptiveCrop;

struct HydroCropData;
struct HydroTimedCropData;
struct HydroAdaptiveCropData;

#include "Hydruino.h"
#include "HydroReservoirs.h"

// Creates crop object from passed crop data (return ownership transfer - user code *must* delete returned object)
extern HydroCrop *newCropObjectFromData(const HydroCropData *dataIn);


// Hydroponic Crop Base
// This is the base class for all crops, which defines how the crop is identified, at
// what point it is in the growth cycle, which sensors are attached to it, what reservoir
// feeds it, etc.
class HydroCrop : public HydroObject, public HydroCropObjectInterface, public HydroFeedReservoirAttachmentInterface {
public:
    const enum : signed char { Timed, Adaptive, Unknown = -1 } classType; // Crop class type (custom RTTI)
    inline bool isTimedClass() const { return classType == Timed; }
    inline bool isAdaptiveClass() const { return classType == Adaptive; }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    HydroCrop(Hydro_CropType cropType,
              hposi_t cropIndex,
              Hydro_SubstrateType substrateType,
              DateTime sowTime,
              int classType = Unknown);
    HydroCrop(const HydroCropData *dataIn);
    virtual ~HydroCrop();

    virtual void update() override;
    virtual void handleLowMemory() override;

    virtual bool needsFeeding(bool poll = false) = 0;
    virtual void notifyFeedingBegan() override;
    virtual void notifyFeedingEnded() override;

    virtual HydroAttachment &getFeedReservoirAttachment() override;

    void setFeedingWeight(float weight);
    inline float getFeedingWeight() const { return _feedingWeight; }

    inline Hydro_CropType getCropType() const { return _id.objTypeAs.cropType; }
    inline hposi_t getCropIndex() const { return _id.posIndex; }
    inline Hydro_SubstrateType getSubstrateType() const { return _substrateType; }
    inline DateTime getSowTime() const { return localTime(_sowTime); }

    inline const HydroCropsLibData *getCropsLibData() const { return _cropsData; }
    inline int getGrowWeek() const { return _growWeek; }
    inline int getTotalGrowWeeks() const { return _totalGrowWeeks; }
    inline Hydro_CropPhase getCropPhase() const { return _cropPhase; }

    Signal<HydroCrop *, HYDRO_FEEDING_SIGNAL_SLOTS> &getFeedingSignal();

    inline void notifyDayChanged() { recalcGrowthParams(); }

protected:
    Hydro_SubstrateType _substrateType;                     // Substrate type (fixme: currently unused)
    time_t _sowTime;                                        // Sow date (UTC), may be past or future date
    HydroAttachment _feedReservoir;                         // Feed reservoir attachment
    const HydroCropsLibData *_cropsData;                    // Crops library data (checked out if !nullptr)
    hposi_t _growWeek;                                      // Current grow week
    hposi_t _totalGrowWeeks;                                // Cached copy of total grow weeks
    Hydro_CropPhase _cropPhase;                             // Current crop phase
    Hydro_TriggerState _feedingState;                       // Feeding state (last handled)
    float _feedingWeight;                                   // Feeding weight (if used, default: 1)
    Signal<HydroCrop *, HYDRO_FEEDING_SIGNAL_SLOTS> _feedingSignal; // Feeding requested signal

    virtual HydroData *allocateData() const override;
    virtual void saveToData(HydroData *dataOut) override;

    void handleFeeding(Hydro_TriggerState feedingState);
    friend class HydroAdaptiveCrop;

    void recalcGrowthParams();
    void checkoutCropsLibData();
    void returnCropsLibData();
    friend class HydroCropsLibrary;

    void handleCustomCropUpdated(Hydro_CropType cropType);
};


// Simple Timed Crop
// Standard crop object that feeds itself based on a time on/time off watering schedule.
// Further limitations on watering, etc., can be set through the scheduler.
class HydroTimedCrop : public HydroCrop {
public:
    HydroTimedCrop(Hydro_CropType cropType,
                   hposi_t cropIndex,
                   Hydro_SubstrateType substrateType,
                   DateTime sowTime,
                   TimeSpan timeOn = TimeSpan(0,0,15,0), TimeSpan timeOff = TimeSpan(0,0,45,0),
                   int classType = Timed);
    HydroTimedCrop(const HydroTimedCropData *dataIn);

    virtual bool needsFeeding(bool poll = false) override;
    virtual void notifyFeedingBegan() override;

    void setFeedTimeOn(TimeSpan timeOn);
    inline TimeSpan getFeedTimeOn() const { return TimeSpan(_feedTimingMins[0] * SECS_PER_MIN); }

    void setFeedTimeOff(TimeSpan timeOff);
    inline TimeSpan getFeedTimeOff() const { return TimeSpan(_feedTimingMins[1] * SECS_PER_MIN); }

protected:
    time_t _lastFeedingTime;                                // Last feeding date (UTC)
    uint8_t _feedTimingMins[2];                             // Feed timing (on/off), in minutes

    virtual void saveToData(HydroData *dataOut) override;
};


// Adaptive Sensing Crop
// Crop type that can manage feedings based on sensor readings of the nearby soil.
class HydroAdaptiveCrop : public HydroCrop, HydroConcentrateUnitsInterfaceStorage, public HydroSoilMoistureSensorAttachmentInterface, public HydroFeedingTriggerAttachmentInterface {
public:
    HydroAdaptiveCrop(Hydro_CropType cropType,
                      hposi_t cropIndex,
                      Hydro_SubstrateType substrateType,
                      DateTime sowTime,
                      int classType = Adaptive);
    HydroAdaptiveCrop(const HydroAdaptiveCropData *dataIn);

    virtual void update() override;
    virtual void handleLowMemory() override;

    virtual bool needsFeeding(bool poll = false) override;

    virtual void setConcentrateUnits(Hydro_UnitsType concentrateUnits) override;

    virtual HydroSensorAttachment &getSoilMoistureSensorAttachment() override;

    virtual HydroTriggerAttachment &getFeedingAttachment() override;

protected:
    HydroSensorAttachment _soilMoisture;                    // Soil moisture sensor attachment
    HydroTriggerAttachment _feedingTrigger;                 // Feeding trigger attachment

    virtual void saveToData(HydroData *dataOut) override;
};


// Crop Serialization Data
struct HydroCropData : public HydroObjectData
{
    Hydro_SubstrateType substrateType;                      // Substrate type
    time_t sowTime;                                         // Sow date (UTC)
    char feedReservoir[HYDRO_NAME_MAXSIZE];                 // Feed reservoir
    float feedingWeight;                                    // Feeding weight

    HydroCropData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Timed Crop Serialization Data
struct HydroTimedCropData : public HydroCropData
{
    time_t lastFeedingTime;                                 // Last feeding (UTC)
    uint8_t feedTimingMins[2];                              // Timing minutes (on/off)

    HydroTimedCropData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Adaptive Crop Serialization Data
struct HydroAdaptiveCropData : public HydroCropData
{
    Hydro_UnitsType concentrateUnits;                       // Concentration units
    char moistureSensor[HYDRO_NAME_MAXSIZE];                // Soil moisture sensor
    HydroTriggerSubData feedingTrigger;                     // Feeding trigger

    HydroAdaptiveCropData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroCrops_H
