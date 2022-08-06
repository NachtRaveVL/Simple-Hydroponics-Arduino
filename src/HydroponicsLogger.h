/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Logger
*/

#ifndef HydroponicsLogger_H
#define HydroponicsLogger_H

class HydroponicsLogger;
struct HydroponicsLoggerSubData;

#include "Hydroponics.h"

// Logging Level
// Log levels that can be filtered upon if desired.
enum Hydroponics_LogLevel : signed char {
    Hydroponics_LogLevel_All,                               // All (info, warn, err)
    Hydroponics_LogLevel_Warnings,                          // Warnings & errors (warn, err)
    Hydroponics_LogLevel_Errors,                            // Just errors (err)
    Hydroponics_LogLevel_None = -1                          // None / disabled
};

// Hydroponics Logger
// The Logger acts as the system's event monitor that collects and reports on the various
// processes of interest inside of the system. It allows for different log levels to be
// used that can help filter out unwanted noise, as well as attempts to be more optimized
// for embedded systems by spreading string data out over multiple call parameters to
// avoid large string concatenations that can crash constrained devices.
// Logging to SD card .txt log files via SPI card reader is supported.
class HydroponicsLogger {
public:
    HydroponicsLogger();
    ~HydroponicsLogger();

    bool beginLoggingToSDCard(String logFilePrefix);
    inline bool isLoggingToSDCard() const;

    inline void logActivation(const HydroponicsActuator *actuator);
    inline void logDeactivation(const HydroponicsActuator *actuator);
    inline void logProcess(const HydroponicsObject *obj, const String &processString = String(), const String &statusString = String());
    inline void logStatus(const HydroponicsObject *obj, const String &statusString = String());

    void logSystemUptime();
    inline void logSystemSave() { logMessage(SFP(HStr_Log_SystemDataSaved)); }

    void logMessage(const String &msg, const String &suffix1 = String(), const String &suffix2 = String());
    void logWarning(const String &warn, const String &suffix1 = String(), const String &suffix2 = String());
    void logError(const String &err, const String &suffix1 = String(), const String &suffix2 = String());
    void flush();

    void setLogLevel(Hydroponics_LogLevel logLevel);
    inline Hydroponics_LogLevel getLogLevel() const;

    inline bool isLoggingEnabled() const;
    inline time_t getSystemUptime() const { return unixNow() - (_initDate ?: SECONDS_FROM_1970_TO_2000); }

    void notifyDayChanged();

protected:
    String _logFileName;                                    // Resolved log file name (based on day)
    time_t _initDate;                                       // Init date (UTC)
    time_t _lastSpaceCheck;                                 // Last time enough space was checked (UTC)

    friend class Hydroponics;

    inline HydroponicsLoggerSubData *loggerData() const;
    inline bool hasLoggerData() const;

    inline void updateInitTracking() { _initDate = unixNow(); }
    void log(const String &prefix, const String &msg, const String &suffix1, const String &suffix2);
    void cleanupOldestLogs(bool force = false);
};

// Logger Serialization Sub Data
// A part of HSYS system data.
struct HydroponicsLoggerSubData : public HydroponicsSubData {
    Hydroponics_LogLevel logLevel;                          // Log level filter (default: All)
    char logFilePrefix[16];                                 // Base log file name prefix / folder (default: "logs/hy")
    bool logToSDCard;                                       // If publishing to SD card is enabled (default: false)

    HydroponicsLoggerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef HydroponicsLogger_H
