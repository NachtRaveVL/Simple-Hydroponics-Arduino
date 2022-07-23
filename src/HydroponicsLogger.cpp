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

        if (sd) {
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
        logMessage(actuator->getId().keyStr, SFP(HS_Log_HasEnabled));
    }
}

void HydroponicsLogger::logDeactivation(HydroponicsActuator *actuator)
{
    if (actuator) {
        logMessage(actuator->getId().keyStr, SFP(HS_Log_HasDisabled));
    }
}

void HydroponicsLogger::logProcess(HydroponicsFeedReservoir *feedReservoir, String processString, String statusString)
{
    if (feedReservoir) {
        logMessage(feedReservoir->getId().keyStr, processString, statusString);
    }
}

void HydroponicsLogger::logPumping(HydroponicsPumpObjectInterface *pump, String pumpString)
{
    if (pump) {
        logMessage(((HydroponicsObject *)pump)->getId().keyStr, pumpString);
    }
}

void HydroponicsLogger::logSystemUptime()
{
    TimeSpan elapsed(unixNow() - _initDate);

    if (elapsed.totalseconds()) {
        logMessage(SFP(HS_Log_SystemUptime), stringFromSpan(elapsed));
    }
}

void HydroponicsLogger::logSystemSave()
{
    logMessage(SFP(HS_Log_SystemDataSaved));
}

void HydroponicsLogger::logMessage(String msg, String suffix1, String suffix2)
{
    if (_loggerData && _loggerData->logLevel != Hydroponics_LogLevel_None && _loggerData->logLevel <= Hydroponics_LogLevel_All) {
        log(F("[INFO] "), msg, suffix1, suffix2);
    }
}

void HydroponicsLogger::logWarning(String warn, String suffix1, String suffix2)
{
    if (_loggerData && _loggerData->logLevel != Hydroponics_LogLevel_None && _loggerData->logLevel <= Hydroponics_LogLevel_Warnings) {
        log(F("[WARN] "), warn, suffix1, suffix2);
    }
}

void HydroponicsLogger::logError(String err, String suffix1, String suffix2)
{
    if (_loggerData && _loggerData->logLevel != Hydroponics_LogLevel_None && _loggerData->logLevel <= Hydroponics_LogLevel_Errors) {
        log(F("[FAIL] "), err, suffix1, suffix2);
    }
}

void HydroponicsLogger::log(String prefix, String &msg, String &suffix1, String &suffix2)
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

    if (getIsLoggingToSDCard()) {
        auto sd = getHydroponicsInstance()->getSDCard();

        if (sd) {
            auto logFile = sd->open(_logFileName, FILE_WRITE);

            if (logFile && logFile.availableForWrite()) {
                logFile.print(getCurrentTime().timestamp(DateTime::TIMESTAMP_FULL));
                logFile.print(' ');
                logFile.print(prefix);
                logFile.print(msg);
                logFile.print(suffix1);
                logFile.println(suffix2);
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

time_t HydroponicsLogger::getSystemUptime() const
{
    return unixNow() - _initDate ?: SECONDS_FROM_1970_TO_2000;
}

void HydroponicsLogger::notifyDayChanged()
{
    if (getIsLoggingEnabled()) {
        _logFileName = getYYMMDDFilename(stringFromChars(_loggerData->logFilePrefix, 16), SFP(HS_txt));
        cleanupOldestLogs();
    }
}

void HydroponicsLogger::updateInitTracking()
{
    _initDate = unixNow();
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
