/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Utilities
*/

#include "Hydruino.h"
#include <pins_arduino.h>

bool HydroRTCWrapper<RTC_DS1307>::begin(TwoWire *wireInstance)
{
    return _rtc.begin(wireInstance);
}

void HydroRTCWrapper<RTC_DS1307>::adjust(const DateTime &dt)
{
    _rtc.adjust(dt);
}

bool HydroRTCWrapper<RTC_DS1307>::lostPower(void)
{
    return false; // not implemented
}

DateTime HydroRTCWrapper<RTC_DS1307>::now()
{
    return _rtc.now();
}


#ifdef HYDRO_USE_MULTITASKING

BasicArduinoInterruptAbstraction interruptImpl;


ActuatorTimedEnableTask::ActuatorTimedEnableTask(SharedPtr<HydroActuator> actuator, float intensity, millis_t duration)
    : taskId(TASKMGR_INVALIDID), _actuator(actuator), _intensity(intensity), _duration(duration)
{ ; }

void ActuatorTimedEnableTask::exec()
{
    HydroActivationHandle handle = _actuator->enableActuator(_intensity, _duration);

    while (!handle.isDone()) {
        handle.elapseTo();
        if (handle.getTimeLeft() > HYDRO_SYS_DELAYFINE_SPINMILLIS) { yield(); }
    }

    // Custom run loop allows calling this method directly - will disable actuator if needed
    _actuator->update();
}

taskid_t scheduleActuatorTimedEnableOnce(SharedPtr<HydroActuator> actuator, float intensity, time_t enableTime)
{
    ActuatorTimedEnableTask *enableTask = actuator ? new ActuatorTimedEnableTask(actuator, intensity, enableTime) : nullptr;
    HYDRO_SOFT_ASSERT(!actuator || enableTask, SFP(HStr_Err_AllocationFailure));
    taskid_t retVal = enableTask ? taskManager.scheduleOnce(0, enableTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (enableTask ? (enableTask->taskId = retVal) : retVal);
}

taskid_t scheduleActuatorTimedEnableOnce(SharedPtr<HydroActuator> actuator, time_t enableTime)
{
    ActuatorTimedEnableTask *enableTask = actuator ? new ActuatorTimedEnableTask(actuator, 1.0f, enableTime) : nullptr;
    HYDRO_SOFT_ASSERT(!actuator || enableTask, SFP(HStr_Err_AllocationFailure));
    taskid_t retVal = enableTask ? taskManager.scheduleOnce(0, enableTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (enableTask ? (enableTask->taskId = retVal) : retVal);
}

#endif // /ifdef HYDRO_USE_MULTITASKING


#ifdef HYDRO_USE_DEBUG_ASSERTIONS

static String fileFromFullPath(String fullPath)
{
    int index = fullPath.lastIndexOf(HYDRO_BLDPATH_SEPARATOR);
    return index != -1 ? fullPath.substring(index+1) : fullPath;
}

static String makeAssertMsg(const char *file, const char *func, int line)
{
    String retVal;

    retVal.concat(fileFromFullPath(String(file)));
    retVal.concat(':');
    retVal.concat(line);
    retVal.concat(F(" in "));
    retVal.concat(func);
    retVal.concat(SFP(HStr_ColonSpace));

    return retVal;
}

void softAssert(bool cond, String msg, const char *file, const char *func, int line)
{
    if (!cond) {
        String assertMsg = makeAssertMsg(file, func, line);
        if (getLogger()) {
            getLogger()->logWarning(SFP(HStr_Err_AssertionFailure), SFP(HStr_ColonSpace), assertMsg);
            getLogger()->logWarning(SFP(HStr_DoubleSpace), msg);
            getLogger()->flush();
        }
        #ifdef HYDRO_ENABLE_DEBUG_OUTPUT
            else if (Serial) {
                Serial.print(localNow().timestamp(DateTime::TIMESTAMP_FULL));
                Serial.print(' ');
                Serial.print(SFP(HStr_Log_Prefix_Warning));
                Serial.print(SFP(HStr_Err_AssertionFailure));
                Serial.print(SFP(HStr_ColonSpace));
                Serial.println(assertMsg);
                Serial.flush(); yield();
            }
        #endif
    }
}

void hardAssert(bool cond, String msg, const char *file, const char *func, int line)
{
    if (!cond) {
        String assertMsg = makeAssertMsg(file, func, line);
        String colonSpace = String(F(" HARD")) + SFP(HStr_ColonSpace);
        if (getLogger()) {
            getLogger()->logError(SFP(HStr_Err_AssertionFailure), colonSpace, assertMsg);
            getLogger()->logError(SFP(HStr_DoubleSpace), msg);
            getLogger()->flush();
        }
        #ifdef HYDRO_ENABLE_DEBUG_OUTPUT
            else if (Serial) {
                Serial.print(localNow().timestamp(DateTime::TIMESTAMP_FULL));
                Serial.print(' ');
                Serial.print(SFP(HStr_Log_Prefix_Error));
                Serial.print(SFP(HStr_Err_AssertionFailure));
                Serial.print(colonSpace);
                Serial.println(assertMsg);
                Serial.flush(); yield();
            }
        #endif

        if (getController()) { getController()->suspend(); }
        yield(); delay(10);
        abort();
    }
}

#endif // /ifdef HYDRO_USE_DEBUG_ASSERTIONS


void publishData(HydroSensor *sensor)
{
    HYDRO_HARD_ASSERT(sensor, SFP(HStr_Err_InvalidParameter));

    if (getPublisher()) {
        auto measurement = sensor->getMeasurement();
        hposi_t rows = getMeasurementRowCount(measurement);
        hposi_t columnIndexStart = getPublisher()->getColumnIndexStart(sensor->getKey());

        if (columnIndexStart >= 0) {
            for (uint8_t measurementRow = 0; measurementRow < rows; ++measurementRow) {
                getPublisher()->publishData(columnIndexStart + measurementRow, getAsSingleMeasurement(measurement, measurementRow));
            }
        }
    }
}

bool setUnixTime(DateTime unixTime)
{
    auto rtc = getController() ? getController()->getRTC() : nullptr;
    if (rtc) {
        rtc->adjust(unixTime);
        getController()->notifyRTCTimeUpdated();
        return true;
    }
    return false;
}

String getYYMMDDFilename(String prefix, String ext)
{
    DateTime currTime = localNow();
    uint8_t yy = currTime.year() % 100;
    uint8_t mm = currTime.month();
    uint8_t dd = currTime.day();

    String retVal; retVal.reserve(prefix.length() + 11);

    retVal.concat(prefix);
    if (yy < 10) { retVal.concat('0'); }
    retVal.concat(yy);
    if (mm < 10) { retVal.concat('0'); }
    retVal.concat(mm);
    if (dd < 10) { retVal.concat('0'); }
    retVal.concat(dd);
    retVal.concat('.');
    retVal.concat(ext);

    return retVal;
}

String getNNFilename(String prefix, unsigned int value, String ext)
{
    String retVal; retVal.reserve(prefix.length() + 7);

    retVal.concat(prefix);
    if (value < 10) { retVal.concat('0'); }
    retVal.concat(value);
    retVal.concat('.');
    retVal.concat(ext);

    return retVal;
}

void createDirectoryFor(SDClass *sd, String filename)
{
    auto slashIndex = filename.indexOf(HYDRO_FSPATH_SEPARATOR);
    String directory = slashIndex != -1 ? filename.substring(0, slashIndex) : String();
    String dirWithSep = directory + String(HYDRO_FSPATH_SEPARATOR);
    if (directory.length() && !sd->exists(dirWithSep.c_str())) {
        sd->mkdir(directory.c_str());
    }
}

hkey_t stringHash(String string)
{
    hkey_t hash = 5381;
    for(int index = 0; index < string.length(); ++index) {
        hash = ((hash << 5) + hash) + (hkey_t)string[index]; // Good 'ol DJB2
    }
    return hash != hkey_none ? hash : 5381;
}

String addressToString(uintptr_t addr)
{
    String retVal; retVal.reserve((2 * sizeof(void*)) + 3);
    if (addr == (uintptr_t)-1) { addr = 0; }
    retVal.concat('0'); retVal.concat('x');

    if (sizeof(void*) >= 4) {
        if (addr < 0x10000000) { retVal.concat('0'); }
        if (addr <  0x1000000) { retVal.concat('0'); }
        if (addr <   0x100000) { retVal.concat('0'); }
        if (addr <    0x10000) { retVal.concat('0'); }
    }
    if (sizeof(void*) >= 2) {
        if (addr <     0x1000) { retVal.concat('0'); }
        if (addr <      0x100) { retVal.concat('0'); }
    }
    if (sizeof(void*) >= 1) {
        if (addr <       0x10) { retVal.concat('0'); }
    }

    retVal.concat(String((unsigned long)addr, 16));

    return retVal;
}

String charsToString(const char *charsIn, size_t length)
{
    if (!charsIn || !length) { return String(SFP(HStr_null)); }
    String retVal; retVal.reserve(length + 1);
    for (size_t index = 0; index < length && charsIn[index] != '\0'; ++index) {
        retVal.concat(charsIn[index]);
    }
    return retVal.length() ? retVal : String(SFP(HStr_null));
}

String timeSpanToString(const TimeSpan &span)
{
    String retVal; retVal.reserve(12);

    if (span.days()) {
        retVal.concat(span.days());
        retVal.concat('d');
    }
    if (span.hours()) {
        if (retVal.length()) { retVal.concat(' '); }
        retVal.concat(span.hours());
        retVal.concat('h');
    }
    if (span.minutes()) {
        if (retVal.length()) { retVal.concat(' '); }
        retVal.concat(span.minutes());
        retVal.concat('m');
    }
    if (span.seconds()) {
        if (retVal.length()) { retVal.concat(' '); }
        retVal.concat(span.seconds());
        retVal.concat('s');
    }

    return retVal;
}

extern String measurementToString(float value, Hydro_UnitsType units, unsigned int additionalDecPlaces)
{
    String retVal; retVal.reserve(12);
    retVal.concat(roundToString(value, additionalDecPlaces));

    String unitsSym = unitsTypeToSymbol(units, true); // also excludes dimensionless, e.g. pH
    if (unitsSym.length()) {
        retVal.concat(' ');
        retVal.concat(unitsSym);
    }

    return retVal;
}

template<>
String commaStringFromArray<float>(const float *arrayIn, size_t length)
{
    if (!arrayIn || !length) { return String(SFP(HStr_null)); }
    String retVal; retVal.reserve(length << 1);
    for (size_t index = 0; index < length; ++index) {
        if (retVal.length()) { retVal.concat(','); }

        String floatString = String(arrayIn[index], 6);
        int trimIndex = floatString.length() - 1;

        while (floatString[trimIndex] == '0' && trimIndex > 0) { trimIndex--; }
        if (floatString[trimIndex] == '.' && trimIndex > 0) { trimIndex--; }
        if (trimIndex < floatString.length() - 1) {
            floatString = floatString.substring(0, trimIndex+1);
        }

        retVal += floatString;
    }
    return retVal.length() ? retVal : String(SFP(HStr_null));
}

template<>
String commaStringFromArray<double>(const double *arrayIn, size_t length)
{
    if (!arrayIn || !length) { return String(SFP(HStr_null)); }
    String retVal; retVal.reserve(length << 1);
    for (size_t index = 0; index < length; ++index) {
        if (retVal.length()) { retVal.concat(','); }

        String doubleString = String(arrayIn[index], 14);
        int trimIndex = doubleString.length() - 1;

        while (doubleString[trimIndex] == '0' && trimIndex > 0) { trimIndex--; }
        if (doubleString[trimIndex] == '.' && trimIndex > 0) { trimIndex--; }
        if (trimIndex < doubleString.length() - 1) {
            doubleString = doubleString.substring(0, trimIndex+1);
        }

        retVal += doubleString;
    }
    return retVal.length() ? retVal : String(SFP(HStr_null));
}

template<>
void commaStringToArray<float>(String stringIn, float *arrayOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(SFP(HStr_null))) { return; }
    int lastSepPos = -1;
    for (size_t index = 0; index < length; ++index) {
        int nextSepPos = stringIn.indexOf(',', lastSepPos+1);
        if (nextSepPos == -1) { nextSepPos = stringIn.length(); }
        String subString = stringIn.substring(lastSepPos+1, nextSepPos);
        if (nextSepPos < stringIn.length()) { lastSepPos = nextSepPos; }

        arrayOut[index] = subString.toFloat();
    }
}

template<>
void commaStringToArray<double>(String stringIn, double *arrayOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(SFP(HStr_null))) { return; }
    int lastSepPos = -1;
    for (size_t index = 0; index < length; ++index) {
        int nextSepPos = stringIn.indexOf(',', lastSepPos+1);
        if (nextSepPos == -1) { nextSepPos = stringIn.length(); }
        String subString = stringIn.substring(lastSepPos+1, nextSepPos);
        if (nextSepPos < stringIn.length()) { lastSepPos = nextSepPos; }

        #if !defined(CORE_TEENSY)
            arrayOut[index] = subString.toDouble();
        #else
            arrayOut[index] = subString.toFloat();
        #endif
    }
}

template<>
void commaStringToArray<String>(String stringIn, String *arrayOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(SFP(HStr_null))) { return; }
    int lastSepPos = -1;
    for (size_t index = 0; index < length; ++index) {
        int nextSepPos = stringIn.indexOf(',', lastSepPos+1);
        if (nextSepPos == -1) { nextSepPos = stringIn.length(); }
        String subString = stringIn.substring(lastSepPos+1, nextSepPos);
        if (nextSepPos < stringIn.length()) { lastSepPos = nextSepPos; }
        arrayOut[index] = subString;
    }
}

String hexStringFromBytes(const uint8_t *bytesIn, size_t length)
{
    if (!bytesIn || !length) { return String(SFP(HStr_null)); }
    String retVal; retVal.reserve((length << 1) + 1);
    for (size_t index = 0; index < length; ++index) {
        String valStr = String(bytesIn[index], 16);
        if (valStr.length() == 1) { valStr = String('0') + valStr; }

        retVal += valStr;
    }
    return retVal.length() ? retVal : String(SFP(HStr_null));
}

void hexStringToBytes(String stringIn, uint8_t *bytesOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(SFP(HStr_null))) { return; }
    for (size_t index = 0; index < length; ++index) {
        String valStr = stringIn.substring(index << 1,(index+1) << 1);
        if (valStr.length() == 2) { bytesOut[index] = strtoul(valStr.c_str(), nullptr, 16); }
        else { bytesOut[index] = 0; }
    }
}

void hexStringToBytes(JsonVariantConst &variantIn, uint8_t *bytesOut, size_t length)
{
    if (variantIn.isNull() || variantIn.is<JsonObjectConst>() || variantIn.is<JsonArrayConst>()) { return; }
    hexStringToBytes(variantIn.as<String>(), bytesOut, length);
}

int occurrencesInString(String string, char singleChar)
{
    int retVal = 0;
    int posIndex = string.indexOf(singleChar);
    while (posIndex != -1) {
        retVal++;
        posIndex = string.indexOf(singleChar, posIndex+1);
    }
    return retVal;
}

int occurrencesInString(String string, String subString)
{
    int retVal = 0;
    int posIndex = string.indexOf(subString[0]);
    while (posIndex != -1) {
        if (subString.equals(string.substring(posIndex, posIndex + subString.length()))) {
            retVal++;
            posIndex += subString.length();
        }
        posIndex = string.indexOf(subString[0], posIndex+1);
    }
    return retVal;
}

int occurrencesInStringIgnoreCase(String string, char singleChar)
{
    int retVal = 0;
    int posIndex = min(string.indexOf(tolower(singleChar)), string.indexOf(toupper(singleChar)));
    while (posIndex != -1) {
        retVal++;
        posIndex = min(string.indexOf(tolower(singleChar), posIndex+1), string.indexOf(toupper(singleChar), posIndex+1));
    }
    return retVal;
}

int occurrencesInStringIgnoreCase(String string, String subString)
{
    int retVal = 0;
    int posIndex = min(string.indexOf(tolower(subString[0])), string.indexOf(toupper(subString[0])));
    while (posIndex != -1) {
        if (subString.equalsIgnoreCase(string.substring(posIndex, posIndex + subString.length()))) {
            retVal++;
            posIndex += subString.length();
        }
        posIndex = min(string.indexOf(tolower(subString[0]), posIndex+1), string.indexOf(toupper(subString[0]), posIndex+1));
    }
    return retVal;
}

template<>
bool arrayElementsEqual<float>(const float *arrayIn, size_t length, float value)
{
    for (size_t index = 0; index < length; ++index) {
        if (!isFPEqual(arrayIn[index], value)) {
            return false;
        }
    }
    return true;
}

template<>
bool arrayElementsEqual<double>(const double *arrayIn, size_t length, double value)
{
    for (size_t index = 0; index < length; ++index) {
        if (!isFPEqual(arrayIn[index], value)) {
            return false;
        }
    }
    return true;
}

// See: https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#elif !defined(ESP_PLATFORM)
extern char *__brkval;
#elif defined(ESP8266)
extern "C" {
#include "user_interface.h"
}
#endif

unsigned int freeMemory() {
    #if defined(ESP32)
        return esp_get_free_heap_size();
    #elif defined(ESP8266)
        return system_get_free_heap_size();
    #else
        char top;
        #ifdef __arm__
            return &top - reinterpret_cast<char*>(sbrk(0));
        #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
            return &top - __brkval;
        #else
            return __brkval ? &top - __brkval : &top - __malloc_heap_start;
        #endif
        return 0;
    #endif
}

void delayFine(millis_t duration) {
    millis_t start = millis();
    millis_t end = start + duration;

    {   millis_t left = max(0, duration - HYDRO_SYS_DELAYFINE_SPINMILLIS);
        if (left > 0) { delay(left); }
    }

    {   millis_t time = millis();
        while ((end >= start && (time < end)) ||
               (end < start && (time >= start || time < end))) {
            time = millis();
        }
    }
}

static void hy_bin_pinMode_def(pintype_t pin, uint8_t mode)
{
    pinMode(pin, mode);
}

static void hy_bin_digitalWrite_def(pintype_t pin, uint8_t status)
{
    digitalWrite(pin, status);
}

static uint8_t hy_bin_digitalRead_def(pintype_t pin)
{
    return digitalRead(pin);
}

void (*hy_bin_pinMode)(pintype_t,uint8_t) = &hy_bin_pinMode_def;
void (*hy_bin_digitalWrite)(pintype_t,uint8_t) = &hy_bin_digitalWrite_def;
uint8_t (*hy_bin_digitalRead)(pintype_t) = &hy_bin_digitalRead_def;

bool tryConvertUnits(float valueIn, Hydro_UnitsType unitsIn, float *valueOut, Hydro_UnitsType unitsOut, float convertParam)
{
    if (!valueOut || unitsOut == Hydro_UnitsType_Undefined || unitsIn == unitsOut) return false;

    switch (unitsIn) {
        case Hydro_UnitsType_Raw_1:
            switch (unitsOut) {
                // Known extents

                case Hydro_UnitsType_Percentile_100:
                    *valueOut = valueIn * 100.0;
                    return true;

                case Hydro_UnitsType_Alkalinity_pH_14:
                    *valueOut = valueIn * 14.0;
                    return true;

                case Hydro_UnitsType_Concentration_EC_5:
                    *valueOut = valueIn * 5.0;
                    return true;

                case Hydro_UnitsType_Concentration_PPM_500:
                    *valueOut = valueIn * (5.0 * 500.0);
                    return true;

                case Hydro_UnitsType_Concentration_PPM_640:
                    *valueOut = valueIn * (5.0 * 640.0);
                    return true;

                case Hydro_UnitsType_Concentration_PPM_700:
                    *valueOut = valueIn * (5.0 * 700.0);
                    return true;

                default:
                    if (convertParam != FLT_UNDEF) {
                        *valueOut = valueIn * convertParam;
                        return true;
                    }
                    break;
            }
            break;

        case Hydro_UnitsType_Percentile_100:
            switch (unitsOut) {
                case Hydro_UnitsType_Raw_1:
                    *valueOut = valueIn / 100.0;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_Alkalinity_pH_14:
            switch (unitsOut) {
                case Hydro_UnitsType_Raw_1:
                    *valueOut = valueIn / 14.0;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_Concentration_EC_5:
            switch (unitsOut) {
                case Hydro_UnitsType_Raw_1:
                    *valueOut = valueIn / 5.0;
                    return true;

                case Hydro_UnitsType_Concentration_PPM_500:
                    *valueOut = valueIn * 500.0;
                    return true;

                case Hydro_UnitsType_Concentration_PPM_640:
                    *valueOut = valueIn * 640.0;
                    return true;

                case Hydro_UnitsType_Concentration_PPM_700:
                    *valueOut = valueIn * 700.0;
                    return true;

                default:
                    break;
            }
            break;
        
        case Hydro_UnitsType_Concentration_PPM_500:
            switch (unitsOut) {
                case Hydro_UnitsType_Raw_1:
                    *valueOut = valueIn / (5.0 * 500.0);
                    return true;

                case Hydro_UnitsType_Concentration_EC_5:
                    *valueOut = valueIn / 500.0;
                    return true;

                case Hydro_UnitsType_Concentration_PPM_640:
                    *valueOut = valueIn / 500.0 * 640.0;
                    return true;

                case Hydro_UnitsType_Concentration_PPM_700:
                    *valueOut = valueIn / 500.0 * 700.0;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_Concentration_PPM_640:
            switch (unitsOut) {
                case Hydro_UnitsType_Raw_1:
                    *valueOut = valueIn / (5.0 * 640.0);
                    return true;

                case Hydro_UnitsType_Concentration_EC_5:
                    *valueOut = valueIn / 640.0;
                    return true;

                case Hydro_UnitsType_Concentration_PPM_500:
                    *valueOut = valueIn / 640.0 * 500.0;
                    return true;

                case Hydro_UnitsType_Concentration_PPM_700:
                    *valueOut = valueIn / 640.0 * 700.0;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_Concentration_PPM_700:
            switch (unitsOut) {
                case Hydro_UnitsType_Raw_1:
                    *valueOut = valueIn / (5.0 * 700.0);
                    return true;

                case Hydro_UnitsType_Concentration_EC_5:
                    *valueOut = valueIn / 700.0;
                    return true;

                case Hydro_UnitsType_Concentration_PPM_500:
                    *valueOut = valueIn / 700.0 * 500.0;
                    return true;

                case Hydro_UnitsType_Concentration_PPM_640:
                    *valueOut = valueIn / 700.0 * 640.0;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_Distance_Feet:
            switch (unitsOut) {
                case Hydro_UnitsType_Distance_Meters:
                    *valueOut = valueIn * 0.3048;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_Distance_Meters:
            switch (unitsOut) {
                case Hydro_UnitsType_Distance_Feet:
                    *valueOut = valueIn * 3.28084;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_LiqVolume_Gallons:
            switch (unitsOut) {
                case Hydro_UnitsType_LiqVolume_Liters:
                    *valueOut = valueIn * 3.78541;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_LiqVolume_Liters:
            switch (unitsOut) {
                case Hydro_UnitsType_LiqVolume_Gallons:
                    *valueOut = valueIn * 0.264172;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_LiqFlowRate_GallonsPerMin:
            switch (unitsOut) {
                case Hydro_UnitsType_LiqFlowRate_LitersPerMin:
                    *valueOut = valueIn * 3.78541;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_LiqFlowRate_LitersPerMin:
            switch (unitsOut) {
                case Hydro_UnitsType_LiqFlowRate_GallonsPerMin:
                    *valueOut = valueIn * 0.264172;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_LiqDilution_MilliLiterPerGallon:
            switch (unitsOut) {
                case Hydro_UnitsType_LiqDilution_MilliLiterPerLiter:
                    *valueOut = valueIn * 0.264172;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_LiqDilution_MilliLiterPerLiter:
            switch (unitsOut) {
                case Hydro_UnitsType_LiqDilution_MilliLiterPerGallon:
                    *valueOut = valueIn * 3.78541;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_Power_Amperage:
            switch (unitsOut) {
                case Hydro_UnitsType_Power_Wattage:
                    if (convertParam != FLT_UNDEF) { // convertParam = rail voltage
                        *valueOut = valueIn * convertParam;
                        return true;
                    }
                break;
            }
            break;

        case Hydro_UnitsType_Power_Wattage:
            switch (unitsOut) {
                case Hydro_UnitsType_Power_Amperage:
                    if (convertParam != FLT_UNDEF) { // convertParam = rail voltage
                        *valueOut = valueIn / convertParam;
                        return true;
                    }
                break;
            }
            break;

        case Hydro_UnitsType_Temperature_Celsius:
            switch (unitsOut) {
                case Hydro_UnitsType_Temperature_Fahrenheit:
                    *valueOut = valueIn * 1.8 + 32.0;
                    return true;

                case Hydro_UnitsType_Temperature_Kelvin:
                    *valueOut = valueIn + 273.15;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_Temperature_Fahrenheit:
            switch (unitsOut) {
                case Hydro_UnitsType_Temperature_Celsius:
                    *valueOut = (valueIn - 32.0) / 1.8;
                    return true;

                case Hydro_UnitsType_Temperature_Kelvin:
                    *valueOut = ((valueIn + 459.67) * 5.0) / 9.0;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_Temperature_Kelvin:
            switch (unitsOut) {
                case Hydro_UnitsType_Temperature_Celsius:
                    *valueOut = valueIn - 273.15;
                    return true;

                case Hydro_UnitsType_Temperature_Fahrenheit:
                    *valueOut = ((valueIn * 9.0) / 5.0) - 459.67;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_Weight_Kilograms:
            switch (unitsOut) {
                case Hydro_UnitsType_Weight_Pounds:
                    *valueOut = valueIn * 2.20462;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_Weight_Pounds:
            switch (unitsOut) {
                case Hydro_UnitsType_Weight_Kilograms:
                    *valueOut = valueIn * 0.453592;
                    return true;

                default:
                    break;
            }
            break;

        case Hydro_UnitsType_Undefined:
            *valueOut = valueIn;
            return true;

        default:
            break;
    }

    return false;
}

Hydro_UnitsType baseUnits(Hydro_UnitsType units)
{
    switch (units) {
        case Hydro_UnitsType_LiqFlowRate_LitersPerMin:
        case Hydro_UnitsType_LiqDilution_MilliLiterPerLiter:
            return Hydro_UnitsType_LiqVolume_Liters;
        case Hydro_UnitsType_LiqFlowRate_GallonsPerMin:
        case Hydro_UnitsType_LiqDilution_MilliLiterPerGallon:
            return Hydro_UnitsType_LiqVolume_Gallons;
        default:
            return Hydro_UnitsType_Undefined;
    }
}

Hydro_UnitsType rateUnits(Hydro_UnitsType units)
{
    switch (units) {
        case Hydro_UnitsType_LiqVolume_Liters:
            return Hydro_UnitsType_LiqFlowRate_LitersPerMin;
        case Hydro_UnitsType_LiqVolume_Gallons:
            return Hydro_UnitsType_LiqFlowRate_GallonsPerMin;
        default:
            return Hydro_UnitsType_Undefined;
    }
}

Hydro_UnitsType volumeUnits(Hydro_UnitsType units)
{
    switch (units) {
        case Hydro_UnitsType_LiqDilution_MilliLiterPerLiter:
            return Hydro_UnitsType_LiqVolume_Liters;
        case Hydro_UnitsType_LiqDilution_MilliLiterPerGallon:
            return Hydro_UnitsType_LiqVolume_Gallons;
        default:
            return Hydro_UnitsType_Undefined;
    }
}

Hydro_UnitsType dilutionUnits(Hydro_UnitsType units)
{
    switch (units) {
        case Hydro_UnitsType_LiqDilution_MilliLiterPerLiter:
            return Hydro_UnitsType_LiqVolume_Liters;
        case Hydro_UnitsType_LiqDilution_MilliLiterPerGallon:
            return Hydro_UnitsType_LiqVolume_Gallons;
        default:
            return Hydro_UnitsType_Undefined;
    }
}

Hydro_UnitsType defaultUnits(Hydro_UnitsCategory unitsCategory, Hydro_MeasurementMode measureMode)
{
    measureMode = (measureMode == Hydro_MeasurementMode_Undefined && getController() ? getController()->getMeasurementMode() : measureMode);

    switch (unitsCategory) {
        case Hydro_UnitsCategory_Alkalinity:
            return Hydro_UnitsType_Alkalinity_pH_14;

        case Hydro_UnitsCategory_Concentration:
            return Hydro_UnitsType_Concentration_EC_5;

        case Hydro_UnitsCategory_Distance:
            switch (measureMode) {
                case Hydro_MeasurementMode_Imperial:
                    return Hydro_UnitsType_Distance_Feet;
                case Hydro_MeasurementMode_Metric:
                case Hydro_MeasurementMode_Scientific:
                    return Hydro_UnitsType_Distance_Meters;
                default:
                    return Hydro_UnitsType_Undefined;
            }

        case Hydro_UnitsCategory_LiqDilution:
            switch (measureMode) {
                case Hydro_MeasurementMode_Imperial:
                    return Hydro_UnitsType_LiqDilution_MilliLiterPerGallon;
                case Hydro_MeasurementMode_Metric:
                case Hydro_MeasurementMode_Scientific:
                    return Hydro_UnitsType_LiqDilution_MilliLiterPerLiter;
                default:
                    return Hydro_UnitsType_Undefined;
            }

        case Hydro_UnitsCategory_LiqFlowRate:
            switch (measureMode) {
                case Hydro_MeasurementMode_Imperial:
                    return Hydro_UnitsType_LiqFlowRate_GallonsPerMin;
                case Hydro_MeasurementMode_Metric:
                case Hydro_MeasurementMode_Scientific:
                    return Hydro_UnitsType_LiqFlowRate_LitersPerMin;
                default:
                    return Hydro_UnitsType_Undefined;
            }

        case Hydro_UnitsCategory_LiqVolume:
            switch (measureMode) {
                case Hydro_MeasurementMode_Imperial:
                    return Hydro_UnitsType_LiqVolume_Gallons;
                case Hydro_MeasurementMode_Metric:
                case Hydro_MeasurementMode_Scientific:
                    return Hydro_UnitsType_LiqVolume_Liters;
                default:
                    return Hydro_UnitsType_Undefined;
            }

        case Hydro_UnitsCategory_Percentile:
            return Hydro_UnitsType_Percentile_100;

        case Hydro_UnitsCategory_Power:
            return Hydro_UnitsType_Power_Wattage;

        case Hydro_UnitsCategory_Temperature:
            switch (measureMode) {
                case Hydro_MeasurementMode_Imperial:
                    return Hydro_UnitsType_Temperature_Fahrenheit;
                case Hydro_MeasurementMode_Metric:
                    return Hydro_UnitsType_Temperature_Celsius;
                case Hydro_MeasurementMode_Scientific:
                    return Hydro_UnitsType_Temperature_Kelvin;
                default:
                    return Hydro_UnitsType_Undefined;
            }

        case Hydro_UnitsCategory_Weight:
            switch (measureMode) {
                case Hydro_MeasurementMode_Imperial:
                    return Hydro_UnitsType_Weight_Pounds;
                case Hydro_MeasurementMode_Metric:
                case Hydro_MeasurementMode_Scientific:
                    return Hydro_UnitsType_Weight_Kilograms;
                default:
                    return Hydro_UnitsType_Undefined;
            }

        case Hydro_UnitsCategory_Count:
            switch (measureMode) {
                case Hydro_MeasurementMode_Scientific:
                    return (Hydro_UnitsType)2;
                default:
                    return (Hydro_UnitsType)1;
            }

        case Hydro_UnitsCategory_Undefined:
            return Hydro_UnitsType_Undefined;
    }
    return Hydro_UnitsType_Undefined;
}


int linksCountSowableCrops(Pair<uint8_t, Pair<HydroObject *, int8_t> *> links)
{
    int retVal = 0;

    for (hposi_t linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
        if (links.second[linksIndex].first->isCropType()) {
            auto crop = static_cast<HydroCrop *>(links.second[linksIndex].first);

            if (localNow() >= crop->getSowTime()) {
                retVal++;
            }
        }
    }

    return retVal;
}

int linksCountActuatorsByReservoirAndType(Pair<uint8_t, Pair<HydroObject *, int8_t> *> links, HydroReservoir *srcReservoir, Hydro_ActuatorType actuatorType)
{
    int retVal = 0;

    for (hposi_t linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
        if (links.second[linksIndex].first->isActuatorType()) {
            auto actuator = static_cast<HydroActuator *>(links.second[linksIndex].first);

            if (actuator->getActuatorType() == actuatorType && actuator->getParentReservoir().get() == srcReservoir) {
                retVal++;
            }
        }
    }

    return retVal;
}


bool checkPinIsAnalogInput(pintype_t pin)
{
    #if !defined(NUM_ANALOG_INPUTS) || NUM_ANALOG_INPUTS == 0
        return false;
    #else
        switch (pin) {
            #if NUM_ANALOG_INPUTS > 0
                case (pintype_t)A0:
            #endif
            #if NUM_ANALOG_INPUTS > 1 && !defined(ESP32) && !(defined(PIN_A0) && !defined(PIN_A1))
                case (pintype_t)A1:
            #endif
            #if NUM_ANALOG_INPUTS > 2 && !defined(ESP32) && !(defined(PIN_A0) && !defined(PIN_A2))
                case (pintype_t)A2:
            #endif
            #if NUM_ANALOG_INPUTS > 3 && !(defined(PIN_A0) && !defined(PIN_A3))
                case (pintype_t)A3:
            #endif
            #if NUM_ANALOG_INPUTS > 4 && !(defined(PIN_A0) && !defined(PIN_A4))
                case (pintype_t)A4:
            #endif
            #if NUM_ANALOG_INPUTS > 5 && !(defined(PIN_A0) && !defined(PIN_A5))
                case (pintype_t)A5:
            #endif
            #if NUM_ANALOG_INPUTS > 6 && !(defined(PIN_A0) && !defined(PIN_A6))
                case (pintype_t)A6:
            #endif
            #if NUM_ANALOG_INPUTS > 7 && !(defined(PIN_A0) && !defined(PIN_A7))
                case (pintype_t)A7:
            #endif
            #if NUM_ANALOG_INPUTS > 8 && !defined(ESP32) && !(defined(PIN_A0) && !defined(PIN_A8))
                case (pintype_t)A8:
            #endif
            #if NUM_ANALOG_INPUTS > 9 && !defined(ESP32) && !(defined(PIN_A0) && !defined(PIN_A9))
                case (pintype_t)A9:
            #endif
            #if NUM_ANALOG_INPUTS > 10 && !(defined(PIN_A0) && !defined(PIN_A10))
                case (pintype_t)A10:
            #endif
            #if NUM_ANALOG_INPUTS > 11 && !(defined(PIN_A0) && !defined(PIN_A11))
                case (pintype_t)A11:
            #endif
            #if NUM_ANALOG_INPUTS > 12 && !(defined(PIN_A0) && !defined(PIN_A12))
                case (pintype_t)A12:
            #endif
            #if NUM_ANALOG_INPUTS > 13 && !(defined(PIN_A0) && !defined(PIN_A13))
                case (pintype_t)A13:
            #endif
            #if NUM_ANALOG_INPUTS > 14 && !(defined(PIN_A0) && !defined(PIN_A14))
                case (pintype_t)A14:
            #endif
            #if NUM_ANALOG_INPUTS > 15 && !(defined(PIN_A0) && !defined(PIN_A15))
                case (pintype_t)A15:
            #endif
            #ifdef ESP32
                case (pintype_t)A16:
                case (pintype_t)A17:
                case (pintype_t)A18:
                case (pintype_t)A19:
            #endif
                return true;

            default:
                return false;
        }
    #endif
}

bool checkPinIsAnalogOutput(pintype_t pin)
{
    #if !defined(NUM_ANALOG_OUTPUTS) || NUM_ANALOG_OUTPUTS == 0
        return false;
    #else
        switch (pin) {
            #if NUM_ANALOG_OUTPUTS > 0
                #ifndef PIN_DAC0
                    case (pintype_t)A0:
                #else
                    case (pintype_t)DAC0:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 1
                #ifndef PIN_DAC1
                    case (pintype_t)A1:
                #else
                    case (pintype_t)DAC1:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 2
                #ifndef PIN_DAC2
                    case (pintype_t)A2:
                #else
                    case (pintype_t)DAC2:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 3
                #ifndef PIN_DAC3
                    case (pintype_t)A3:
                #else
                    case (pintype_t)DAC3:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 4
                #ifndef PIN_DAC4
                    case (pintype_t)A4:
                #else
                    case (pintype_t)DAC4:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 5
                #ifndef PIN_DAC5
                    case (pintype_t)A5:
                #else
                    case (pintype_t)DAC5:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 6
                #ifndef PIN_DAC6
                    case (pintype_t)A6:
                #else
                    case (pintype_t)DAC6:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 7
                #ifndef PIN_DAC7
                    case (pintype_t)A7:
                #else
                    case (pintype_t)DAC7:
                #endif
            #endif
                return true;

            default:
                return false;
        }
    #endif
}


String systemModeToString(Hydro_SystemMode systemMode, bool excludeSpecial)
{
    switch (systemMode) {
        case Hydro_SystemMode_Recycling:
            return SFP(HStr_Enum_Recycling);
        case Hydro_SystemMode_DrainToWaste:
            return SFP(HStr_Enum_DrainToWaste);
        case Hydro_SystemMode_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydro_SystemMode_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String measurementModeToString(Hydro_MeasurementMode measurementMode, bool excludeSpecial)
{
    switch (measurementMode) {
        case Hydro_MeasurementMode_Imperial:
            return SFP(HStr_Enum_Imperial);
        case Hydro_MeasurementMode_Metric:
            return SFP(HStr_Enum_Metric);
        case Hydro_MeasurementMode_Scientific:
            return SFP(HStr_Enum_Scientific);
        case Hydro_MeasurementMode_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydro_MeasurementMode_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String displayOutputModeToString(Hydro_DisplayOutputMode displayOutMode, bool excludeSpecial)
{
    switch (displayOutMode) {
        case Hydro_DisplayOutputMode_Disabled:
            return SFP(HStr_Disabled);
        case Hydro_DisplayOutputMode_20x4LCD:
            return SFP(HStr_Enum_20x4LCD);
        case Hydro_DisplayOutputMode_20x4LCD_Swapped:
            return SFP(HStr_Enum_20x4LCDSwapped);
        case Hydro_DisplayOutputMode_16x2LCD:
            return SFP(HStr_Enum_16x2LCD);
        case Hydro_DisplayOutputMode_16x2LCD_Swapped:
            return SFP(HStr_Enum_16x2LCDSwapped);
        case Hydro_DisplayOutputMode_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydro_DisplayOutputMode_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String controlInputModeToString(Hydro_ControlInputMode controlInMode, bool excludeSpecial)
{
    switch (controlInMode) {
        case Hydro_ControlInputMode_Disabled:
            return SFP(HStr_Disabled);
        case Hydro_ControlInputMode_2x2Matrix:
            return SFP(HStr_Enum_2x2Matrix);
        case Hydro_ControlInputMode_4xButton:
            return SFP(HStr_Enum_4xButton);
        case Hydro_ControlInputMode_6xButton:
            return SFP(HStr_Enum_6xButton);
        case Hydro_ControlInputMode_RotaryEncoder:
            return SFP(HStr_Enum_RotaryEncoder);
        case Hydro_ControlInputMode_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydro_ControlInputMode_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String actuatorTypeToString(Hydro_ActuatorType actuatorType, bool excludeSpecial)
{
    switch (actuatorType) {
        case Hydro_ActuatorType_FanExhaust:
            return SFP(HStr_Enum_FanExhaust);
        case Hydro_ActuatorType_GrowLights:
            return SFP(HStr_Enum_GrowLights);
        case Hydro_ActuatorType_PeristalticPump:
            return SFP(HStr_Enum_PeristalticPump);
        case Hydro_ActuatorType_WaterAerator:
            return SFP(HStr_Enum_WaterAerator);
        case Hydro_ActuatorType_WaterHeater:
            return SFP(HStr_Enum_WaterHeater);
        case Hydro_ActuatorType_WaterPump:
            return SFP(HStr_Enum_WaterPump);
        case Hydro_ActuatorType_WaterSprayer:
            return SFP(HStr_Enum_WaterSprayer);
        case Hydro_ActuatorType_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydro_ActuatorType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String sensorTypeToString(Hydro_SensorType sensorType, bool excludeSpecial)
{
    switch (sensorType) {
        case Hydro_SensorType_AirCarbonDioxide:
            return SFP(HStr_Enum_AirCarbonDioxide);
        case Hydro_SensorType_AirTempHumidity:
            return SFP(HStr_Enum_AirTemperatureHumidity);
        case Hydro_SensorType_PotentialHydrogen:
            return SFP(HStr_Enum_WaterPH);
        case Hydro_SensorType_PowerLevel:
            return SFP(HStr_Enum_PowerLevel);
        case Hydro_SensorType_PumpFlow:
            return SFP(HStr_Enum_PumpFlow);
        case Hydro_SensorType_SoilMoisture:
            return SFP(HStr_Enum_SoilMoisture);
        case Hydro_SensorType_TotalDissolvedSolids:
            return SFP(HStr_Enum_WaterTDS);
        case Hydro_SensorType_WaterHeight:
            return SFP(HStr_Enum_WaterHeight);
        case Hydro_SensorType_WaterLevel:
            return SFP(HStr_Enum_WaterLevel);
        case Hydro_SensorType_WaterTemperature:
            return SFP(HStr_Enum_WaterTemperature);
        case Hydro_SensorType_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydro_SensorType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String cropTypeToString(Hydro_CropType cropType, bool excludeSpecial)
{
    switch (cropType) {
        case Hydro_CropType_AloeVera:
            return SFP(HStr_Enum_AloeVera);
        case Hydro_CropType_Anise:
            return SFP(HStr_Enum_Anise);
        case Hydro_CropType_Artichoke:
            return SFP(HStr_Enum_Artichoke);
        case Hydro_CropType_Arugula:
            return SFP(HStr_Enum_Arugula);
        case Hydro_CropType_Asparagus:
            return SFP(HStr_Enum_Asparagus);
        case Hydro_CropType_Basil:
            return SFP(HStr_Enum_Basil);
        case Hydro_CropType_Bean:
            return SFP(HStr_Enum_Bean);
        case Hydro_CropType_BeanBroad:
            return SFP(HStr_Enum_BeanBroad);
        case Hydro_CropType_Beetroot:
            return SFP(HStr_Enum_Beetroot);
        case Hydro_CropType_BlackCurrant:
            return SFP(HStr_Enum_BlackCurrant);
        case Hydro_CropType_Blueberry:
            return SFP(HStr_Enum_Blueberry);
        case Hydro_CropType_BokChoi:
            return SFP(HStr_Enum_BokChoi);
        case Hydro_CropType_Broccoli:
            return SFP(HStr_Enum_Broccoli);
        case Hydro_CropType_BrusselsSprout:
            return SFP(HStr_Enum_BrusselsSprout);
        case Hydro_CropType_Cabbage:
            return SFP(HStr_Enum_Cabbage);
        case Hydro_CropType_Cannabis:
            return SFP(HStr_Enum_Cannabis);
        case Hydro_CropType_Capsicum:
            return SFP(HStr_Enum_Capsicum);
        case Hydro_CropType_Carrots:
            return SFP(HStr_Enum_Carrots);
        case Hydro_CropType_Catnip:
            return SFP(HStr_Enum_Catnip);
        case Hydro_CropType_Cauliflower:
            return SFP(HStr_Enum_Cauliflower);
        case Hydro_CropType_Celery:
            return SFP(HStr_Enum_Celery);
        case Hydro_CropType_Chamomile:
            return SFP(HStr_Enum_Chamomile);
        case Hydro_CropType_Chicory:
            return SFP(HStr_Enum_Chicory);
        case Hydro_CropType_Chives:
            return SFP(HStr_Enum_Chives);
        case Hydro_CropType_Cilantro:
            return SFP(HStr_Enum_Cilantro);
        case Hydro_CropType_Coriander:
            return SFP(HStr_Enum_Coriander);
        case Hydro_CropType_CornSweet:
            return SFP(HStr_Enum_CornSweet);
        case Hydro_CropType_Cucumber:
            return SFP(HStr_Enum_Cucumber);
        case Hydro_CropType_Dill:
            return SFP(HStr_Enum_Dill);
        case Hydro_CropType_Eggplant:
            return SFP(HStr_Enum_Eggplant);
        case Hydro_CropType_Endive:
            return SFP(HStr_Enum_Endive);
        case Hydro_CropType_Fennel:
            return SFP(HStr_Enum_Fennel);
        case Hydro_CropType_Fodder:
            return SFP(HStr_Enum_Fodder);
        case Hydro_CropType_Flowers:
            return SFP(HStr_Enum_Flowers);
        case Hydro_CropType_Garlic:
            return SFP(HStr_Enum_Garlic);
        case Hydro_CropType_Ginger:
            return SFP(HStr_Enum_Ginger);
        case Hydro_CropType_Kale:
            return SFP(HStr_Enum_Kale);
        case Hydro_CropType_Lavender:
            return SFP(HStr_Enum_Lavender);
        case Hydro_CropType_Leek:
            return SFP(HStr_Enum_Leek);
        case Hydro_CropType_LemonBalm:
            return SFP(HStr_Enum_LemonBalm);
        case Hydro_CropType_Lettuce:
            return SFP(HStr_Enum_Lettuce);
        case Hydro_CropType_Marrow:
            return SFP(HStr_Enum_Marrow);
        case Hydro_CropType_Melon:
            return SFP(HStr_Enum_Melon);
        case Hydro_CropType_Mint:
            return SFP(HStr_Enum_Mint);
        case Hydro_CropType_MustardCress:
            return SFP(HStr_Enum_MustardCress);
        case Hydro_CropType_Okra:
            return SFP(HStr_Enum_Okra);
        case Hydro_CropType_Onions:
            return SFP(HStr_Enum_Onions);
        case Hydro_CropType_Oregano:
            return SFP(HStr_Enum_Oregano);
        case Hydro_CropType_PakChoi:
            return SFP(HStr_Enum_PakChoi);
        case Hydro_CropType_Parsley:
            return SFP(HStr_Enum_Parsley);
        case Hydro_CropType_Parsnip:
            return SFP(HStr_Enum_Parsnip);
        case Hydro_CropType_Pea:
            return SFP(HStr_Enum_Pea);
        case Hydro_CropType_PeaSugar:
            return SFP(HStr_Enum_PeaSugar);
        case Hydro_CropType_Pepino:
            return SFP(HStr_Enum_Pepino);
        case Hydro_CropType_PeppersBell:
            return SFP(HStr_Enum_PeppersBell);
        case Hydro_CropType_PeppersHot:
            return SFP(HStr_Enum_PeppersHot);
        case Hydro_CropType_Potato:
            return SFP(HStr_Enum_Potato);
        case Hydro_CropType_PotatoSweet:
            return SFP(HStr_Enum_PotatoSweet);
        case Hydro_CropType_Pumpkin:
            return SFP(HStr_Enum_Pumpkin);
        case Hydro_CropType_Radish:
            return SFP(HStr_Enum_Radish);
        case Hydro_CropType_Rhubarb:
            return SFP(HStr_Enum_Rhubarb);
        case Hydro_CropType_Rosemary:
            return SFP(HStr_Enum_Rosemary);
        case Hydro_CropType_Sage:
            return SFP(HStr_Enum_Sage);
        case Hydro_CropType_Silverbeet:
            return SFP(HStr_Enum_Silverbeet);
        case Hydro_CropType_Spinach:
            return SFP(HStr_Enum_Spinach);
        case Hydro_CropType_Squash:
            return SFP(HStr_Enum_Squash);
        case Hydro_CropType_Sunflower:
            return SFP(HStr_Enum_Sunflower);
        case Hydro_CropType_Strawberries:
            return SFP(HStr_Enum_Strawberries);
        case Hydro_CropType_SwissChard:
            return SFP(HStr_Enum_SwissChard);
        case Hydro_CropType_Taro:
            return SFP(HStr_Enum_Taro);
        case Hydro_CropType_Tarragon:
            return SFP(HStr_Enum_Tarragon);
        case Hydro_CropType_Thyme:
            return SFP(HStr_Enum_Thyme);
        case Hydro_CropType_Tomato:
            return SFP(HStr_Enum_Tomato);
        case Hydro_CropType_Turnip:
            return SFP(HStr_Enum_Turnip);
        case Hydro_CropType_Watercress:
            return SFP(HStr_Enum_Watercress);
        case Hydro_CropType_Watermelon:
            return SFP(HStr_Enum_Watermelon);
        case Hydro_CropType_Zucchini:
            return SFP(HStr_Enum_Zucchini);
        case Hydro_CropType_CustomCrop1:
            return SFP(HStr_Enum_CustomCrop1);
        case Hydro_CropType_CustomCrop2:
            return SFP(HStr_Enum_CustomCrop2);
        case Hydro_CropType_CustomCrop3:
            return SFP(HStr_Enum_CustomCrop3);
        case Hydro_CropType_CustomCrop4:
            return SFP(HStr_Enum_CustomCrop4);
        case Hydro_CropType_CustomCrop5:
            return SFP(HStr_Enum_CustomCrop5);
        case Hydro_CropType_CustomCrop6:
            return SFP(HStr_Enum_CustomCrop6);
        case Hydro_CropType_CustomCrop7:
            return SFP(HStr_Enum_CustomCrop7);
        case Hydro_CropType_CustomCrop8:
            return SFP(HStr_Enum_CustomCrop8);
        case Hydro_CropType_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydro_CropType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String substrateTypeToString(Hydro_SubstrateType substrateType, bool excludeSpecial)
{
    switch (substrateType) {
        case Hydro_SubstrateType_ClayPebbles:
            return SFP(HStr_Enum_ClayPebbles);
        case Hydro_SubstrateType_CoconutCoir:
            return SFP(HStr_Enum_CoconutCoir);
        case Hydro_SubstrateType_Rockwool:
            return SFP(HStr_Enum_Rockwool);
        case Hydro_SubstrateType_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydro_SubstrateType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String reservoirTypeToString(Hydro_ReservoirType reservoirType, bool excludeSpecial)
{
    switch (reservoirType) {
        case Hydro_ReservoirType_FeedWater:
            return SFP(HStr_Enum_FeedWater);
        case Hydro_ReservoirType_DrainageWater:
            return SFP(HStr_Enum_DrainageWater);
        case Hydro_ReservoirType_NutrientPremix:
            return SFP(HStr_Enum_NutrientPremix);
        case Hydro_ReservoirType_FreshWater:
            return SFP(HStr_Enum_FreshWater);
        case Hydro_ReservoirType_PhUpSolution:
            return SFP(HStr_Enum_PhUpSolution);
        case Hydro_ReservoirType_PhDownSolution:
            return SFP(HStr_Enum_PhDownSolution);
        case Hydro_ReservoirType_CustomAdditive1:
            return SFP(HStr_Enum_CustomAdditive1);
        case Hydro_ReservoirType_CustomAdditive2:
            return SFP(HStr_Enum_CustomAdditive2);
        case Hydro_ReservoirType_CustomAdditive3:
            return SFP(HStr_Enum_CustomAdditive3);
        case Hydro_ReservoirType_CustomAdditive4:
            return SFP(HStr_Enum_CustomAdditive4);
        case Hydro_ReservoirType_CustomAdditive5:
            return SFP(HStr_Enum_CustomAdditive5);
        case Hydro_ReservoirType_CustomAdditive6:
            return SFP(HStr_Enum_CustomAdditive6);
        case Hydro_ReservoirType_CustomAdditive7:
            return SFP(HStr_Enum_CustomAdditive7);
        case Hydro_ReservoirType_CustomAdditive8:
            return SFP(HStr_Enum_CustomAdditive8);
        case Hydro_ReservoirType_CustomAdditive9:
            return SFP(HStr_Enum_CustomAdditive9);
        case Hydro_ReservoirType_CustomAdditive10:
            return SFP(HStr_Enum_CustomAdditive10);
        case Hydro_ReservoirType_CustomAdditive11:
            return SFP(HStr_Enum_CustomAdditive11);
        case Hydro_ReservoirType_CustomAdditive12:
            return SFP(HStr_Enum_CustomAdditive12);
        case Hydro_ReservoirType_CustomAdditive13:
            return SFP(HStr_Enum_CustomAdditive13);
        case Hydro_ReservoirType_CustomAdditive14:
            return SFP(HStr_Enum_CustomAdditive14);
        case Hydro_ReservoirType_CustomAdditive15:
            return SFP(HStr_Enum_CustomAdditive15);
        case Hydro_ReservoirType_CustomAdditive16:
            return SFP(HStr_Enum_CustomAdditive16);
        case Hydro_ReservoirType_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydro_ReservoirType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

float getRailVoltageFromType(Hydro_RailType railType)
{
    switch (railType) {
        case Hydro_RailType_AC110V:
            return 110.0f;
        case Hydro_RailType_AC220V:
            return 220.0f;
        case Hydro_RailType_DC3V3:
            return 3.3f;
        case Hydro_RailType_DC5V:
            return 5.0f;
        case Hydro_RailType_DC12V:
            return 12.0f;
        case Hydro_RailType_DC24V:
            return 24.0f;
        case Hydro_RailType_DC48V:
            return 48.0f;
        default:
            return 0.0f;
    }
}

String railTypeToString(Hydro_RailType railType, bool excludeSpecial)
{
    switch (railType) {
        case Hydro_RailType_AC110V:
            return SFP(HStr_Enum_AC110V);
        case Hydro_RailType_AC220V:
            return SFP(HStr_Enum_AC220V);
        case Hydro_RailType_DC3V3:
            return SFP(HStr_Enum_DC3V3);
        case Hydro_RailType_DC5V:
            return SFP(HStr_Enum_DC5V);
        case Hydro_RailType_DC12V:
            return SFP(HStr_Enum_DC12V);
        case Hydro_RailType_DC24V:
            return SFP(HStr_Enum_DC24V);
        case Hydro_RailType_DC48V:
            return SFP(HStr_Enum_DC48V);
        case Hydro_RailType_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydro_RailType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String pinModeToString(Hydro_PinMode pinMode, bool excludeSpecial)
{
    switch (pinMode) {
        case Hydro_PinMode_Digital_Input:
            return SFP(HStr_Enum_DigitalInput);
        case Hydro_PinMode_Digital_Input_PullUp:
            return SFP(HStr_Enum_DigitalInputPullUp);
        case Hydro_PinMode_Digital_Input_PullDown:
            return SFP(HStr_Enum_DigitalInputPullDown);
        case Hydro_PinMode_Digital_Output:
            return SFP(HStr_Enum_DigitalOutput);
        case Hydro_PinMode_Digital_Output_PushPull:
            return SFP(HStr_Enum_DigitalOutputPushPull);
        case Hydro_PinMode_Analog_Input:
            return SFP(HStr_Enum_AnalogInput);
        case Hydro_PinMode_Analog_Output:
            return SFP(HStr_Enum_AnalogOutput);
        case Hydro_PinMode_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydro_PinMode_Undefined:
            break;
        default:
            return String((int)pinMode);
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String enableModeToString(Hydro_EnableMode enableMode, bool excludeSpecial)
{
    switch (enableMode) {
        case Hydro_EnableMode_Highest:
            return SFP(HStr_Enum_Highest);
        case Hydro_EnableMode_Lowest:
            return SFP(HStr_Enum_Lowest);
        case Hydro_EnableMode_Average:
            return SFP(HStr_Enum_Average);
        case Hydro_EnableMode_Multiply:
            return SFP(HStr_Enum_Multiply);
        case Hydro_EnableMode_InOrder:
            return SFP(HStr_Enum_InOrder);
        case Hydro_EnableMode_RevOrder:
            return SFP(HStr_Enum_RevOrder);
        case Hydro_EnableMode_DesOrder:
            return SFP(HStr_Enum_DesOrder);
        case Hydro_EnableMode_AscOrder:
            return SFP(HStr_Enum_AscOrder);
        case Hydro_EnableMode_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydro_EnableMode_Undefined:
            break;
        default:
            return String((int)enableMode);
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String unitsCategoryToString(Hydro_UnitsCategory unitsCategory, bool excludeSpecial)
{
    switch (unitsCategory) {
        case Hydro_UnitsCategory_Alkalinity:
            return SFP(HStr_Enum_Alkalinity);
        case Hydro_UnitsCategory_Concentration:
            return SFP(HStr_Enum_Concentration);
        case Hydro_UnitsCategory_Distance:
            return SFP(HStr_Enum_Distance);
        case Hydro_UnitsCategory_LiqDilution:
            return SFP(HStr_Enum_LiqDilution);
        case Hydro_UnitsCategory_LiqFlowRate:
            return SFP(HStr_Enum_LiqFlowRate);
        case Hydro_UnitsCategory_LiqVolume:
            return SFP(HStr_Enum_LiqVolume);
        case Hydro_UnitsCategory_Percentile:
            return SFP(HStr_Enum_Percentile);
        case Hydro_UnitsCategory_Power:
            return SFP(HStr_Enum_Power);
        case Hydro_UnitsCategory_Temperature:
            return SFP(HStr_Enum_Temperature);
        case Hydro_UnitsCategory_Weight:
            return SFP(HStr_Enum_Weight);
        case Hydro_UnitsCategory_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydro_UnitsCategory_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String unitsTypeToSymbol(Hydro_UnitsType unitsType, bool excludeSpecial)
{
    switch (unitsType) {
        case Hydro_UnitsType_Raw_1:
            return SFP(HStr_raw);
        case Hydro_UnitsType_Percentile_100:
            return String('%');
        case Hydro_UnitsType_Alkalinity_pH_14:
            return !excludeSpecial ? SFP(HStr_Unit_pH14) : String(); // technically unitless
        case Hydro_UnitsType_Concentration_EC_5:
            return SFP(HStr_Unit_EC5); // alt: mS/cm, TDS
        case Hydro_UnitsType_Concentration_PPM_500:
            return SFP(HStr_Unit_PPM500);
        case Hydro_UnitsType_Concentration_PPM_640:
            return SFP(HStr_Unit_PPM640);
        case Hydro_UnitsType_Concentration_PPM_700:
            return SFP(HStr_Unit_PPM700);
        case Hydro_UnitsType_Distance_Feet:
            return SFP(HStr_Unit_Feet);
        case Hydro_UnitsType_Distance_Meters:
            return String('m');
        case Hydro_UnitsType_LiqDilution_MilliLiterPerGallon: {
            String retVal(SFP(HStr_Unit_MilliLiterPer));
            retVal.concat(SFP(HStr_Unit_Gallons));
            return retVal;
        }
        case Hydro_UnitsType_LiqDilution_MilliLiterPerLiter: {
            String retVal(SFP(HStr_Unit_MilliLiterPer));
            retVal.concat('L');
            return retVal;
        }
        case Hydro_UnitsType_LiqFlowRate_GallonsPerMin: {
            String retVal(SFP(HStr_Unit_Gallons));
            retVal.concat(SFP(HStr_Unit_PerMinute));
            return retVal;
        }
        case Hydro_UnitsType_LiqFlowRate_LitersPerMin: {
            String retVal('L');
            retVal.concat(SFP(HStr_Unit_PerMinute));
            return retVal;
        }
        case Hydro_UnitsType_LiqVolume_Gallons:
            return SFP(HStr_Unit_Gallons);
        case Hydro_UnitsType_LiqVolume_Liters:
            return String('L');
        case Hydro_UnitsType_Power_Amperage:
            return String('A');
        case Hydro_UnitsType_Power_Wattage:
            return String('W'); // alt: J/s
        case Hydro_UnitsType_Temperature_Celsius: {
            String retVal(SFP(HStr_Unit_Degree));
            retVal.concat('C');
            return retVal;
        }
        case Hydro_UnitsType_Temperature_Fahrenheit: {
            String retVal(SFP(HStr_Unit_Degree));
            retVal.concat('F');
            return retVal;
        }
        case Hydro_UnitsType_Temperature_Kelvin: {
            String retVal(SFP(HStr_Unit_Degree));
            retVal.concat('K');
            return retVal;
        }
        case Hydro_UnitsType_Weight_Kilograms:
            return SFP(HStr_Unit_Kilograms);
        case Hydro_UnitsType_Weight_Pounds:
            return SFP(HStr_Unit_Pounds);
        case Hydro_UnitsType_Count:
            return !excludeSpecial ? SFP(HStr_Unit_Count) : String();
        case Hydro_UnitsType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Unit_Undefined) : String();
}

String positionIndexToString(hposi_t positionIndex, bool excludeSpecial)
{
    if (positionIndex >= 0 && positionIndex < HYDRO_POS_MAXSIZE) {
        return String(positionIndex + HYDRO_POS_EXPORT_BEGFROM);
    } else if (!excludeSpecial) {
        if (positionIndex == HYDRO_POS_MAXSIZE) {
            return SFP(HStr_Count);
        } else {
            return SFP(HStr_Undefined);
        }
    }
    return String();
}

hposi_t positionIndexFromString(String positionIndexStr)
{
    if (positionIndexStr == positionIndexToString(HYDRO_POS_MAXSIZE)) {
        return HYDRO_POS_MAXSIZE;
    } else if (positionIndexStr == positionIndexToString(-1)) {
        return -1;
    } else {
        int8_t decode = positionIndexStr.toInt();
        return decode >= 0 && decode < HYDRO_POS_MAXSIZE ? decode : -1;
    }
}


// All remaining methods generated from minimum spanning trie

Hydro_SystemMode systemModeFromString(String systemModeStr)
{
        switch (systemModeStr.length() >= 1 ? systemModeStr[0] : '\0') {
        case 'C':
            return (Hydro_SystemMode)2;
        case 'D':
            return (Hydro_SystemMode)1;
        case 'R':
            return (Hydro_SystemMode)0;
        case 'U':
            return (Hydro_SystemMode)-1;
    }
    return Hydro_SystemMode_Undefined;
}

Hydro_MeasurementMode measurementModeFromString(String measurementModeStr)
{
    switch (measurementModeStr.length() >= 1 ? measurementModeStr[0] : '\0') {
        case 'C':
            return (Hydro_MeasurementMode)3;
        case 'I':
            return (Hydro_MeasurementMode)0;
        case 'M':
            return (Hydro_MeasurementMode)1;
        case 'S':
            return (Hydro_MeasurementMode)2;
        case 'U':
            return (Hydro_MeasurementMode)-1;
    }
    return Hydro_MeasurementMode_Undefined;
}

Hydro_DisplayOutputMode displayOutputModeFromString(String displayOutModeStr)
{
    switch (displayOutModeStr.length() >= 1 ? displayOutModeStr[0] : '\0') {
        case '1':
            switch (displayOutModeStr.length() >= 8 ? displayOutModeStr[7] : '\0') {
                case '\0':
                    return (Hydro_DisplayOutputMode)3;
                case 'S':
                    return (Hydro_DisplayOutputMode)4;
            }
            break;
        case '2':
            switch (displayOutModeStr.length() >= 8 ? displayOutModeStr[7] : '\0') {
                case '\0':
                    return (Hydro_DisplayOutputMode)1;
                case 'S':
                    return (Hydro_DisplayOutputMode)2;
            }
            break;
        case 'C':
            return (Hydro_DisplayOutputMode)5;
        case 'D':
            return (Hydro_DisplayOutputMode)0;
        case 'U':
            return (Hydro_DisplayOutputMode)-1;
    }
    return Hydro_DisplayOutputMode_Undefined;
}

Hydro_ControlInputMode controlInputModeFromString(String controlInModeStr)
{
    switch (controlInModeStr.length() >= 1 ? controlInModeStr[0] : '\0') {
        case '2':
            return (Hydro_ControlInputMode)1;
        case '4':
            return (Hydro_ControlInputMode)2;
        case '6':
            return (Hydro_ControlInputMode)3;
        case 'C':
            return (Hydro_ControlInputMode)5;
        case 'D':
            return (Hydro_ControlInputMode)0;
        case 'R':
            return (Hydro_ControlInputMode)4;
        case 'U':
            return (Hydro_ControlInputMode)-1;
    }
    return Hydro_ControlInputMode_Undefined;
}

Hydro_ActuatorType actuatorTypeFromString(String actuatorTypeStr)
{
    switch (actuatorTypeStr.length() >= 1 ? actuatorTypeStr[0] : '\0') {
        case 'C':
            return (Hydro_ActuatorType)7;
        case 'F':
            return (Hydro_ActuatorType)0;
        case 'G':
            return (Hydro_ActuatorType)1;
        case 'P':
            return (Hydro_ActuatorType)2;
        case 'U':
            return (Hydro_ActuatorType)-1;
        case 'W':
            switch (actuatorTypeStr.length() >= 6 ? actuatorTypeStr[5] : '\0') {
                case 'A':
                    return (Hydro_ActuatorType)3;
                case 'H':
                    return (Hydro_ActuatorType)4;
                case 'P':
                    return (Hydro_ActuatorType)5;
                case 'S':
                    return (Hydro_ActuatorType)6;
            }
            break;
    }
    return Hydro_ActuatorType_Undefined;
}

Hydro_SensorType sensorTypeFromString(String sensorTypeStr)
{
    switch (sensorTypeStr.length() >= 1 ? sensorTypeStr[0] : '\0') {
        case 'A':
            switch (sensorTypeStr.length() >= 4 ? sensorTypeStr[3] : '\0') {
                case 'C':
                    return (Hydro_SensorType)0;
                case 'T':
                    return (Hydro_SensorType)1;
            }
            break;
        case 'C':
            return (Hydro_SensorType)10;
        case 'L':
            return (Hydro_SensorType)8;
        case 'P':
            switch (sensorTypeStr.length() >= 2 ? sensorTypeStr[1] : '\0') {
                case 'o':
                    return (Hydro_SensorType)3;
                case 'u':
                    return (Hydro_SensorType)4;
            }
            break;
        case 'S':
            return (Hydro_SensorType)5;
        case 'U':
            return (Hydro_SensorType)-1;
        case 'W':
            switch (sensorTypeStr.length() >= 6 ? sensorTypeStr[5] : '\0') {
                case 'H':
                    return (Hydro_SensorType)7;
                case 'P':
                    return (Hydro_SensorType)2;
                case 'T':
                    switch (sensorTypeStr.length() >= 7 ? sensorTypeStr[6] : '\0') {
                        case 'D':
                            return (Hydro_SensorType)6;
                        case 'e':
                            return (Hydro_SensorType)9;
                    }
                    break;
            }
            break;
    }
    return Hydro_SensorType_Undefined;
}

Hydro_CropType cropTypeFromString(String cropTypeStr)
{
    switch (cropTypeStr.length() >= 1 ? cropTypeStr[0] : '\0') {
        case 'A':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'l':
                    return (Hydro_CropType)0;
                case 'n':
                    return (Hydro_CropType)1;
                case 'r':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 't':
                            return (Hydro_CropType)2;
                        case 'u':
                            return (Hydro_CropType)3;
                    }
                    break;
                case 's':
                    return (Hydro_CropType)4;
            }
            break;
        case 'B':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    return (Hydro_CropType)5;
                case 'e':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'a':
                            switch (cropTypeStr.length() >= 5 ? cropTypeStr[4] : '\0') {
                                case '\0':
                                    return (Hydro_CropType)6;
                                case 'B':
                                    return (Hydro_CropType)7;
                            }
                            break;
                        case 'e':
                            return (Hydro_CropType)8;
                    }
                    break;
                case 'l':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'a':
                            return (Hydro_CropType)9;
                        case 'u':
                            return (Hydro_CropType)10;
                    }
                    break;
                case 'o':
                    return (Hydro_CropType)11;
                case 'r':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'o':
                            return (Hydro_CropType)12;
                        case 'u':
                            return (Hydro_CropType)13;
                    }
                    break;
            }
            break;
        case 'C':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'b':
                            return (Hydro_CropType)14;
                        case 'n':
                            return (Hydro_CropType)15;
                        case 'p':
                            return (Hydro_CropType)16;
                        case 'r':
                            return (Hydro_CropType)17;
                        case 't':
                            return (Hydro_CropType)18;
                        case 'u':
                            return (Hydro_CropType)19;
                    }
                    break;
                case 'e':
                    return (Hydro_CropType)20;
                case 'h':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'a':
                            return (Hydro_CropType)21;
                        case 'i':
                            switch (cropTypeStr.length() >= 4 ? cropTypeStr[3] : '\0') {
                                case 'c':
                                    return (Hydro_CropType)22;
                                case 'v':
                                    return (Hydro_CropType)23;
                            }
                            break;
                    }
                    break;
                case 'i':
                    return (Hydro_CropType)24;
                case 'o':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'r':
                            switch (cropTypeStr.length() >= 4 ? cropTypeStr[3] : '\0') {
                                case 'i':
                                    return (Hydro_CropType)25;
                                case 'n':
                                    return (Hydro_CropType)26;
                            }
                            break;
                        case 'u':
                            return (Hydro_CropType)85;
                    }
                    break;
                case 'u':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'c':
                            return (Hydro_CropType)27;
                        case 's':
                            switch (cropTypeStr.length() >= 11 ? cropTypeStr[10] : '\0') {
                                case '1':
                                    return (Hydro_CropType)77;
                                case '2':
                                    return (Hydro_CropType)78;
                                case '3':
                                    return (Hydro_CropType)79;
                                case '4':
                                    return (Hydro_CropType)80;
                                case '5':
                                    return (Hydro_CropType)81;
                                case '6':
                                    return (Hydro_CropType)82;
                                case '7':
                                    return (Hydro_CropType)83;
                                case '8':
                                    return (Hydro_CropType)84;
                            }
                            break;
                    }
                    break;
            }
            break;
        case 'D':
            return (Hydro_CropType)28;
        case 'E':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'g':
                    return (Hydro_CropType)29;
                case 'n':
                    return (Hydro_CropType)30;
            }
            break;
        case 'F':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'e':
                    return (Hydro_CropType)31;
                case 'l':
                    return (Hydro_CropType)33;
                case 'o':
                    return (Hydro_CropType)32;
            }
            break;
        case 'G':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    return (Hydro_CropType)34;
                case 'i':
                    return (Hydro_CropType)35;
            }
            break;
        case 'K':
            return (Hydro_CropType)36;
        case 'L':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    return (Hydro_CropType)37;
                case 'e':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'e':
                            return (Hydro_CropType)38;
                        case 'm':
                            return (Hydro_CropType)39;
                        case 't':
                            return (Hydro_CropType)40;
                    }
                    break;
            }
            break;
        case 'M':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    return (Hydro_CropType)41;
                case 'e':
                    return (Hydro_CropType)42;
                case 'i':
                    return (Hydro_CropType)43;
                case 'u':
                    return (Hydro_CropType)44;
            }
            break;
        case 'O':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'k':
                    return (Hydro_CropType)45;
                case 'n':
                    return (Hydro_CropType)46;
                case 'r':
                    return (Hydro_CropType)47;
            }
            break;
        case 'P':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'k':
                            return (Hydro_CropType)48;
                        case 'r':
                            switch (cropTypeStr.length() >= 5 ? cropTypeStr[4] : '\0') {
                                case 'l':
                                    return (Hydro_CropType)49;
                                case 'n':
                                    return (Hydro_CropType)50;
                            }
                            break;
                    }
                    break;
                case 'e':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'a':
                            switch (cropTypeStr.length() >= 4 ? cropTypeStr[3] : '\0') {
                                case '\0':
                                    return (Hydro_CropType)51;
                                case 'S':
                                    return (Hydro_CropType)52;
                            }
                            break;
                        case 'p':
                            switch (cropTypeStr.length() >= 4 ? cropTypeStr[3] : '\0') {
                                case 'i':
                                    return (Hydro_CropType)53;
                                case 'p':
                                    switch (cropTypeStr.length() >= 8 ? cropTypeStr[7] : '\0') {
                                        case 'B':
                                            return (Hydro_CropType)54;
                                        case 'H':
                                            return (Hydro_CropType)55;
                                    }
                                    break;
                            }
                            break;
                    }
                    break;
                case 'o':
                    switch (cropTypeStr.length() >= 7 ? cropTypeStr[6] : '\0') {
                        case '\0':
                            return (Hydro_CropType)56;
                        case 'S':
                            return (Hydro_CropType)57;
                    }
                    break;
                case 'u':
                    return (Hydro_CropType)58;
            }
            break;
        case 'R':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    return (Hydro_CropType)59;
                case 'h':
                    return (Hydro_CropType)60;
                case 'o':
                    return (Hydro_CropType)61;
            }
            break;
        case 'S':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    return (Hydro_CropType)62;
                case 'i':
                    return (Hydro_CropType)63;
                case 'p':
                    return (Hydro_CropType)64;
                case 'q':
                    return (Hydro_CropType)65;
                case 't':
                    return (Hydro_CropType)67;
                case 'u':
                    return (Hydro_CropType)66;
                case 'w':
                    return (Hydro_CropType)68;
            }
            break;
        case 'T':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    switch (cropTypeStr.length() >= 4 ? cropTypeStr[3] : '\0') {
                        case 'o':
                            return (Hydro_CropType)69;
                        case 'r':
                            return (Hydro_CropType)70;
                    }
                    break;
                case 'h':
                    return (Hydro_CropType)71;
                case 'o':
                    return (Hydro_CropType)72;
                case 'u':
                    return (Hydro_CropType)73;
            }
            break;
        case 'U':
            return (Hydro_CropType)-1;
        case 'W':
            switch (cropTypeStr.length() >= 6 ? cropTypeStr[5] : '\0') {
                case 'c':
                    return (Hydro_CropType)74;
                case 'm':
                    return (Hydro_CropType)75;
            }
            break;
        case 'Z':
            return (Hydro_CropType)76;
    }
    return Hydro_CropType_Undefined;
}

Hydro_SubstrateType substrateTypeFromString(String substrateTypeStr)
{
        switch (substrateTypeStr.length() >= 1 ? substrateTypeStr[0] : '\0') {
        case 'C':
            switch (substrateTypeStr.length() >= 2 ? substrateTypeStr[1] : '\0') {
                case 'l':
                    return (Hydro_SubstrateType)0;
                case 'o':
                    switch (substrateTypeStr.length() >= 3 ? substrateTypeStr[2] : '\0') {
                        case 'c':
                            return (Hydro_SubstrateType)1;
                        case 'u':
                            return (Hydro_SubstrateType)3;
                    }
                    break;
            }
            break;
        case 'R':
            return (Hydro_SubstrateType)2;
        case 'U':
            return (Hydro_SubstrateType)-1;
    }
    return Hydro_SubstrateType_Undefined;
}

Hydro_ReservoirType reservoirTypeFromString(String reservoirTypeStr)
{
     switch (reservoirTypeStr.length() >= 1 ? reservoirTypeStr[0] : '\0') {
        case 'C':
            switch (reservoirTypeStr.length() >= 2 ? reservoirTypeStr[1] : '\0') {
                case 'o':
                    return (Hydro_ReservoirType)22;
                case 'u':
                    switch (reservoirTypeStr.length() >= 15 ? reservoirTypeStr[14] : '\0') {
                        case '1':
                            switch (reservoirTypeStr.length() >= 16 ? reservoirTypeStr[15] : '\0') {
                                case '\0':
                                    return (Hydro_ReservoirType)6;
                                case '0':
                                    return (Hydro_ReservoirType)15;
                                case '1':
                                    return (Hydro_ReservoirType)16;
                                case '2':
                                    return (Hydro_ReservoirType)17;
                                case '3':
                                    return (Hydro_ReservoirType)18;
                                case '4':
                                    return (Hydro_ReservoirType)19;
                                case '5':
                                    return (Hydro_ReservoirType)20;
                                case '6':
                                    return (Hydro_ReservoirType)21;
                            }
                            break;
                        case '2':
                            return (Hydro_ReservoirType)7;
                        case '3':
                            return (Hydro_ReservoirType)8;
                        case '4':
                            return (Hydro_ReservoirType)9;
                        case '5':
                            return (Hydro_ReservoirType)10;
                        case '6':
                            return (Hydro_ReservoirType)11;
                        case '7':
                            return (Hydro_ReservoirType)12;
                        case '8':
                            return (Hydro_ReservoirType)13;
                        case '9':
                            return (Hydro_ReservoirType)14;
                    }
                    break;
            }
            break;
        case 'D':
            return (Hydro_ReservoirType)1;
        case 'F':
            switch (reservoirTypeStr.length() >= 2 ? reservoirTypeStr[1] : '\0') {
                case 'e':
                    return (Hydro_ReservoirType)0;
                case 'r':
                    return (Hydro_ReservoirType)3;
            }
            break;
        case 'N':
            return (Hydro_ReservoirType)2;
        case 'P':
            switch (reservoirTypeStr.length() >= 3 ? reservoirTypeStr[2] : '\0') {
                case 'D':
                    return (Hydro_ReservoirType)5;
                case 'U':
                    return (Hydro_ReservoirType)4;
            }
            break;
        case 'U':
            return (Hydro_ReservoirType)-1;
    }
    return Hydro_ReservoirType_Undefined;
}

Hydro_RailType railTypeFromString(String railTypeStr) {
    switch (railTypeStr.length() >= 1 ? railTypeStr[0] : '\0') {
        case 'A':
            switch (railTypeStr.length() >= 3 ? railTypeStr[2] : '\0') {
                case '1':
                    return (Hydro_RailType)0;
                case '2':
                    return (Hydro_RailType)1;
            }
            break;
        case 'C':
            return (Hydro_RailType)7;
        case 'D':
            switch (railTypeStr.length() >= 3 ? railTypeStr[2] : '\0') {
                case '1':
                    return (Hydro_RailType)4;
                case '2':
                    return (Hydro_RailType)5;
                case '3':
                    return (Hydro_RailType)2;
                case '4':
                    return (Hydro_RailType)6;
                case '5':
                    return (Hydro_RailType)3;
            }
            break;
        case 'U':
            return (Hydro_RailType)-1;
    }
    return Hydro_RailType_Undefined;
}

Hydro_PinMode pinModeFromString(String pinModeStr)
{
    switch (pinModeStr.length() >= 1 ? pinModeStr[0] : '\0') {
        case 'A':
            switch (pinModeStr.length() >= 7 ? pinModeStr[6] : '\0') {
                case 'I':
                    return (Hydro_PinMode)5;
                case 'O':
                    return (Hydro_PinMode)6;
            }
            break;
        case 'C':
            return (Hydro_PinMode)7;
        case 'D':
            switch (pinModeStr.length() >= 8 ? pinModeStr[7] : '\0') {
                case 'I':
                    switch (pinModeStr.length() >= 13 ? pinModeStr[12] : '\0') {
                        case '\0':
                            return (Hydro_PinMode)0;
                        case 'P':
                            switch (pinModeStr.length() >= 17 ? pinModeStr[16] : '\0') {
                                case 'D':
                                    return (Hydro_PinMode)2;
                                case 'U':
                                    return (Hydro_PinMode)1;
                            }
                            break;
                    }
                    break;
                case 'O':
                    switch (pinModeStr.length() >= 14 ? pinModeStr[13] : '\0') {
                        case '\0':
                            return (Hydro_PinMode)3;
                        case 'P':
                            return (Hydro_PinMode)4;
                    }
                    break;
            }
            break;
        case 'U':
            return (Hydro_PinMode)-1;
    }
    return Hydro_PinMode_Undefined;
}

Hydro_EnableMode enableModeFromString(String enableModeStr)
{
    switch (enableModeStr.length() >= 1 ? enableModeStr[0] : '\0') {
        case 'A':
            switch (enableModeStr.length() >= 2 ? enableModeStr[1] : '\0') {
                case 's':
                    return (Hydro_EnableMode)7;
                case 'v':
                    return (Hydro_EnableMode)2;
            }
            break;
        case 'C':
            return (Hydro_EnableMode)8;
        case 'D':
            return (Hydro_EnableMode)6;
        case 'H':
            return (Hydro_EnableMode)0;
        case 'I':
            return (Hydro_EnableMode)4;
        case 'L':
            return (Hydro_EnableMode)1;
        case 'M':
            return (Hydro_EnableMode)3;
        case 'R':
            return (Hydro_EnableMode)5;
        case 'U':
            return (Hydro_EnableMode)-1;
    }
    return Hydro_EnableMode_Undefined;
}

Hydro_UnitsCategory unitsCategoryFromString(String unitsCategoryStr)
{
     switch (unitsCategoryStr.length() >= 1 ? unitsCategoryStr[0] : '\0') {
        case 'A':
            return (Hydro_UnitsCategory)0;
        case 'C':
            switch (unitsCategoryStr.length() >= 3 ? unitsCategoryStr[2] : '\0') {
                case 'n':
                    return (Hydro_UnitsCategory)1;
                case 'u':
                    return (Hydro_UnitsCategory)10;
            }
            break;
        case 'D':
            return (Hydro_UnitsCategory)2;
        case 'L':
            switch (unitsCategoryStr.length() >= 4 ? unitsCategoryStr[3] : '\0') {
                case 'D':
                    return (Hydro_UnitsCategory)3;
                case 'F':
                    return (Hydro_UnitsCategory)4;
                case 'V':
                    return (Hydro_UnitsCategory)5;
            }
            break;
        case 'P':
            switch (unitsCategoryStr.length() >= 2 ? unitsCategoryStr[1] : '\0') {
                case 'e':
                    return (Hydro_UnitsCategory)7;
                case 'o':
                    return (Hydro_UnitsCategory)8;
            }
            break;
        case 'T':
            return (Hydro_UnitsCategory)6;
        case 'U':
            return (Hydro_UnitsCategory)-1;
        case 'W':
            return (Hydro_UnitsCategory)9;
    }
    return Hydro_UnitsCategory_Undefined;
}

Hydro_UnitsType unitsTypeFromSymbol(String unitsSymbolStr)
{
    switch (unitsSymbolStr.length() >= 1 ? unitsSymbolStr[0] : '\0') {
        case '%':
            return (Hydro_UnitsType)1;
        case 'A':
            return (Hydro_UnitsType)15;
        case 'E':
            return (Hydro_UnitsType)3;
        case 'J':
            return (Hydro_UnitsType)16;
        case 'K':
            return (Hydro_UnitsType)20;
        case 'L':
            switch (unitsSymbolStr.length() >= 2 ? unitsSymbolStr[1] : '\0') {
                case '\0':
                    return (Hydro_UnitsType)14;
                case '/':
                    return (Hydro_UnitsType)12;
            }
            break;
        case 'T':
            return (Hydro_UnitsType)3;
        case 'W':
            return (Hydro_UnitsType)16;
        case '[':
            switch (unitsSymbolStr.length() >= 2 ? unitsSymbolStr[1] : '\0') {
                case 'p':
                    return (Hydro_UnitsType)2;
                case 'q':
                    return (Hydro_UnitsType)22;
                case 'u':
                    return (Hydro_UnitsType)-1;
            }
            break;
        case 'f':
            return (Hydro_UnitsType)7;
        case 'g':
            switch (unitsSymbolStr.length() >= 4 ? unitsSymbolStr[3] : '\0') {
                case '\0':
                    return (Hydro_UnitsType)13;
                case '/':
                    return (Hydro_UnitsType)11;
            }
            break;
        case 'l':
            return (Hydro_UnitsType)21;
        case 'm':
            switch (unitsSymbolStr.length() >= 2 ? unitsSymbolStr[1] : '\0') {
                case '\0':
                    return (Hydro_UnitsType)8;
                case 'L':
                    switch (unitsSymbolStr.length() >= 4 ? unitsSymbolStr[3] : '\0') {
                        case 'L':
                            return (Hydro_UnitsType)10;
                        case 'g':
                            return (Hydro_UnitsType)9;
                    }
                    break;
                case 'S':
                    return (Hydro_UnitsType)3;
            }
            break;
        case 'p':
            switch (unitsSymbolStr.length() >= 4 ? unitsSymbolStr[3] : '\0') {
                case '\0':
                    return (Hydro_UnitsType)4;
                case '(':
                    switch (unitsSymbolStr.length() >= 5 ? unitsSymbolStr[4] : '\0') {
                        case '5':
                            return (Hydro_UnitsType)4;
                        case '6':
                            return (Hydro_UnitsType)5;
                        case '7':
                            return (Hydro_UnitsType)6;
                    }
                    break;
            }
            break;
        case 'r':
            return (Hydro_UnitsType)0;
        default: // degree symbol
            switch (unitsSymbolStr.length() >= 3 ? unitsSymbolStr[2] : '\0') {
                case 'C':
                    return (Hydro_UnitsType)17;
                case 'F':
                    return (Hydro_UnitsType)18;
                case 'K':
                    return (Hydro_UnitsType)19;
            }
            break;
    }
    return Hydro_UnitsType_Undefined;
}
