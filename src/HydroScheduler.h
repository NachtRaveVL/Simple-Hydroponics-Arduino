
/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Scheduler
*/

#ifndef HydroScheduler_H
#define HydroScheduler_H

class HydroScheduler;
struct HydroSchedulerSubData;
struct HydroProcess;
struct HydroFeeding;
struct HydroLighting;

#include "Hydruino.h"

// Scheduler
// The Scheduler acts as the system's main scheduling attendant, who looks through all
// the various equipment and crops you have programmed in, and figures out the best
// case feeding and lighting processes that should occur to support them. It is also
// responsible for setting up and maintaining the system balancers that get assigned to
// feed reservoirs (such as the various dosing actuators in use), as well as determining
// when significant time or event changes have occurred and broadcasting such out.
class HydroScheduler {
public:
    HydroScheduler();
    ~HydroScheduler();

    void update();

    inline void setNeedsScheduling() { _needsScheduling = hasSchedulerData(); }
    inline bool needsScheduling() { return _needsScheduling; }
    inline bool inDaytimeMode() const { return _inDaytimeMode; }

    void setupWaterPHBalancer(HydroReservoir *reservoir, SharedPtr<HydroBalancer> waterPHBalancer);
    void setupWaterTDSBalancer(HydroReservoir *reservoir, SharedPtr<HydroBalancer> waterTDSBalancer);
    void setupWaterTemperatureBalancer(HydroReservoir *reservoir, SharedPtr<HydroBalancer> waterTempBalancer);
    void setupAirTemperatureBalancer(HydroReservoir *reservoir, SharedPtr<HydroBalancer> airTempBalancer);
    void setupAirCO2Balancer(HydroReservoir *reservoir, SharedPtr<HydroBalancer> airCO2Balancer);

    void setBaseFeedMultiplier(float feedMultiplier);
    void setWeeklyDosingRate(int weekIndex, float dosingRate, Hydro_ReservoirType reservoirType = Hydro_ReservoirType_NutrientPremix);
    void setStandardDosingRate(float dosingRate, Hydro_ReservoirType reservoirType);
    void setLastWeekAsFlush(Hydro_CropType cropType);
    inline void setLastWeekAsFlush(HydroCrop *crop);
    void setFlushWeek(int weekIndex);
    void setTotalFeedingsPerDay(unsigned int feedingsDay);
    void setPreFeedAeratorMins(unsigned int aeratorMins);
    void setPreDawnSprayMins(unsigned int sprayMins);
    void setAirReportInterval(TimeSpan reportInterval);
    void setUseNaturalLight(bool useNaturalLight, unsigned int twilightOffsetMins = 20);

    float getCombinedDosingRate(HydroReservoir *reservoir, Hydro_ReservoirType reservoirType = Hydro_ReservoirType_NutrientPremix);
    float getBaseFeedMultiplier() const;
    float getWeeklyDosingRate(int weekIndex, Hydro_ReservoirType reservoirType = Hydro_ReservoirType_NutrientPremix) const;
    float getStandardDosingRate(Hydro_ReservoirType reservoirType) const;
    bool isFlushWeek(int weekIndex);
    unsigned int getTotalFeedingsPerDay() const;
    unsigned int getPreFeedAeratorMins() const;
    unsigned int getPreDawnSprayMins() const;
    TimeSpan getAirReportInterval() const;
    int getNaturalLightOffsetMins() const;
    inline bool getUseNaturalLight() const { return getNaturalLightOffsetMins() >= 0; }

    inline const Twilight &getDailyTwilight() const { return _dailyTwilight; }

protected:
    Twilight _dailyTwilight;                                // Daily twilight settings
    bool _needsScheduling;                                  // Needs rescheduling tracking flag
    bool _inDaytimeMode;                                    // Daytime mode flag
    hposi_t _lastDay[3];                                    // Last day tracking for rescheduling (Y-2k,M,D)
    Map<hkey_t, HydroFeeding *, HYDRO_SCH_PROCS_MAXSIZE> _feedings; // Feed reservoir feeding processes
    Map<hkey_t, HydroLighting *, HYDRO_SCH_PROCS_MAXSIZE> _lightings; // Feed reservoir lighting processes

    friend class Hydruino;
    friend struct HydroProcess;
    friend struct HydroFeeding;
    friend struct HydroLighting;

    inline HydroSchedulerSubData *schedulerData() const;
    inline bool hasSchedulerData() const;

    void updateDayTracking();
    void performScheduling();
    void broadcastDayChange();
};


// Scheduler Feeding Process Log Type
enum HydroFeedingLogType : signed char {
    HydroFeedingLogType_WaterReport,                        // Water report
    HydroFeedingLogType_AirReport                           // Air report
};

// Scheduler Feeding Process Broadcast Type
enum HydroFeedingBroadcastType : signed char {
    HydroFeedingBroadcastType_Began,                        // Began main process
    HydroFeedingBroadcastType_Ended                         // Ended main process
};

// Scheduler Process Base
// Processes are created and managed by Scheduler to manage the daily control
// sequences necessary for crops to grow.
struct HydroProcess {
    SharedPtr<HydroFeedReservoir> feedRes;                  // Feed reservoir
    Vector<HydroActuatorAttachment, HYDRO_SCH_REQACTS_MAXSIZE> actuatorReqs; // Actuators required for this stage (keep-enabled list)

    time_t stageStart;                                      // Stage start time

    HydroProcess(SharedPtr<HydroFeedReservoir> feedRes);

    void clearActuatorReqs();
    void setActuatorReqs(const Vector<HydroActuatorAttachment, HYDRO_SCH_REQACTS_MAXSIZE> &actuatorReqsIn);
};

// Scheduler Feeding Process
struct HydroFeeding : public HydroProcess {
    enum : signed char {Init,TopOff,PreFeed,Feed,Drain,Done} stage; // Current feeding stage

    time_t canProcessAfter;                                 // Time next processing can occur (unix/UTC), else 0/disabled
    time_t lastAirReport;                                   // Last time an air report was generated (unix/UTC)

    float phSetpoint;                                       // Calculated pH setpoint for attached crops
    float tdsSetpoint;                                      // Calculated TDS setpoint for attached crops
    float waterTempSetpoint;                                // Calculated water temp setpoint for attached crops
    float airTempSetpoint;                                  // Calculated air temp setpoint for attached crops
    float co2Setpoint;                                      // Calculated co2 level setpoint for attached crops

    HydroFeeding(SharedPtr<HydroFeedReservoir> feedRes);
    ~HydroFeeding();

    void recalcFeeding();
    void setupStaging();
    void update();

private:
    void logFeeding(HydroFeedingLogType logType, bool withSetpoints = true);
    void broadcastFeeding(HydroFeedingBroadcastType broadcastType);
};

// Scheduler Lighting Process
struct HydroLighting : public HydroProcess {
    enum : signed char {Init,Spray,Light,NatLight,Done} stage; // Current lighting stage

    time_t sprayStart;                                      // Time when spraying should start (TZ offset)
    time_t lightStart;                                      // Time when lighting should start / spraying should end (TZ offset, same as sprayStart when no spraying needed)
    time_t lightEnd;                                        // Time when lighting should finish (TZ offset)

    time_t augNatLightCease;                                // Time to stop augmenting due to incoming natural light (TZ offset, same as lightEnd when no natural light augment)
    time_t augNatLightResume;                               // Time to restart augmenting due to outgoing natural light (TZ offset, same as lightEnd when no natural light augment)
    time_t lightTimeOffset;                                 // Used to augment lighting sequence's final elapsed time (saved copy of elapsed before augmented natlight switch)

    float lightHours;                                       // Calculated light hours for attached crops

    HydroLighting(SharedPtr<HydroFeedReservoir> feedRes);
    ~HydroLighting();

    void recalcLighting();
    void setupStaging();
    void update();
};


// Scheduler Serialization Sub Data
// A part of HSYS system data.
struct HydroSchedulerSubData : public HydroSubData {
    float baseFeedMultiplier;                               // Feed aggressiveness base TDS/EC multiplier (applies to *ALL* feeding solutions in use - default: 1)
    float weeklyDosingRates[HYDRO_CROPS_GROWWEEKS_MAX];     // Nutrient dosing rate percentages (applies to any nutrient premixes in use - default: 1)
    float stdDosingRates[3];                                // Standard dosing rates for fresh water, pH up, and pH down (default: 1,1/2,1/2)
    uint8_t totalFeedingsPerDay;                            // Total number of feedings per day, if any (else 0 for disable - default: 0)
    uint8_t preFeedAeratorMins;                             // Time to run aerators (if present) before feed pumps turn on, in minutes (default: 30)
    uint8_t preDawnSprayMins;                               // Time to run sprayers/sprinklers (if present/needed) before grow lights turn on, in minutes (default: 60)
    time_t airReportInterval;                               // Interval between air sensor reports, in seconds (default: 8hrs)
    uint8_t natLightOffsetMins;                             // If grow lights should be augmented by natural light (i.e. turn off during daylight hours), in minutes offset from sunrise/sunset (else -1 for disable - default: -1)

    HydroSchedulerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef HydroScheduler_H
