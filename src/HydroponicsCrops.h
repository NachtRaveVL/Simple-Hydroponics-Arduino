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
class HydroponicsCrop : public HydroponicsObject, public HydroponicsCropObjectInterface, public HydroponicsSensorAttachmentsInterface {
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
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    virtual bool getNeedsFeeding() const = 0;
    virtual void notifyFeedingBegan() override;
    virtual void notifyFeedingEnded() override;

    virtual bool addSensor(HydroponicsSensor *sensor) override;
    virtual bool removeSensor(HydroponicsSensor *sensor) override;
    virtual bool hasSensor(HydroponicsSensor *sensor) const override;

    void setFeedReservoir(HydroponicsIdentity reservoirId);
    void setFeedReservoir(shared_ptr<HydroponicsFeedReservoir> reservoir);
    shared_ptr<HydroponicsFeedReservoir> getFeedReservoir();

    void setFeedingWeight(float weight);
    float getFeedingWeight() const;

    Hydroponics_CropType getCropType() const;
    Hydroponics_PositionIndex getCropIndex() const;
    Hydroponics_SubstrateType getSubstrateType() const;
    DateTime getSowDate() const;

    const HydroponicsCropsLibData *getCropsLibData() const;
    int getGrowWeek() const;
    int getTotalGrowWeeks() const;
    Hydroponics_CropPhase getCropPhase() const;

    Signal<HydroponicsCrop *> &getFeedingSignal();

    void notifyDayChanged();

protected:
    Hydroponics_SubstrateType _substrateType;               // Substrate type
    time_t _sowDate;                                        // Sow date (UTC)

    const HydroponicsCropsLibData *_cropsData;              // Crops library data (checked out iff !nullptr)
    int _growWeek;                                          // Current grow week
    int _totalGrowWeeks;                                    // Total grow weeks (cached)
    Hydroponics_CropPhase _cropPhase;                       // Current crop phase
    Hydroponics_TriggerState _feedingState;                 // Current feeding signal state
    float _feedingWeight;                                   // Feeding weight (if used, default: 1)

    Map<Hydroponics_KeyType, HydroponicsSensor *>::type _sensors; // Attached crop sensors (weak)
    HydroponicsDLinkObject<HydroponicsFeedReservoir> _feedReservoir; // Feed reservoir linkage
    Signal<HydroponicsCrop *> _feedingSignal;               // Feeding requested signal

    virtual HydroponicsData *allocateData() const override;
    virtual void saveToData(HydroponicsData *dataOut) override;

    virtual void handleFeedingState();

    void recalcGrowWeekAndPhase();
    void checkoutCropsLibData();
    void returnCropsLibData();

    void attachCustomCrop();
    void detachCustomCrop();
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

    virtual bool getNeedsFeeding() const override;
    virtual void notifyFeedingBegan() override;

    void setFeedTimeOn(TimeSpan timeOn);
    TimeSpan getFeedTimeOn() const;

    void setFeedTimeOff(TimeSpan timeOff);
    TimeSpan getFeedTimeOff() const;

protected:
    time_t _lastFeedingDate;                                // Last feeding date (UTC)
    byte _feedTimingMins[2];                                // Feed timing (on/off), in minutes

    virtual void saveToData(HydroponicsData *dataOut) override;
};

// Adaptive Sensing Crop
// Crop type that can manage feedings based on sensor readings of the nearby soil.
class HydroponicsAdaptiveCrop : public HydroponicsCrop, public HydroponicsSoilMoistureAwareInterface {
public:
    HydroponicsAdaptiveCrop(Hydroponics_CropType cropType,
                            Hydroponics_PositionIndex cropIndex,
                            Hydroponics_SubstrateType substrateType,
                            DateTime sowDate,
                            int classType = Adaptive);
    HydroponicsAdaptiveCrop(const HydroponicsAdaptiveCropData *dataIn);
    virtual ~HydroponicsAdaptiveCrop();

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    virtual bool getNeedsFeeding() const override;

    void setMoistureUnits(Hydroponics_UnitsType moistureUnits);
    Hydroponics_UnitsType getMoistureUnits() const;

    virtual void setMoistureSensor(HydroponicsIdentity moistureSensorId) override;
    virtual void setMoistureSensor(shared_ptr<HydroponicsSensor> moistureSensor) override;
    virtual shared_ptr<HydroponicsSensor> getMoistureSensor() override;

    virtual void setSoilMoisture(float soilMoisture, Hydroponics_UnitsType soilMoistureUnits = Hydroponics_UnitsType_Undefined) override;
    virtual void setSoilMoisture(HydroponicsSingleMeasurement soilMoisture) override;
    virtual const HydroponicsSingleMeasurement &getSoilMoisture() override;

    void setFeedingTrigger(HydroponicsTrigger *feedingTrigger);
    const HydroponicsTrigger *getFeedingTrigger() const;

protected:
    Hydroponics_UnitsType _moistureUnits;                   // Moisture units preferred (else default)
    HydroponicsDLinkObject<HydroponicsSensor> _moistureSensor; // Soil moisture sensor
    bool _needsSoilMoisture;                                // Needs soil moisture update tracking flag
    HydroponicsSingleMeasurement _soilMoisture;             // Soil moisture measurement
    HydroponicsTrigger *_feedingTrigger;                    // Feeding trigger (owned)

    virtual void saveToData(HydroponicsData *dataOut) override;

    void attachSoilMoistureSensor();
    void detachSoilMoistureSensor();
    void handleSoilMoistureMeasure(const HydroponicsMeasurement *measurement);
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
