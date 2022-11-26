/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Logger
*/

#include "Hydroponics.h"

HydroponicsLogEvent::HydroponicsLogEvent(Hydroponics_LogLevel levelIn, const String &prefixIn, const String &msgIn, const String &suffix1In, const String &suffix2In)
    : level(levelIn), timestamp(getCurrentTime().timestamp(DateTime::TIMESTAMP_FULL)), prefix(prefixIn), msg(msgIn), suffix1(suffix1In), suffix2(suffix2In)
{ ; }


HydroponicsLogger::HydroponicsLogger()
    : _logFilename(), _initDate(0), _lastSpaceCheck(0)
{ ; }

HydroponicsLogger::~HydroponicsLogger()
{
    flush();

    #if HYDRUINO_SYS_LEAVE_FILES_OPEN
        if (_logFileSD) {
            _logFileSD->close();
            delete _logFileSD; _logFileSD = nullptr;
            Hydroponics::_activeInstance->endSDCard();
        }
        #ifdef HYDRUINO_USE_WIFI_STORAGE
            if (_logFileWS) {
                _logFileWS->close();
                delete _logFileWS; _logFileWS = nullptr;
            }
        #endif
    #endif
}

bool HydroponicsLogger::beginLoggingToSDCard(String logFilePrefix)
{
    HYDRUINO_SOFT_ASSERT(hasLoggerData(), SFP(HStr_Err_NotYetInitialized));

    if (hasLoggerData() && !loggerData()->logToSDCard) {
        auto sd = Hydroponics::_activeInstance->getSDCard();

        if (sd) {
            String logFilename = getYYMMDDFilename(logFilePrefix, SFP(HStr_txt));
            createDirectoryFor(sd, logFilename);
            #if HYDRUINO_SYS_LEAVE_FILES_OPEN
                auto &logFile = _logFileSD ? *_logFileSD : *(_logFileSD = new File(sd->open(logFilename.c_str(), FILE_WRITE)));
            #else
                auto logFile = sd->open(logFilename.c_str(), FILE_WRITE);
            #endif

            if (logFile) {
                #if !HYDRUINO_SYS_LEAVE_FILES_OPEN
                    logFile.close();
                    Hydroponics::_activeInstance->endSDCard(sd);
                #endif

                Hydroponics::_activeInstance->_systemData->_bumpRevIfNotAlreadyModded();
                strncpy(loggerData()->logFilePrefix, logFilePrefix.c_str(), 16);
                loggerData()->logToSDCard = true;
                _logFilename = logFilename;

                return true;
            }
        }

        #if !HYDRUINO_SYS_LEAVE_FILES_OPEN
            Hydroponics::_activeInstance->endSDCard(sd);
        #endif
    }

    return false;
}

#ifdef HYDRUINO_USE_WIFI_STORAGE

bool HydroponicsLogger::beginLoggingToWiFiStorage(String logFilePrefix)
{
    HYDRUINO_SOFT_ASSERT(hasLoggerData(), SFP(HStr_Err_NotYetInitialized));

    if (hasLoggerData() && !loggerData()->logToWiFiStorage) {
        String logFilename = getYYMMDDFilename(logFilePrefix, SFP(HStr_txt));
        #if HYDRUINO_SYS_LEAVE_FILES_OPEN
            auto &logFile = _logFileWS ? *_logFileWS : *(_logFileWS = new WiFiStorageFile(WiFiStorage.open(logFilename.c_str())));
        #else
            auto logFile = WiFiStorage.open(logFilename.c_str());
        #endif

        if (logFile) {
            #if !HYDRUINO_SYS_LEAVE_FILES_OPEN
                logFile.close();
            #endif

            Hydroponics::_activeInstance->_systemData->_bumpRevIfNotAlreadyModded();
            strncpy(loggerData()->logFilePrefix, logFilePrefix.c_str(), 16);
            loggerData()->logToWiFiStorage = true;
            _logFilename = logFilename;

            return true;
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
        log(HydroponicsLogEvent(Hydroponics_LogLevel_Info, SFP(HStr_Log_Prefix_Info), msg, suffix1, suffix2));
    }
}

void HydroponicsLogger::logWarning(const String &warn, const String &suffix1, const String &suffix2)
{
    if (hasLoggerData() && loggerData()->logLevel != Hydroponics_LogLevel_None && loggerData()->logLevel <= Hydroponics_LogLevel_Warnings) {
        log(HydroponicsLogEvent(Hydroponics_LogLevel_Warnings, SFP(HStr_Log_Prefix_Warning), warn, suffix1, suffix2));
    }
}

void HydroponicsLogger::logError(const String &err, const String &suffix1, const String &suffix2)
{
    if (hasLoggerData() && loggerData()->logLevel != Hydroponics_LogLevel_None && loggerData()->logLevel <= Hydroponics_LogLevel_Errors) {
        log(HydroponicsLogEvent(Hydroponics_LogLevel_Errors, SFP(HStr_Log_Prefix_Error), err, suffix1, suffix2));
    }
}

void HydroponicsLogger::log(const HydroponicsLogEvent &event)
{
    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
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
        auto sd = Hydroponics::_activeInstance->getSDCard(HYDRUINO_LOFS_BEGIN);

        if (sd) {
            #if HYDRUINO_SYS_LEAVE_FILES_OPEN
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

                #if !HYDRUINO_SYS_LEAVE_FILES_OPEN
                    logFile.flush();
                    logFile.close();
                #endif
            }

            #if !HYDRUINO_SYS_LEAVE_FILES_OPEN
                Hydroponics::_activeInstance->endSDCard(sd);
            #endif
        }
    }

#ifdef HYDRUINO_USE_WIFI_STORAGE

    if (isLoggingToWiFiStorage()) {
        #if HYDRUINO_SYS_LEAVE_FILES_OPEN
            auto &logFile = _logFileWS ? *_logFileWS : *(_logFileWS = new WiFiStorageFile(WiFiStorage.open(_logFilename.c_str())));
        #else
            auto logFile = WiFiStorage.open(_logFilename.c_str());
        #endif

        if (logFile) {
            auto logFileStream = HydroponicsWiFiStorageFileStream(logFile, logFile.size());

            logFileStream.print(event.timestamp);
            logFileStream.print(' ');
            logFileStream.print(event.prefix);
            logFileStream.print(event.msg);
            logFileStream.print(event.suffix1);
            logFileStream.println(event.suffix2);

            #if !HYDRUINO_SYS_LEAVE_FILES_OPEN
                logFileStream.flush();
                logFile.close();
            #endif
        }
    }

#endif

    #ifndef HYDRUINO_DISABLE_MULTITASKING
        scheduleSignalFireOnce<const HydroponicsLogEvent>(_logSignal, event);
    #else
        _logSignal.fire(event);
    #endif
}

void HydroponicsLogger::flush()
{
    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
        if (Serial) { Serial.flush(); }
    #endif
    #if HYDRUINO_SYS_LEAVE_FILES_OPEN
        if(_logFileSD) { _logFileSD->flush(); }
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

Signal<const HydroponicsLogEvent, HYDRUINO_LOG_STATE_SLOTS> &HydroponicsLogger::getLogSignal()
{
    return _logSignal;
}

void HydroponicsLogger::notifyDayChanged()
{
    if (isLoggingEnabled()) {
        _logFilename = getYYMMDDFilename(charsToString(loggerData()->logFilePrefix, 16), SFP(HStr_txt));
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
