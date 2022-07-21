/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Logger
*/

#include "Hydroponics.h"

HydroponicsLogger::HydroponicsLogger()
    : _loggerData(nullptr), _logFileName(), _lastSpaceCheck(0)
{ ; }

HydroponicsLogger::~HydroponicsLogger()
{
    flush();
}

void HydroponicsLogger::initFromData(HydroponicsLoggerSubData *dataIn)
{
    _loggerData = dataIn;
}

void HydroponicsLogger::update()
{ ; }

void HydroponicsLogger::resolveLinks()
{ ; }

void HydroponicsLogger::handleLowMemory()
{ ; }

bool HydroponicsLogger::beginLoggingToSDCard(String logFilePrefix)
{
    HYDRUINO_SOFT_ASSERT(_loggerData, SFP(HS_Err_NotYetInitialized));

    if (_loggerData) {
        auto sd = getHydroponicsInstance()->getSDCard();

        if (sd && sd->exists("/")) {
            String logFileName = getYYMMDDFilename(logFilePrefix, SFP(HS_txt));
            auto logFile = sd->open(logFileName, FILE_WRITE);

            if (logFile.availableForWrite()) {
                logFile.close();
                getHydroponicsInstance()->endSDCard(sd);

                getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
                strncpy(_loggerData->logFilePrefix, logFilePrefix.c_str(), 16);
                _loggerData->logToSDCard = true;
                _logFileName = logFileName;

                return true;
            }

            if (logFile) { logFile.close(); }
        }

        if (sd) { getHydroponicsInstance()->endSDCard(sd); }
    }

    return false;
}

bool HydroponicsLogger::getIsLoggingToSDCard() const
{
    HYDRUINO_SOFT_ASSERT(_loggerData, SFP(HS_Err_NotYetInitialized));
    return _loggerData && _loggerData->logToSDCard;
}

void HydroponicsLogger::logActivation(HydroponicsActuator *actuator)
{
    if (actuator) {
        String msg = actuator->getId().keyStr + SFP(HS_Log_HasEnabled);
        logMessage(msg);
    }
}

void HydroponicsLogger::logDeactivation(HydroponicsActuator *actuator)
{
    if (actuator) {
        String msg = actuator->getId().keyStr + SFP(HS_Log_HasDisabled);
        logMessage(msg);
    }
}

void HydroponicsLogger::logFeedingBegan(HydroponicsFeedReservoir *feedReservoir, String feedingInfo)
{
    if (feedReservoir) {
        String msg = feedReservoir->getId().keyStr + SFP(HS_Log_FeedingSequence) + SFP(HS_Log_HasBegan) +
                     (feedingInfo.length() ? String(' ') + String('(') + feedingInfo + String(')') : String());
        logMessage(msg);
    }
}

void HydroponicsLogger::logFeedingEnded(HydroponicsFeedReservoir *feedReservoir, String feedingInfo)
{
    if (feedReservoir) {
        String msg = feedReservoir->getId().keyStr + SFP(HS_Log_FeedingSequence) + SFP(HS_Log_HasEnded) +
                     (feedingInfo.length() ? String(' ') + String('(') + feedingInfo + String(')') : String());
        logMessage(msg);
    }
}

void HydroponicsLogger::logLightingBegan(HydroponicsFeedReservoir *feedReservoir, String lightingInfo)
{
    if (feedReservoir) {
        String msg = feedReservoir->getId().keyStr + SFP(HS_Log_LightingSequence) + SFP(HS_Log_HasBegan) +
                     (lightingInfo.length() ? String(' ') + String('(') + lightingInfo + String(')') : String());
        logMessage(msg);
    }
}

void HydroponicsLogger::logLightingEnded(HydroponicsFeedReservoir *feedReservoir, String lightingInfo)
{
    if (feedReservoir) {
        String msg = feedReservoir->getId().keyStr + SFP(HS_Log_LightingSequence) + SFP(HS_Log_HasEnded) +
                     (lightingInfo.length() ? String(' ') + String('(') + lightingInfo + String(')') : String());
        logMessage(msg);
    }
}

void HydroponicsLogger::logEstimatedPumping(HydroponicsPumpObjectInterface *pump, String estimationInfo)
{
    if (pump) {
        String msg = ((HydroponicsObject *)pump)->getId().keyStr + SFP(HS_Log_EstimatedPumping) + estimationInfo;
        logMessage(msg);
    }
}

void HydroponicsLogger::logMeasuredPumping(HydroponicsPumpObjectInterface *pump, String measuredInfo)
{
    if (pump) {
        String msg = ((HydroponicsObject *)pump)->getId().keyStr + SFP(HS_Log_MeasuredPumping) + measuredInfo;
        logMessage(msg);
    }
}

void HydroponicsLogger::logSystemUptime()
{
    time_t elapsedTime = now() - _initDate;
    DateTime elapsed((uint32_t)elapsedTime);
    String msg = SFP(HS_Log_SystemUptime) + String(elapsedTime / SECS_PER_DAY) + String('d') + String(' ') +
                 elapsed.timestamp(DateTime::TIMESTAMP_TIME);
}

void HydroponicsLogger::logSystemSave()
{
    logMessage(SFP(HS_Log_SystemDataSaved));
}

void HydroponicsLogger::logMessage(String msg)
{
    if (_loggerData && _loggerData->logLevel != Hydroponics_LogLevel_None && _loggerData->logLevel <= Hydroponics_LogLevel_All) {
        log(String(F("[INFO] ")) + msg);
    }
}

void HydroponicsLogger::logWarning(String warn)
{
    if (_loggerData && _loggerData->logLevel != Hydroponics_LogLevel_None && _loggerData->logLevel <= Hydroponics_LogLevel_Warnings) {
        log(String(F("[WARN] ")) + warn);
    }
}

void HydroponicsLogger::logError(String err)
{
    if (_loggerData && _loggerData->logLevel != Hydroponics_LogLevel_None && _loggerData->logLevel <= Hydroponics_LogLevel_Errors) {
        log(String(F("[FAIL] ")) + err);
    }
}

void HydroponicsLogger::log(String msg)
{
    String timestamp = getCurrentTime().timestamp(DateTime::TIMESTAMP_FULL);

    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
        if (Serial) {
            Serial.print(timestamp);
            Serial.print(' ');
            Serial.println(msg);
        }
    #endif

    if (getIsLoggingToSDCard()) {
        auto sd = getHydroponicsInstance()->getSDCard();

        if (sd) {
            auto logFile = sd->open(_logFileName, FILE_WRITE);

            if (logFile && logFile.availableForWrite()) {
                logFile.print(timestamp);
                logFile.print(' ');
                logFile.println(msg);
            }

            if (logFile) { logFile.close(); }

            getHydroponicsInstance()->endSDCard(sd);
        }
    }
}

void HydroponicsLogger::flush()
{
    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
        if (Serial) {
            Serial.flush();
            yield();
        }
    #endif
}

void HydroponicsLogger::setLogLevel(Hydroponics_LogLevel logLevel)
{
    HYDRUINO_SOFT_ASSERT(_loggerData, SFP(HS_Err_NotYetInitialized));

    if (_loggerData && _loggerData->logLevel != logLevel) {
        getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
        _loggerData->logLevel = logLevel;
    }
}

Hydroponics_LogLevel HydroponicsLogger::getLogLevel() const
{
    HYDRUINO_SOFT_ASSERT(_loggerData, SFP(HS_Err_NotYetInitialized));
    return _loggerData ? _loggerData->logLevel : Hydroponics_LogLevel_None;
}

bool HydroponicsLogger::getIsLoggingEnabled() const
{
    HYDRUINO_SOFT_ASSERT(_loggerData, SFP(HS_Err_NotYetInitialized));
    return _loggerData && _loggerData->logLevel != Hydroponics_LogLevel_None &&
           (_loggerData->logToSDCard);
}

void HydroponicsLogger::notifyDayChanged()
{
    if (getIsLoggingEnabled()) {
        _logFileName = getYYMMDDFilename(stringFromChars(_loggerData->logFilePrefix, 16), SFP(HS_txt));
        cleanupOldestLogs();
    }
}

void HydroponicsLogger::cleanupOldestLogs(bool force)
{
    // TODO: Old data cleanup.
}


HydroponicsLoggerSubData::HydroponicsLoggerSubData()
    : HydroponicsSubData(), logLevel(Hydroponics_LogLevel_All), logFilePrefix{0}, logToSDCard(false)
{
    type = 0; // no type differentiation
}

void HydroponicsLoggerSubData::toJSONObject(JsonObject &objectOut) const
{
    //HydroponicsSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    if (logLevel != Hydroponics_LogLevel_All) { objectOut[SFP(HS_Key_LogLevel)] = logLevel; }
    if (logFilePrefix[0]) { objectOut[SFP(HS_Key_LogFilePrefix)] = stringFromChars(logFilePrefix, 16); }
    if (logToSDCard != false) { objectOut[SFP(HS_Key_LogToSDCard)] = logToSDCard; }
}

void HydroponicsLoggerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    //HydroponicsSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    logLevel = objectIn[SFP(HS_Key_LogLevel)] | logLevel;
    const char *logFilePrefixStr = objectIn[SFP(HS_Key_LogFilePrefix)];
    if (logFilePrefixStr && logFilePrefixStr[0]) { strncpy(logFilePrefix, logFilePrefixStr, 16); }
    logToSDCard = objectIn[SFP(HS_Key_LogToSDCard)] | logToSDCard;
}
