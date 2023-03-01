/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Logger
*/

#ifndef HydroLogger_H
#define HydroLogger_H

class HydroLogger;
struct HydroLoggerSubData;

#include "Hydruino.h"

// Logging Level
// Log levels that can be filtered upon if desired.
enum Hydro_LogLevel : signed char {
    Hydro_LogLevel_All,                                     // All (info, warn, err)
    Hydro_LogLevel_Warnings,                                // Warnings & errors (warn, err)
    Hydro_LogLevel_Errors,                                  // Just errors (err)
    Hydro_LogLevel_None = -1,                               // None / disabled
    Hydro_LogLevel_Info = Hydro_LogLevel_All                // Info alias
};

// Logging Events
// Logging event structure that is used in signaling.
struct HydroLogEvent {
    Hydro_LogLevel level;                                   // Log level
    String timestamp;                                       // Timestamp (generated)
    String prefix;                                          // Prefix
    String msg;                                             // Message
    String suffix1;                                         // Suffix1 (optional)
    String suffix2;                                         // Suffix2 (optional)

    HydroLogEvent(Hydro_LogLevel levelIn,
                  const String &prefixIn,
                  const String &msgIn,
                  const String &suffix1In = String(),
                  const String &suffix2In = String());
};

// Data Logger
// The Logger acts as the system's event monitor that collects and reports on the various
// processes of interest inside of the system. It allows for different log levels to be
// used that can help filter out unwanted noise, as well as attempts to be more optimized
// for embedded systems by spreading string data out over multiple call parameters to
// avoid large string concatenations that can overstress and crash constrained devices.
// Logging to SD card .txt log files (via SPI card reader) is supported as is logging to
// WiFiStorage .txt log files (via OS/OTA filesystem / WiFiNINA_Generic only).
class HydroLogger {
public:
    HydroLogger();
    ~HydroLogger();

    bool beginLoggingToSDCard(String logFilePrefix);
    inline bool isLoggingToSDCard() const;

#ifdef HYDRO_USE_WIFI_STORAGE
    bool beginLoggingToWiFiStorage(String logFilePrefix);
    inline bool isLoggingToWiFiStorage() const;
#endif

    inline void logActivation(const HydroActuator *actuator);
    inline void logDeactivation(const HydroActuator *actuator);
    inline void logProcess(const HydroObjInterface *obj, const String &processString = String(), const String &statusString = String());
    inline void logStatus(const HydroObjInterface *obj, const String &statusString = String());

    void logSystemUptime();
    inline void logSystemSave() { logMessage(SFP(HStr_Log_SystemDataSaved)); }

    void logMessage(const String &msg, const String &suffix1 = String(), const String &suffix2 = String());
    void logWarning(const String &warn, const String &suffix1 = String(), const String &suffix2 = String());
    void logError(const String &err, const String &suffix1 = String(), const String &suffix2 = String());
    void flush();

    void setLogLevel(Hydro_LogLevel logLevel);
    inline Hydro_LogLevel getLogLevel() const;

    inline bool isLoggingEnabled() const;
    inline time_t getSystemUptime() const { return unixNow() - (_initTime ?: SECS_YR_2000); }

    Signal<const HydroLogEvent, HYDRO_LOG_SIGNAL_SLOTS> &getLogSignal();

    void notifyDayChanged();

protected:
#if HYDRO_SYS_LEAVE_FILES_OPEN
    File *_logFileSD;                                       // SD card log file instance (owned)
#ifdef HYDRO_USE_WIFI_STORAGE
    WiFiStorageFile *_logFileWS;                            // WiFiStorageFile log file instance (owned)
#endif
#endif
    String _logFilename;                                    // Resolved log file name (based on day)
    time_t _initTime;                                       // Time of init, for uptime (UTC)
    time_t _lastSpaceCheck;                                 // Last time enough space was checked (UTC)

    Signal<const HydroLogEvent, HYDRO_LOG_SIGNAL_SLOTS> _logSignal; // Logging signal

    friend class Hydruino;

    inline HydroLoggerSubData *loggerData() const;
    inline bool hasLoggerData() const;

    inline void updateInitTracking() { _initTime = unixNow(); }
    void log(const HydroLogEvent &event);
    void cleanupOldestLogs(bool force = false);
};

// Logger Serialization Sub Data
// A part of HSYS system data.
struct HydroLoggerSubData : public HydroSubData {
    Hydro_LogLevel logLevel;                                // Log level filter (default: All)
    char logFilePrefix[16];                                 // Base log file name prefix / folder (default: "logs/hy")
    bool logToSDCard;                                       // If system logging to SD card is enabled (default: false)
    bool logToWiFiStorage;                                  // If system logging to WiFiStorage is enabled (default: false)

    HydroLoggerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef HydroLogger_H
