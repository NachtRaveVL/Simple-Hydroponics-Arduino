
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
#include "HydroponicsCrops.h"

// Hydroponics Scheduler
class HydroponicsScheduler {
public:
    HydroponicsScheduler();
    ~HydroponicsScheduler();
    void initFromData(HydroponicsSchedulerSubData *dataIn);

    void update();
    void handleLowMemory();

    void setupWaterPHBalancer(HydroponicsReservoir *reservoir, shared_ptr<HydroponicsBalancer> waterPHBalancer);
    void setupWaterTDSBalancer(HydroponicsReservoir *reservoir, shared_ptr<HydroponicsBalancer> waterTDSBalancer);
    void setupWaterTemperatureBalancer(HydroponicsReservoir *reservoir, shared_ptr<HydroponicsBalancer> waterTempBalancer);
    void setupAirTemperatureBalancer(HydroponicsReservoir *reservoir, shared_ptr<HydroponicsBalancer> airTempBalancer);
    void setupAirCO2Balancer(HydroponicsReservoir *reservoir, shared_ptr<HydroponicsBalancer> airCO2Balancer);

    void setBaseFeedMultiplier(float feedMultiplier);
    void setWeeklyDosingRate(int weekIndex, float dosingRate, Hydroponics_ReservoirType reservoirType = Hydroponics_ReservoirType_NutrientPremix);
    void setStandardDosingRate(float dosingRate, Hydroponics_ReservoirType reservoirType);
    void setLastWeekAsFlush(Hydroponics_CropType cropType);
    inline void setLastWeekAsFlush(HydroponicsCrop *crop) { if (crop) { setFlushWeek(crop->getTotalGrowWeeks() - 1); } }
    void setFlushWeek(int weekIndex);
    void setTotalFeedingsDay(unsigned int feedingsDay);
    void setPreFeedAeratorMins(unsigned int aeratorMins);
    void setPreLightSprayMins(unsigned int sprayMins);
    void setAirReportInterval(TimeSpan interval);

    inline void setNeedsScheduling() { _needsScheduling = _schedulerData; }
    inline bool needsScheduling() { return _needsScheduling; }

    float getCombinedDosingRate(HydroponicsReservoir *reservoir, Hydroponics_ReservoirType reservoirType = Hydroponics_ReservoirType_NutrientPremix);
    inline bool inDaytimeMode() const { return _inDaytimeMode; }

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


// Hydroponics Scheduler Process Base
// Processes are created and managed by Scheduler to manage the feeding and lighting
// necessary for crops to grow.
struct HydroponicsProcess {
    shared_ptr<HydroponicsFeedReservoir> feedRes;           // Feed reservoir
    Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type actuatorReqs; // Actuators required for this stage (keep-enabled list)

    time_t stageStart;                                      // Stage start time

    HydroponicsProcess(shared_ptr<HydroponicsFeedReservoir> feedRes);

    void clearActuatorReqs();
    void setActuatorReqs(const Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type &actuatorReqsIn);
};

// Hydroponics Scheduler Feeding Process Log Type
enum HydroponicsFeedingLogType : char {
    HydroponicsFeedingLogType_WaterSetpoints,               // Water setpoints
    HydroponicsFeedingLogType_WaterMeasures,                // Water measurements
    HydroponicsFeedingLogType_AirSetpoints,                 // Air setpoints
    HydroponicsFeedingLogType_AirMeasures                   // Air measurements
};

// Hydroponics Scheduler Feeding Process Broadcast Type
enum HydroponicsFeedingBroadcastType : char {
    HydroponicsFeedingBroadcastType_Began,                  // Began main process
    HydroponicsFeedingBroadcastType_Ended                   // Ended main process
};

// Hydroponics Scheduler Feeding Process
struct HydroponicsFeeding : public HydroponicsProcess {
    enum : char {Init,TopOff,PreFeed,Feed,Drain,Done,Unknown = -1} stage; // Current feeding stage

    time_t canFeedAfter;                                    // Time next feeding can occur (UTC)
    time_t lastAirReport;                                   // Last time an air report was generated (UTC)

    float phSetpoint;                                       // Calculated pH setpoint for detected crops
    float tdsSetpoint;                                      // Calculated TDS setpoint for detected crops
    float waterTempSetpoint;                                // Calculated water temp setpoint for detected crops
    float airTempSetpoint;                                  // Calculated air temp setpoint for detected crops
    float co2Setpoint;                                      // Calculated co2 level setpoint for detected crops

    HydroponicsFeeding(shared_ptr<HydroponicsFeedReservoir> feedRes);
    ~HydroponicsFeeding();

    void recalcFeeding();
    void setupStaging();
    void update();

private:
    void reset();
    void logFeeding(HydroponicsFeedingLogType logType);
    void broadcastFeeding(HydroponicsFeedingBroadcastType broadcastType);
};

// Hydroponics Scheduler Lighting Process
struct HydroponicsLighting : public HydroponicsProcess {
    enum : char {Init,Spray,Light,Done,Unknown = -1} stage; // Current lighting stage

    time_t sprayStart;                                      // Time when spraying should start (TZ)
    time_t lightStart;                                      // Time when lighting should start / spraying should end (TZ, same as sprayStart when no spraying needed)
    time_t lightEnd;                                        // Time when lighting should finish (TZ)

    float lightHours;                                       // Calculated light hours for detected crops

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
