/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Logger
*/

#include "Hydruino.h"

HydroLogEvent::HydroLogEvent(Hydro_LogLevel levelIn, const String &prefixIn, const String &msgIn, const String &suffix1In, const String &suffix2In)
    : level(levelIn), timestamp(localNow().timestamp(DateTime::TIMESTAMP_FULL)), prefix(prefixIn), msg(msgIn), suffix1(suffix1In), suffix2(suffix2In)
{ ; }


HydroLogger::HydroLogger() :
#if HYDRO_SYS_LEAVE_FILES_OPEN
    _logFileSD(nullptr),
#ifdef HYDRO_USE_WIFI_STORAGE
    _logFileWS(nullptr),
#endif
#endif
    _logFilename(), _initTime(0), _lastSpaceCheck(0)
{ ; }

HydroLogger::~HydroLogger()
{
    flush();

    #if HYDRO_SYS_LEAVE_FILES_OPEN
        if (_logFileSD) {
            _logFileSD->close();
            delete _logFileSD; _logFileSD = nullptr;
            Hydruino::_activeInstance->endSDCard();
        }
        #ifdef HYDRO_USE_WIFI_STORAGE
            if (_logFileWS) {
                _logFileWS->close();
                delete _logFileWS; _logFileWS = nullptr;
            }
        #endif
    #endif
}

bool HydroLogger::beginLoggingToSDCard(String logFilePrefix)
{
    HYDRO_SOFT_ASSERT(hasLoggerData(), SFP(HStr_Err_NotYetInitialized));

    if (hasLoggerData() && !loggerData()->logToSDCard) {
        auto sd = Hydruino::_activeInstance->getSDCard();

        if (sd) {
            String logFilename = getYYMMDDFilename(logFilePrefix, SFP(HStr_txt));
            createDirectoryFor(sd, logFilename);
            #if HYDRO_SYS_LEAVE_FILES_OPEN
                auto &logFile = _logFileSD ? *_logFileSD : *(_logFileSD = new File(sd->open(logFilename.c_str(), FILE_WRITE)));
            #else
                auto logFile = sd->open(logFilename.c_str(), FILE_WRITE);
            #endif

            if (logFile) {
                #if !HYDRO_SYS_LEAVE_FILES_OPEN
                    logFile.close();
                    Hydruino::_activeInstance->endSDCard(sd);
                #endif

                strncpy(loggerData()->logFilePrefix, logFilePrefix.c_str(), 16);
                loggerData()->logToSDCard = true;
                _logFilename = logFilename;
                Hydruino::_activeInstance->_systemData->bumpRevisionIfNeeded();

                return true;
            }
        }

        #if !HYDRO_SYS_LEAVE_FILES_OPEN
            Hydruino::_activeInstance->endSDCard(sd);
        #endif
    }

    return false;
}

#ifdef HYDRO_USE_WIFI_STORAGE

bool HydroLogger::beginLoggingToWiFiStorage(String logFilePrefix)
{
    HYDRO_SOFT_ASSERT(hasLoggerData(), SFP(HStr_Err_NotYetInitialized));

    if (hasLoggerData() && !loggerData()->logToWiFiStorage) {
        String logFilename = getYYMMDDFilename(logFilePrefix, SFP(HStr_txt));
        #if HYDRO_SYS_LEAVE_FILES_OPEN
            auto &logFile = _logFileWS ? *_logFileWS : *(_logFileWS = new WiFiStorageFile(WiFiStorage.open(logFilename.c_str())));
        #else
            auto logFile = WiFiStorage.open(logFilename.c_str());
        #endif

        if (logFile) {
            #if !HYDRO_SYS_LEAVE_FILES_OPEN
                logFile.close();
            #endif

            strncpy(loggerData()->logFilePrefix, logFilePrefix.c_str(), 16);
            loggerData()->logToWiFiStorage = true;
            _logFilename = logFilename;
            Hydruino::_activeInstance->_systemData->bumpRevisionIfNeeded();

            return true;
        }
    }

    return false;
}

#endif

void HydroLogger::logSystemUptime()
{
    TimeSpan elapsed(getSystemUptime());
    if (elapsed.totalseconds()) {
        logMessage(SFP(HStr_Log_SystemUptime), timeSpanToString(elapsed));
    }
}

void HydroLogger::logMessage(const String &msg, const String &suffix1, const String &suffix2)
{
    if (!hasLoggerData() || (loggerData()->logLevel != Hydro_LogLevel_None && loggerData()->logLevel <= Hydro_LogLevel_All)) {
        log(HydroLogEvent(Hydro_LogLevel_Info, SFP(HStr_Log_Prefix_Info), msg, suffix1, suffix2));
    }
}

void HydroLogger::logWarning(const String &warn, const String &suffix1, const String &suffix2)
{
    if (!hasLoggerData() || (loggerData()->logLevel != Hydro_LogLevel_None && loggerData()->logLevel <= Hydro_LogLevel_Warnings)) {
        log(HydroLogEvent(Hydro_LogLevel_Warnings, SFP(HStr_Log_Prefix_Warning), warn, suffix1, suffix2));
    }
}

void HydroLogger::logError(const String &err, const String &suffix1, const String &suffix2)
{
    if (!hasLoggerData() || (loggerData()->logLevel != Hydro_LogLevel_None && loggerData()->logLevel <= Hydro_LogLevel_Errors)) {
        log(HydroLogEvent(Hydro_LogLevel_Errors, SFP(HStr_Log_Prefix_Error), err, suffix1, suffix2));
    }
}

void HydroLogger::log(const HydroLogEvent &event)
{
    #ifdef HYDRO_ENABLE_DEBUG_OUTPUT
        if (Serial) {
            Serial.print(event.timestamp);
            Serial.print(' ');
            Serial.print(event.prefix);
            Serial.print(event.msg);
            Serial.print(event.suffix1);
            Serial.println(event.suffix2);
        }
    #endif

    if (isLoggingToSDCard()) {
        auto sd = Hydruino::_activeInstance->getSDCard(HYDRO_LOFS_BEGIN);

        if (sd) {
            #if HYDRO_SYS_LEAVE_FILES_OPEN
                auto &logFile = _logFileSD ? *_logFileSD : *(_logFileSD = new File(sd->open(_logFilename.c_str(), FILE_WRITE)));
            #else
                createDirectoryFor(sd, _logFilename);
                auto logFile = sd->open(_logFilename.c_str(), FILE_WRITE);
            #endif

            if (logFile) {
                logFile.print(event.timestamp);
                logFile.print(' ');
                logFile.print(event.prefix);
                logFile.print(event.msg);
                logFile.print(event.suffix1);
                logFile.println(event.suffix2);

                #if !HYDRO_SYS_LEAVE_FILES_OPEN
                    logFile.flush();
                    logFile.close();
                #endif
            }

            #if !HYDRO_SYS_LEAVE_FILES_OPEN
                Hydruino::_activeInstance->endSDCard(sd);
            #endif
        }
    }

#ifdef HYDRO_USE_WIFI_STORAGE

    if (isLoggingToWiFiStorage()) {
        #if HYDRO_SYS_LEAVE_FILES_OPEN
            auto &logFile = _logFileWS ? *_logFileWS : *(_logFileWS = new WiFiStorageFile(WiFiStorage.open(_logFilename.c_str())));
        #else
            auto logFile = WiFiStorage.open(_logFilename.c_str());
        #endif

        if (logFile) {
            auto logFileStream = HydroWiFiStorageFileStream(logFile, logFile.size());

            logFileStream.print(event.timestamp);
            logFileStream.print(' ');
            logFileStream.print(event.prefix);
            logFileStream.print(event.msg);
            logFileStream.print(event.suffix1);
            logFileStream.println(event.suffix2);

            #if !HYDRO_SYS_LEAVE_FILES_OPEN
                logFileStream.flush();
                logFile.close();
            #endif
        }
    }

#endif

    #ifdef HYDRO_USE_MULTITASKING
        scheduleSignalFireOnce<const HydroLogEvent>(_logSignal, event);
    #else
        _logSignal.fire(event);
    #endif
}

void HydroLogger::flush()
{
    #ifdef HYDRO_ENABLE_DEBUG_OUTPUT
        if (Serial) { Serial.flush(); }
    #endif
    #if HYDRO_SYS_LEAVE_FILES_OPEN
        if(_logFileSD) { _logFileSD->flush(); }
    #endif
    yield();
}

void HydroLogger::setLogLevel(Hydro_LogLevel logLevel)
{
    HYDRO_SOFT_ASSERT(hasLoggerData(), SFP(HStr_Err_NotYetInitialized));
    if (hasLoggerData() && loggerData()->logLevel != logLevel) {
        loggerData()->logLevel = logLevel;
        Hydruino::_activeInstance->_systemData->bumpRevisionIfNeeded();
    }
}

Signal<const HydroLogEvent, HYDRO_LOG_SIGNAL_SLOTS> &HydroLogger::getLogSignal()
{
    return _logSignal;
}

void HydroLogger::notifyDateChanged()
{
    if (isLoggingEnabled()) {
        _logFilename = getYYMMDDFilename(charsToString(loggerData()->logFilePrefix, 16), SFP(HStr_txt));
        cleanupOldestLogs();
    }
}

void HydroLogger::cleanupOldestLogs(bool force)
{
    // TODO: Old data cleanup. #17 in Hydruino.
}


HydroLoggerSubData::HydroLoggerSubData()
    : HydroSubData(), logLevel(Hydro_LogLevel_All), logFilePrefix{0}, logToSDCard(false), logToWiFiStorage(false)
{
    type = 0; // no type differentiation
}

void HydroLoggerSubData::toJSONObject(JsonObject &objectOut) const
{
    //HydroSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    if (logLevel != Hydro_LogLevel_All) { objectOut[SFP(HStr_Key_LogLevel)] = logLevel; }
    if (logFilePrefix[0]) { objectOut[SFP(HStr_Key_LogFilePrefix)] = charsToString(logFilePrefix, 16); }
    if (logToSDCard != false) { objectOut[SFP(HStr_Key_LogToSDCard)] = logToSDCard; }
    if (logToWiFiStorage != false) { objectOut[SFP(HStr_Key_LogToWiFiStorage)] = logToWiFiStorage; }
}

void HydroLoggerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    //HydroSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    logLevel = objectIn[SFP(HStr_Key_LogLevel)] | logLevel;
    const char *logFilePrefixStr = objectIn[SFP(HStr_Key_LogFilePrefix)];
    if (logFilePrefixStr && logFilePrefixStr[0]) { strncpy(logFilePrefix, logFilePrefixStr, 16); }
    logToSDCard = objectIn[SFP(HStr_Key_LogToSDCard)] | logToSDCard;
    logToWiFiStorage = objectIn[SFP(HStr_Key_LogToWiFiStorage)] | logToWiFiStorage;
}
