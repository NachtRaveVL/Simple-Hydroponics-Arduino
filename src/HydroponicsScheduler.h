
/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Scheduler
*/

#ifndef HydroponicsScheduler_H
#define HydroponicsScheduler_H

class HydroponicsScheduler;
struct HydroponicsSchedulerSubData;

#include "Hydroponics.h"
#include "HydroponicsReservoirs.h"

// Hydroponics Scheduler Feeding Stage Tracking
struct HydroponicsFeeding {
    shared_ptr<HydroponicsFeedReservoir> feedRes;
    enum {Init,TopOff,PreFeed,Feed,Drain,Done,Unknown = -1} stage;
    arx::vector<shared_ptr<HydroponicsActuator> > actuatorReqs;
    time_t stageStart;
    time_t canFeedAfter;

    float phSetpoint;
    float tdsSetpoint;
    float tempSetpoint;

    HydroponicsFeeding(shared_ptr<HydroponicsFeedReservoir> feedRes);
    ~HydroponicsFeeding();
    void clearActReqs();
    void recalcFeeding();
    void setupStaging();
    void update();
    inline bool isDone() const { return stage == Done; }

private:
    void reset();
    void broadcastFeedingBegan();
    void broadcastFeedingEnded();
};

// Hydroponics Scheduler Lighting Stage Tracking
struct HydroponicsLighting {
    shared_ptr<HydroponicsFeedReservoir> feedRes;
    enum {Init,Spray,Light,Done,Unknown = -1} stage;
    arx::vector<shared_ptr<HydroponicsActuator> > actuatorReqs;

    time_t sprayStart;
    time_t lightStart;
    time_t lightEnd;

    HydroponicsLighting(shared_ptr<HydroponicsFeedReservoir> feedRes);
    ~HydroponicsLighting();
    void clearActReqs();
    void recalcLighting();
    void setupStaging();
    void update();
    inline bool isDone() const { return stage == Done; }
};

// Hydroponics Scheduler
class HydroponicsScheduler : public HydroponicsSubObject {
public:
    HydroponicsScheduler();
    virtual ~HydroponicsScheduler();
    void initFromData(HydroponicsSchedulerSubData *dataIn);

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    void setBaseFeedMultiplier(float feedMultiplier);
    void setWeeklyDosingRate(int weekIndex, float dosingRate, Hydroponics_ReservoirType reservoirType = Hydroponics_ReservoirType_NutrientPremix);
    void setStandardDosingRate(float dosingRate, Hydroponics_ReservoirType reservoirType);
    void setLastWeekAsFlush(Hydroponics_CropType cropType);
    void setLastWeekAsFlush(HydroponicsCrop *crop);
    void setFlushWeek(int weekIndex);
    void setTotalFeedingsDay(unsigned int feedingsDay);
    void setPreFeedAeratorMins(unsigned int aeratorMins);
    void setPreLightSprayMins(unsigned int sprayMins);

    void setNeedsRescheduling();

    float getCombinedDosingRate(HydroponicsFeedReservoir *feedReservoir, Hydroponics_ReservoirType reservoirType = Hydroponics_ReservoirType_NutrientPremix);

    float getBaseFeedMultiplier() const;
    float getWeeklyDosingRate(int weekIndex, Hydroponics_ReservoirType reservoirType = Hydroponics_ReservoirType_NutrientPremix) const;
    float getStandardDosingRate(Hydroponics_ReservoirType reservoirType) const;
    bool getIsFlushWeek(int weekIndex);
    unsigned int getTotalFeedingsDay() const;
    unsigned int getPreFeedAeratorMins() const;
    unsigned int getPreLightSprayMins() const;
    bool getInDaytimeMode() const;

protected:
    HydroponicsSchedulerSubData *_schedulerData;            // Scheduler data (strong, saved to storage via system data)

    bool _inDaytimeMode;                                    // Whenever in daytime feeding mode or not
    bool _needsRescheduling;                                // Needs rescheduling tracking flag
    int _lastDayNum;                                        // Last day number tracking for daily rescheduling
    arx::map<Hydroponics_KeyType, HydroponicsFeeding *> _feedings; // Feedings in progress
    arx::map<Hydroponics_KeyType, HydroponicsLighting *> _lightings; // Lightings in progress

    friend class Hydroponics;

    void performScheduling();
    void broadcastDayChange() const;
};

// Scheduler Serialization Sub Data
// A part of HSYS system data.
struct HydroponicsSchedulerSubData : public HydroponicsSubData {
    float baseFeedMultiplier;                               // Feed aggressiveness base TDS/EC multiplier (applies to *ALL* feeding solutions in use - default: 1)
    float weeklyDosingRates[HYDRUINO_CROP_GROWEEKS_MAX];    // Nutrient dosing rate percentages (applies to any nutrient premixes in use - default: 1)
    float standardDosingRates[3];                           // Standard dosing rates for fresh water, pH up, and pH down (default: 1,1,1)
    uint8_t totalFeedingsDay;                               // Total number of feedings per day, if any (else 0 for disable - default: 0)
    uint8_t preFeedAeratorMins;                             // Minimum time to run aerators (if present) before feed pumps turn on, in minutes (default: 30)
    uint8_t preLightSprayMins;                              // Minimum time to run sprayers/sprinklers (if present/needed) before grow lights turn on, in minutes (default: 60)

    HydroponicsSchedulerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef HydroponicsScheduler_H
