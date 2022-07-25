
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
    void setAirReportInterval(TimeSpan interval);

    void setNeedsScheduling();

    float getCombinedDosingRate(HydroponicsReservoir *reservoir, Hydroponics_ReservoirType reservoirType = Hydroponics_ReservoirType_NutrientPremix);
    bool inDaytimeMode() const;

    float getBaseFeedMultiplier() const;
    float getWeeklyDosingRate(int weekIndex, Hydroponics_ReservoirType reservoirType = Hydroponics_ReservoirType_NutrientPremix) const;
    float getStandardDosingRate(Hydroponics_ReservoirType reservoirType) const;
    bool isFlushWeek(int weekIndex);
    unsigned int getTotalFeedingsDay() const;
    unsigned int getPreFeedAeratorMins() const;
    unsigned int getPreLightSprayMins() const;
    TimeSpan getAirReportInterval() const;

protected:
    HydroponicsSchedulerSubData *_schedulerData;            // Scheduler data (strong, saved to storage via system data)

    bool _inDaytimeMode;                                    // Whenever in daytime feeding mode or not
    bool _needsScheduling;                                  // Needs rescheduling tracking flag
    int _lastDayNum;                                        // Last day number tracking for daily rescheduling tracking
    Map<Hydroponics_KeyType, HydroponicsFeeding *, HYDRUINO_SCH_FEEDRES_MAXSIZE>::type _feedings; // Feedings in progress
    Map<Hydroponics_KeyType, HydroponicsLighting *, HYDRUINO_SCH_FEEDRES_MAXSIZE>::type _lightings; // Lightings in progress

    friend class Hydroponics;

    void updateDayTracking();
    void performScheduling();
    void broadcastDayChange();
};


// Hydroponics Scheduler Process
struct HydroponicsProcess {
    shared_ptr<HydroponicsFeedReservoir> feedRes;
    Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type actuatorReqs;

    time_t stageStart;

    HydroponicsProcess(shared_ptr<HydroponicsFeedReservoir> feedRes);

    void clearActuatorReqs();
    void setActuatorReqs(const Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type &actuatorReqsIn);
};

// Hydroponics Scheduler Feeding Process
struct HydroponicsFeeding : public HydroponicsProcess {
    enum {Init,TopOff,PreFeed,Feed,Drain,Done,Unknown = -1} stage;

    time_t canFeedAfter;
    time_t lastAirReport;

    float phSetpoint;
    float tdsSetpoint;
    float waterTempSetpoint;
    float airTempSetpoint;
    float co2Setpoint;

    HydroponicsFeeding(shared_ptr<HydroponicsFeedReservoir> feedRes);
    ~HydroponicsFeeding();

    void recalcFeeding();
    void setupStaging();
    void update();

private:
    void reset();
    void setupBalancers();
    void logWaterSetpoints();
    void logWaterMeasures();
    void logAirSetpoints();
    void logAirMeasures();
    void broadcastFeedingBegan();
    void broadcastFeedingEnded();
};

// Hydroponics Scheduler Lighting Process
struct HydroponicsLighting : public HydroponicsProcess {
    enum {Init,Spray,Light,Done,Unknown = -1} stage;

    time_t sprayStart;
    time_t lightStart;
    time_t lightEnd;

    float lightHours;

    HydroponicsLighting(shared_ptr<HydroponicsFeedReservoir> feedRes);
    ~HydroponicsLighting();

    void recalcLighting();
    void setupStaging();
    void update();
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
    time_t airReportInterval;                               // Interval between air sensor reports, in seconds (default: 8hrs)

    HydroponicsSchedulerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef HydroponicsScheduler_H
