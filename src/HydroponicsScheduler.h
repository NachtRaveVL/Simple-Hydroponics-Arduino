
/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Scheduler
*/

#ifndef HydroponicsScheduler_H
#define HydroponicsScheduler_H

class HydroponicsScheduler;
struct HydroponicsSchedulerSubData;
struct HydroponicsFeeding;
struct HydroponicsLighting;

#include "Hydroponics.h"
#include "HydroponicsReservoirs.h"

// Hydroponics Scheduler
class HydroponicsScheduler : public HydroponicsSubObject {
public:
    HydroponicsScheduler();
    virtual ~HydroponicsScheduler();
    void initFromData(HydroponicsSchedulerSubData *dataIn);

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    void setupWaterPHBalancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *waterPHBalancer);
    void setupWaterTDSBalancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *waterTDSBalancer);
    void setupWaterTempBalancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *waterTempBalancer);
    void setupAirTempBalancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *airTempBalancer);
    void setupAirCO2Balancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *airCO2Balancer);

    void setBaseFeedMultiplier(float feedMultiplier);
    void setWeeklyDosingRate(int weekIndex, float dosingRate, Hydroponics_ReservoirType reservoirType = Hydroponics_ReservoirType_NutrientPremix);
    void setStandardDosingRate(float dosingRate, Hydroponics_ReservoirType reservoirType);
    void setLastWeekAsFlush(Hydroponics_CropType cropType);
    void setLastWeekAsFlush(HydroponicsCrop *crop);
    void setFlushWeek(int weekIndex);
    void setTotalFeedingsDay(unsigned int feedingsDay);
    void setPreFeedAeratorMins(unsigned int aeratorMins);
    void setPreLightSprayMins(unsigned int sprayMins);

    void setNeedsScheduling();

    float getCombinedDosingRate(HydroponicsReservoir *reservoir, Hydroponics_ReservoirType reservoirType = Hydroponics_ReservoirType_NutrientPremix);

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
    bool _needsScheduling;                                  // Needs rescheduling tracking flag
    int _lastDayNum;                                        // Last day number tracking for daily rescheduling
    arx::map<Hydroponics_KeyType, HydroponicsFeeding *> _feedings; // Feedings in progress
    arx::map<Hydroponics_KeyType, HydroponicsLighting *> _lightings; // Lightings in progress

    friend class Hydroponics;

    void performScheduling();
    void broadcastDayChange() const;
};


// Hydroponics Scheduler Feeding Stage Tracking
struct HydroponicsFeeding {
    shared_ptr<HydroponicsFeedReservoir> feedRes;
    enum {Init,TopOff,PreFeed,Feed,Drain,Done,Unknown = -1} stage;
    arx::vector<shared_ptr<HydroponicsActuator> > actuatorReqs;
    time_t stageStart;
    time_t canFeedAfter;

    float phSetpoint;
    float tdsSetpoint;
    float waterTempSetpoint;
    float airTempSetpoint;
    float co2Setpoint;

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


// Scheduler Serialization Sub Data
// A part of HSYS system data.
struct HydroponicsSchedulerSubData : public HydroponicsSubData {
    float baseFeedMultiplier;                               // Feed aggressiveness base TDS/EC multiplier (applies to *ALL* feeding solutions in use - default: 1)
    float weeklyDosingRates[HYDRUINO_CROP_GROWWEEKS_MAX];   // Nutrient dosing rate percentages (applies to any nutrient premixes in use - default: 1)
    float stdDosingRates[3];                                // Standard dosing rates for fresh water, pH up, and pH down (default: 1,1/2,1/2)
    uint8_t totalFeedingsDay;                               // Total number of feedings per day, if any (else 0 for disable - default: 0)
    uint8_t preFeedAeratorMins;                             // Minimum time to run aerators (if present) before feed pumps turn on, in minutes (default: 30)
    uint8_t preLightSprayMins;                              // Minimum time to run sprayers/sprinklers (if present/needed) before grow lights turn on, in minutes (default: 60)

    HydroponicsSchedulerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef HydroponicsScheduler_H
