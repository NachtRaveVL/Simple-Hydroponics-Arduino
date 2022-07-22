/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Logger
*/

#ifndef HydroponicsLogger_H
#define HydroponicsLogger_H

class HydroponicsLogger;
struct HydroponicsLoggerSubData;

#include "Hydroponics.h"

enum Hydroponics_LogLevel {
    Hydroponics_LogLevel_All,
    Hydroponics_LogLevel_Warnings,
    Hydroponics_LogLevel_Errors,
    Hydroponics_LogLevel_None = -1
};

// Hydroponics Logger
class HydroponicsLogger : public HydroponicsSubObject {
public:
    HydroponicsLogger();
    virtual ~HydroponicsLogger();
    void initFromData(HydroponicsLoggerSubData *dataIn);

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    bool beginLoggingToSDCard(String logFilePrefix);
    bool getIsLoggingToSDCard() const;

    void logActivation(HydroponicsActuator *actuator);
    void logDeactivation(HydroponicsActuator *actuator);

    void logProcess(HydroponicsFeedReservoir *feedReservoir, String processString, String statusString);

    void logPumping(HydroponicsPumpObjectInterface *pump, String pumpString);

    void logSystemUptime();
    void logSystemSave();

    void logMessage(String msg, String suffix = String());
    void logWarning(String warn, String suffix = String());
    void logError(String err, String suffix = String());
    void flush();

    void setLogLevel(Hydroponics_LogLevel logLevel);
    Hydroponics_LogLevel getLogLevel() const;
    bool getIsLoggingEnabled() const;

    void notifyDayChanged();

protected:
    HydroponicsLoggerSubData *_loggerData;                  // Logger data (strong, saved to storage via system data)

    String _logFileName;                                    // Resolved log file name (based on day)
    time_t _initDate;                                       // Init date (UTC, controlled by hydroponics)
    time_t _lastSpaceCheck;                                 // Last time enough space was checked (UTC)

    friend class Hydroponics;

    void log(String prefix, String msg, String suffix);

    void cleanupOldestLogs(bool force = false);
};

// Logger Serialization Sub Data
// A part of HSYS system data.
struct HydroponicsLoggerSubData : public HydroponicsSubData {
    Hydroponics_LogLevel logLevel;
    char logFilePrefix[16];
    bool logToSDCard;

    HydroponicsLoggerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef HydroponicsLogger_H
