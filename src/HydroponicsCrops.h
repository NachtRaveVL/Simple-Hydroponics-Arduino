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

// Creates crop object from passed crop data (return ownership transfer - user code *must* delete returned object)
extern HydroponicsCrop *newCropObjectFromData(const HydroponicsCropData *dataIn);


// Hydroponic Crop Base
// This is the base class for all crops, which defines how the crop is identified, at
// what point it is in the growth cycle, which sensors are attached to it, what reservoir
// feeds it, etc.
class HydroponicsCrop : public HydroponicsObject, public HydroponicsCropObjectInterface, public HydroponicsSensorAttachmentsInterface {
public:
    const enum { Timed, Adaptive, Unknown = -1 } classType; // Crop class type (custom RTTI)
    inline bool isTimedClass() { return classType == Timed; }
    inline bool isAdaptiveClass() { return classType == Adaptive; }
    inline bool isUnknownClass() { return classType <= Unknown; }

    HydroponicsCrop(Hydroponics_CropType cropType,
                    Hydroponics_PositionIndex cropIndex,
                    Hydroponics_SubstrateType substrateType,
                    time_t sowDate,
                    int classType = Unknown);
    HydroponicsCrop(const HydroponicsCropData *dataIn);
    ~HydroponicsCrop();

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    virtual bool getNeedsFeeding() const = 0;

    virtual bool addSensor(HydroponicsSensor *sensor) override;
    virtual bool removeSensor(HydroponicsSensor *sensor) override;
    bool hasSensor(HydroponicsSensor *sensor) const override;
    arx::map<Hydroponics_KeyType, HydroponicsSensor *> getSensors() const override;

    void setFeedReservoir(HydroponicsIdentity reservoirId);
    void setFeedReservoir(shared_ptr<HydroponicsReservoir> reservoir);
    shared_ptr<HydroponicsReservoir> getFeedReservoir();

    Hydroponics_CropType getCropType() const;
    Hydroponics_PositionIndex getCropIndex() const;
    Hydroponics_SubstrateType getSubstrateType() const;
    time_t getSowDate() const;

    const HydroponicsCropsLibData *getCropsLibData() const;
    int getGrowWeek() const;
    Hydroponics_CropPhase getCropPhase() const;

    Signal<HydroponicsCrop *> &getFeedingSignal();

protected:
    Hydroponics_SubstrateType _substrateType;               // Substrate type
    time_t _sowDate;                                        // Sow date

    const HydroponicsCropsLibData *_cropsData;              // Crops library data (checked out iff !nullptr)
    int _growWeek;                                          // Current grow week
    Hydroponics_CropPhase _cropPhase;                       // Current crop phase
    Hydroponics_TriggerState _feedingState;                 // Current feeding signal state

    arx::map<Hydroponics_KeyType, HydroponicsSensor *> _sensors; // Attached crop sensors (weak)
    HydroponicsDLinkObject<HydroponicsReservoir> _feedReservoir; // Feed reservoir linkage
    Signal<HydroponicsCrop *> _feedingSignal;               // Feeding requested signal

    HydroponicsData *allocateData() const override;
    virtual void saveToData(HydroponicsData *dataOut) const override;

    void recalcGrowWeekAndPhase();
    void checkoutCropsLibData();
    void returnCropsLibData();
    virtual void handleFeedingState();
};


// Simple Timed Crop
// Standard crop object that feeds itself based on a time on/time off watering schedule.
class HydroponicsTimedCrop : public HydroponicsCrop {
public:
    HydroponicsTimedCrop(Hydroponics_CropType cropType,
                          Hydroponics_PositionIndex cropIndex,
                          Hydroponics_SubstrateType substrateType,
                          time_t sowDate,
                          TimeSpan timeOn = TimeSpan(0,0,15,0), TimeSpan timeOff = TimeSpan(0,0,45,0),
                          int classType = Timed);
    HydroponicsTimedCrop(const HydroponicsTimedCropData *dataIn);
    virtual ~HydroponicsTimedCrop();

    bool getNeedsFeeding() const override;

    void setFeedTimeOn(TimeSpan timeOn);
    TimeSpan getFeedTimeOn() const;

    void setFeedTimeOff(TimeSpan timeOff);
    TimeSpan getFeedTimeOff() const;

protected:
    time_t _lastFeedingTime;                                // Last feeding time
    byte _feedTimingMins[2];                                // Feed timing (on/off), in minutes

    void saveToData(HydroponicsData *dataOut) const override;

    void handleFeedingState() override;
};

// Adaptive Sensing Crop
// Crop type that can manage feedings based on sensor readings of the nearby soil.
class HydroponicsAdaptiveCrop : public HydroponicsCrop {
public:
    HydroponicsAdaptiveCrop(Hydroponics_CropType cropType,
                           Hydroponics_PositionIndex cropIndex,
                           Hydroponics_SubstrateType substrateType,
                           time_t sowDate,
                           int classType = Adaptive);
    HydroponicsAdaptiveCrop(const HydroponicsAdaptiveCropData *dataIn);
    virtual ~HydroponicsAdaptiveCrop();

    void update() override;
    void resolveLinks() override;
    void handleLowMemory() override;

    bool getNeedsFeeding() const override;

    void setFeedingTrigger(HydroponicsTrigger *feedingTrigger);
    const HydroponicsTrigger *getFeedingTrigger() const;

protected:
    HydroponicsTrigger *_feedingTrigger;                    // Feeding trigger (owned)

    void saveToData(HydroponicsData *dataOut) const override;
};


// Crop Serialization Data
struct HydroponicsCropData : public HydroponicsObjectData
{
    Hydroponics_SubstrateType substrateType;
    time_t sowDate;
    char feedReservoirName[HYDRUINO_NAME_MAXSIZE];

    HydroponicsCropData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Timed Crop Serialization Data
struct HydroponicsTimedCropData : public HydroponicsCropData
{
    time_t lastFeedingTime;
    byte feedTimingMins[2];

    HydroponicsTimedCropData();
    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Adaptive Crop Serialization Data
struct HydroponicsAdaptiveCropData : public HydroponicsCropData
{
    HydroponicsTriggerSubData feedingTrigger;

    HydroponicsAdaptiveCropData();
    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;

};

#endif // /ifndef HydroponicsCrops_H
