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
    bool hasSensor(HydroponicsSensor *sensor) const override;
    arx::map<Hydroponics_KeyType, HydroponicsObject *, HYDRUINO_OBJ_LINKS_MAXSIZE> getSensors() const override;

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
    time_t _sowDate;                                        // Sow date

    const HydroponicsCropsLibData *_cropsData;              // Crops library data (checked out iff !nullptr)
    int _growWeek;                                          // Current grow week
    int _totalGrowWeeks;                                    // Total grow weeks (cached)
    Hydroponics_CropPhase _cropPhase;                       // Current crop phase
    Hydroponics_TriggerState _feedingState;                 // Current feeding signal state
    float _feedingWeight;                                   // Feeding weight (if used, default: 1)

    arx::map<Hydroponics_KeyType, HydroponicsSensor *> _sensors; // Attached crop sensors (weak)
    HydroponicsDLinkObject<HydroponicsFeedReservoir> _feedReservoir; // Feed reservoir linkage
    Signal<HydroponicsCrop *> _feedingSignal;               // Feeding requested signal

    HydroponicsData *allocateData() const override;
    virtual void saveToData(HydroponicsData *dataOut) const override;

    void recalcGrowWeekAndPhase();
    void checkoutCropsLibData();
    void returnCropsLibData();
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

    bool getNeedsFeeding() const override;
    void notifyFeedingBegan() override;

    void setFeedTimeOn(TimeSpan timeOn);
    TimeSpan getFeedTimeOn() const;

    void setFeedTimeOff(TimeSpan timeOff);
    TimeSpan getFeedTimeOff() const;

protected:
    time_t _lastFeedingTime;                                // Last feeding time
    byte _feedTimingMins[2];                                // Feed timing (on/off), in minutes

    void saveToData(HydroponicsData *dataOut) const override;
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

    void update() override;
    void resolveLinks() override;
    void handleLowMemory() override;

    bool getNeedsFeeding() const override;

    void setMoistureUnits(Hydroponics_UnitsType moistureUnits);
    Hydroponics_UnitsType getMoistureUnits() const;

    void setMoistureSensor(HydroponicsIdentity moistureSensorId) override;
    void setMoistureSensor(shared_ptr<HydroponicsSensor> moistureSensor) override;
    shared_ptr<HydroponicsSensor> getMoistureSensor() override;

    void setSoilMoisture(float soilMoisture, Hydroponics_UnitsType soilMoistureUnits = Hydroponics_UnitsType_Undefined) override;
    void setSoilMoisture(HydroponicsSingleMeasurement soilMoisture) override;
    const HydroponicsSingleMeasurement &getSoilMoisture() const override;

    void setFeedingTrigger(HydroponicsTrigger *feedingTrigger);
    const HydroponicsTrigger *getFeedingTrigger() const;

protected:
    Hydroponics_UnitsType _moistureUnits;                   // Moisture units preferred (else default)
    HydroponicsDLinkObject<HydroponicsSensor> _moistureSensor; // Soil moisture sensor
    HydroponicsSingleMeasurement _soilMoisture;             // Soil moisture measurement
    HydroponicsTrigger *_feedingTrigger;                    // Feeding trigger (owned)

    void saveToData(HydroponicsData *dataOut) const override;

    void attachSoilMoistureSensor();
    void detachSoilMoistureSensor();
    void handleSoilMoistureMeasure(const HydroponicsMeasurement *measurement);
};


// Crop Serialization Data
struct HydroponicsCropData : public HydroponicsObjectData
{
    Hydroponics_SubstrateType substrateType;
    time_t sowDate;
    char feedReservoirName[HYDRUINO_NAME_MAXSIZE];
    float feedingWeight;

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
    Hydroponics_UnitsType moistureUnits;
    char moistureSensorName[HYDRUINO_NAME_MAXSIZE];
    HydroponicsTriggerSubData feedingTrigger;

    HydroponicsAdaptiveCropData();
    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;

};

#endif // /ifndef HydroponicsCrops_H
