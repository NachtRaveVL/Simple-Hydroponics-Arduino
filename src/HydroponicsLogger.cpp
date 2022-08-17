/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Logger
*/

#include "Hydroponics.h"

HydroponicsLogger::HydroponicsLogger()
    : _logFileName(), _initDate(0), _lastSpaceCheck(0)
{ ; }

HydroponicsLogger::~HydroponicsLogger()
{
    flush();
}

bool HydroponicsLogger::beginLoggingToSDCard(String logFilePrefix)
{
    HYDRUINO_SOFT_ASSERT(hasLoggerData(), SFP(HStr_Err_NotYetInitialized));

    if (hasLoggerData()) {
        auto sd = Hydroponics::_activeInstance->getSDCard();

        if (sd) {
            String logFileName = getYYMMDDFilename(logFilePrefix, SFP(HStr_txt));
            createDirectoryFor(sd, logFileName);
            auto logFile = sd->open(logFileName.c_str(), FILE_WRITE);

            if (logFile) {
                logFile.close();
                Hydroponics::_activeInstance->endSDCard(sd);

                Hydroponics::_activeInstance->_systemData->_bumpRevIfNotAlreadyModded();
                strncpy(loggerData()->logFilePrefix, logFilePrefix.c_str(), 16);
                loggerData()->logToSDCard = true;
                _logFileName = logFileName;

                return true;
            }
        }

        if (sd) { Hydroponics::_activeInstance->endSDCard(sd); }
    }

    return false;
}

#ifdef HYDRUINO_USE_WIFI_STORAGE

bool HydroponicsLogger::beginLoggingToWiFiStorage(String logFilePrefix)
{
    HYDRUINO_SOFT_ASSERT(hasLoggerData(), SFP(HStr_Err_NotYetInitialized));

    if (hasLoggerData()) {
        if (Hydroponics::_activeInstance->getWiFi()) {
            String logFileName = getYYMMDDFilename(logFilePrefix, SFP(HStr_txt));
            auto logFile = WiFiStorage.open(logFileName.c_str());

            if (logFile) {
                logFile.close();

                Hydroponics::_activeInstance->_systemData->_bumpRevIfNotAlreadyModded();
                strncpy(loggerData()->logFilePrefix, logFilePrefix.c_str(), 16);
                loggerData()->logToWiFiStorage = true;
                _logFileName = logFileName;

                return true;
            }
        }
    }

    return false;
}

#endif

void HydroponicsLogger::logSystemUptime()
{
    TimeSpan elapsed(getSystemUptime());
    if (elapsed.totalseconds()) {
        logMessage(SFP(HStr_Log_SystemUptime), timeSpanToString(elapsed));
    }
}

void HydroponicsLogger::logMessage(const String &msg, const String &suffix1, const String &suffix2)
{
    if (hasLoggerData() && loggerData()->logLevel != Hydroponics_LogLevel_None && loggerData()->logLevel <= Hydroponics_LogLevel_All) {
        log(SFP(HStr_Log_Prefix_Info), msg, suffix1, suffix2);
    }
}

void HydroponicsLogger::logWarning(const String &warn, const String &suffix1, const String &suffix2)
{
    if (hasLoggerData() && loggerData()->logLevel != Hydroponics_LogLevel_None && loggerData()->logLevel <= Hydroponics_LogLevel_Warnings) {
        log(SFP(HStr_Log_Prefix_Warning), warn, suffix1, suffix2);
    }
}

void HydroponicsLogger::logError(const String &err, const String &suffix1, const String &suffix2)
{
    if (hasLoggerData() && loggerData()->logLevel != Hydroponics_LogLevel_None && loggerData()->logLevel <= Hydroponics_LogLevel_Errors) {
        log(SFP(HStr_Log_Prefix_Error), err, suffix1, suffix2);
    }
}

void HydroponicsLogger::log(const String &prefix, const String &msg, const String &suffix1, const String &suffix2)
{
    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
        if (Serial) {
            Serial.print(getCurrentTime().timestamp(DateTime::TIMESTAMP_FULL));
            Serial.print(' ');
            Serial.print(prefix);
            Serial.print(msg);
            Serial.print(suffix1);
            Serial.println(suffix2);
        }
    #endif

    if (isLoggingToSDCard()) {
        auto sd = Hydroponics::_activeInstance->getSDCard();

        if (sd) {
            createDirectoryFor(sd, _logFileName);
            auto logFile = sd->open(_logFileName.c_str(), FILE_WRITE);

            if (logFile) {
                logFile.print(getCurrentTime().timestamp(DateTime::TIMESTAMP_FULL));
                logFile.print(' ');
                logFile.print(prefix);
                logFile.print(msg);
                logFile.print(suffix1);
                logFile.println(suffix2);

                logFile.flush();
                logFile.close();
            }

            Hydroponics::_activeInstance->endSDCard(sd);
        }
    }

#ifdef HYDRUINO_USE_WIFI_STORAGE

    if (isLoggingToWiFiStorage() && Hydroponics::_activeInstance->getWiFi()) {
        auto logFile = WiFiStorage.open(_logFileName.c_str());

        if (logFile) {
            auto logFileStream = HydroponicsWiFiStorageFileStream(logFile, logFile.size());

            logFileStream.print(getCurrentTime().timestamp(DateTime::TIMESTAMP_FULL));
            logFileStream.print(' ');
            logFileStream.print(prefix);
            logFileStream.print(msg);
            logFileStream.print(suffix1);
            logFileStream.println(suffix2);
        }
    }

#endif
}

void HydroponicsLogger::flush()
{
    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
        if (Serial) { Serial.flush(); }
    #endif
    yield();
}

void HydroponicsLogger::setLogLevel(Hydroponics_LogLevel logLevel)
{
    HYDRUINO_SOFT_ASSERT(hasLoggerData(), SFP(HStr_Err_NotYetInitialized));
    if (hasLoggerData() && loggerData()->logLevel != logLevel) {
        Hydroponics::_activeInstance->_systemData->_bumpRevIfNotAlreadyModded();
        loggerData()->logLevel = logLevel;
    }
}

void HydroponicsLogger::notifyDayChanged()
{
    if (isLoggingEnabled()) {
        _logFileName = getYYMMDDFilename(charsToString(loggerData()->logFilePrefix, 16), SFP(HStr_txt));
        cleanupOldestLogs();
    }
}

void HydroponicsLogger::cleanupOldestLogs(bool force)
{
    // TODO: Old data cleanup.
}


HydroponicsLoggerSubData::HydroponicsLoggerSubData()
    : HydroponicsSubData(), logLevel(Hydroponics_LogLevel_All), logFilePrefix{0}, logToSDCard(false), logToWiFiStorage(false)
{
    type = 0; // no type differentiation
}

void HydroponicsLoggerSubData::toJSONObject(JsonObject &objectOut) const
{
    //HydroponicsSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    if (logLevel != Hydroponics_LogLevel_All) { objectOut[SFP(HStr_Key_LogLevel)] = logLevel; }
    if (logFilePrefix[0]) { objectOut[SFP(HStr_Key_LogFilePrefix)] = charsToString(logFilePrefix, 16); }
    if (logToSDCard != false) { objectOut[SFP(HStr_Key_LogToSDCard)] = logToSDCard; }
    if (logToWiFiStorage != false) { objectOut[SFP(HStr_Key_LogToWiFiStorage)] = logToWiFiStorage; }
}

void HydroponicsLoggerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    //HydroponicsSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    logLevel = objectIn[SFP(HStr_Key_LogLevel)] | logLevel;
    const char *logFilePrefixStr = objectIn[SFP(HStr_Key_LogFilePrefix)];
    if (logFilePrefixStr && logFilePrefixStr[0]) { strncpy(logFilePrefix, logFilePrefixStr, 16); }
    logToSDCard = objectIn[SFP(HStr_Key_LogToSDCard)] | logToSDCard;
    logToWiFiStorage = objectIn[SFP(HStr_Key_LogToWiFiStorage)] | logToWiFiStorage;
}
