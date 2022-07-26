/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Crops
*/

#ifndef HydroponicsCrops_H
#define HydroponicsCrops_H

class HydroponicsCrop;
class HydroponicsTimedCrop;
class HydroponicsAdaptiveCrop;

struct HydroponicsCropData;
struct HydroponicsTimedCropData;
struct HydroponicsAdaptiveCropData;

#include "Hydroponics.h"
#include "HydroponicsReservoirs.h"

// Creates crop object from passed crop data (return ownership transfer - user code *must* delete returned object)
extern HydroponicsCrop *newCropObjectFromData(const HydroponicsCropData *dataIn);


// Hydroponic Crop Base
// This is the base class for all crops, which defines how the crop is identified, at
// what point it is in the growth cycle, which sensors are attached to it, what reservoir
// feeds it, etc.
class HydroponicsCrop : public HydroponicsObject, public HydroponicsCropObjectInterface, public HydroponicsFeedReservoirAttachmentInterface {
public:
    const enum { Timed, Adaptive, Unknown = -1 } classType; // Crop class type (custom RTTI)
    inline bool isTimedClass() const { return classType == Timed; }
    inline bool isAdaptiveClass() const { return classType == Adaptive; }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    HydroponicsCrop(Hydroponics_CropType cropType,
                    Hydroponics_PositionIndex cropIndex,
                    Hydroponics_SubstrateType substrateType,
                    DateTime sowDate,
                    int classType = Unknown);
    HydroponicsCrop(const HydroponicsCropData *dataIn);
    ~HydroponicsCrop();

    virtual void update() override;
    virtual void handleLowMemory() override;

    virtual bool needsFeeding() const = 0;
    virtual void notifyFeedingBegan() override;
    virtual void notifyFeedingEnded() override;

    virtual HydroponicsAttachment<HydroponicsFeedReservoir> &getFeedingReservoir() override;

    void setFeedingWeight(float weight);
    inline float getFeedingWeight() const { return _feedingWeight; }

    inline Hydroponics_CropType getCropType() const { return _id.objTypeAs.cropType; }
    inline Hydroponics_PositionIndex getCropIndex() const { return _id.posIndex; }
    inline Hydroponics_SubstrateType getSubstrateType() const { return _substrateType; }
    inline DateTime getSowDate() const { return DateTime((uint32_t)_sowDate); }

    inline const HydroponicsCropsLibData *getCropsLibData() const { return _cropsData; }
    inline int getGrowWeek() const { return _growWeek; }
    inline int getTotalGrowWeeks() const { return _totalGrowWeeks; }
    inline Hydroponics_CropPhase getCropPhase() const { return _cropPhase; }

    Signal<HydroponicsCrop *> &getFeedingSignal();

    void notifyDayChanged();

protected:
    Hydroponics_SubstrateType _substrateType;               // Substrate type
    time_t _sowDate;                                        // Sow date (UTC)
    HydroponicsAttachment<HydroponicsFeedReservoir> _feedReservoir; // Feed reservoir attachment

    const HydroponicsCropsLibData *_cropsData;              // Crops library data (checked out iff !nullptr)
    int _growWeek;                                          // Current grow week
    int _totalGrowWeeks;                                    // Total grow weeks (cached)
    Hydroponics_CropPhase _cropPhase;                       // Current crop phase
    Hydroponics_TriggerState _feedingState;                 // Current feeding signal state
    float _feedingWeight;                                   // Feeding weight (if used, default: 1)

    
    Signal<HydroponicsCrop *> _feedingSignal;               // Feeding requested signal

    virtual HydroponicsData *allocateData() const override;
    virtual void saveToData(HydroponicsData *dataOut) override;

    virtual void handleFeedingState();

    void recalcGrowWeekAndPhase();
    void checkoutCropsLibData();
    void returnCropsLibData();

    void handleCustomCropUpdated(Hydroponics_CropType cropType);
};


// Simple Timed Crop
// Standard crop object that feeds itself based on a time on/time off watering schedule.
// Further limitations on watering, etc., can be set through the scheduler.
class HydroponicsTimedCrop : public HydroponicsCrop {
public:
    HydroponicsTimedCrop(Hydroponics_CropType cropType,
                         Hydroponics_PositionIndex cropIndex,
                         Hydroponics_SubstrateType substrateType,
                         DateTime sowDate,
                         TimeSpan timeOn = TimeSpan(0,0,15,0), TimeSpan timeOff = TimeSpan(0,0,45,0),
                         int classType = Timed);
    HydroponicsTimedCrop(const HydroponicsTimedCropData *dataIn);
    virtual ~HydroponicsTimedCrop();

    virtual bool needsFeeding() const override;
    virtual void notifyFeedingBegan() override;

    void setFeedTimeOn(TimeSpan timeOn);
    inline TimeSpan getFeedTimeOn() const { return TimeSpan(_feedTimingMins[0] * SECS_PER_MIN); }

    void setFeedTimeOff(TimeSpan timeOff);
    inline TimeSpan getFeedTimeOff() const { return TimeSpan(_feedTimingMins[1] * SECS_PER_MIN); }

protected:
    time_t _lastFeedingDate;                                // Last feeding date (UTC)
    byte _feedTimingMins[2];                                // Feed timing (on/off), in minutes

    virtual void saveToData(HydroponicsData *dataOut) override;
};

// Adaptive Sensing Crop
// Crop type that can manage feedings based on sensor readings of the nearby soil.
class HydroponicsAdaptiveCrop : public HydroponicsCrop, public HydroponicsSoilMoistureSensorAttachmentInterface {
public:
    HydroponicsAdaptiveCrop(Hydroponics_CropType cropType,
                            Hydroponics_PositionIndex cropIndex,
                            Hydroponics_SubstrateType substrateType,
                            DateTime sowDate,
                            int classType = Adaptive);
    HydroponicsAdaptiveCrop(const HydroponicsAdaptiveCropData *dataIn);
    virtual ~HydroponicsAdaptiveCrop();

    virtual void update() override;
    virtual void handleLowMemory() override;

    virtual bool needsFeeding() const override;

    void setMoistureUnits(Hydroponics_UnitsType moistureUnits);
    Hydroponics_UnitsType getMoistureUnits() const;

    virtual HydroponicsSensorAttachment &getSoilMoisture() override;

    void setFeedingTrigger(HydroponicsTrigger *feedingTrigger);
    inline const HydroponicsTrigger *getFeedingTrigger() const { return _feedingTrigger; }

protected:
    Hydroponics_UnitsType _moistureUnits;                   // Moisture units preferred (else default)
    HydroponicsSensorAttachment _soilMoisture;              // Soil moisture sensor attachment
    HydroponicsTrigger *_feedingTrigger;                    // Feeding trigger (owned)

    virtual void saveToData(HydroponicsData *dataOut) override;

    void attachFeedingTrigger();
    void detachFeedingTrigger();
    void handleFeedingTrigger(Hydroponics_TriggerState triggerState);
};


// Crop Serialization Data
struct HydroponicsCropData : public HydroponicsObjectData
{
    Hydroponics_SubstrateType substrateType;
    time_t sowDate;
    char feedReservoir[HYDRUINO_NAME_MAXSIZE];
    float feedingWeight;

    HydroponicsCropData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Timed Crop Serialization Data
struct HydroponicsTimedCropData : public HydroponicsCropData
{
    time_t lastFeedingDate;
    byte feedTimingMins[2];

    HydroponicsTimedCropData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Adaptive Crop Serialization Data
struct HydroponicsAdaptiveCropData : public HydroponicsCropData
{
    Hydroponics_UnitsType moistureUnits;
    char moistureSensor[HYDRUINO_NAME_MAXSIZE];
    HydroponicsTriggerSubData feedingTrigger;

    HydroponicsAdaptiveCropData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroponicsCrops_H
