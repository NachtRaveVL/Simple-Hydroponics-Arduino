/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Utilities
*/

#include "Hydroponics.h"
#include <pins_arduino.h>

HydroponicsBitResolution::HydroponicsBitResolution(uint8_t bitResIn)
    : bitRes(bitResIn), maxVal(1 << bitResIn)
{ ; }


#ifndef HYDRUINO_DISABLE_MULTITASKING

BasicArduinoInterruptAbstraction interruptImpl;

ActuatorTimedEnableTask::ActuatorTimedEnableTask(SharedPtr<HydroponicsActuator> actuator, float enableIntensity, time_t enableTimeMillis)
    : taskId(TASKMGR_INVALIDID), _actuator(actuator), _enableIntensity(enableIntensity), _enableTimeMillis(enableTimeMillis)
{ ; }

void ActuatorTimedEnableTask::exec()
{
    while (!_actuator->enableActuator(_enableIntensity)) { yield(); }

    delayFine(_enableTimeMillis);

    HYDRUINO_SOFT_ASSERT(_actuator->isEnabled(), SFP(HStr_Err_OperationFailure));
    _actuator->disableActuator();
}

taskid_t scheduleActuatorTimedEnableOnce(SharedPtr<HydroponicsActuator> actuator, float enableIntensity, time_t enableTimeMillis)
{
    ActuatorTimedEnableTask *enableTask = actuator ? new ActuatorTimedEnableTask(actuator, enableIntensity, enableTimeMillis) : nullptr;
    HYDRUINO_SOFT_ASSERT(!actuator || enableTask, SFP(HStr_Err_AllocationFailure));
    taskid_t retVal = enableTask ? taskManager.scheduleOnce(0, enableTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (enableTask ? (enableTask->taskId = retVal) : retVal);
}

taskid_t scheduleActuatorTimedEnableOnce(SharedPtr<HydroponicsActuator> actuator, time_t enableTimeMillis)
{
    ActuatorTimedEnableTask *enableTask = actuator ? new ActuatorTimedEnableTask(actuator, 1.0f, enableTimeMillis) : nullptr;
    HYDRUINO_SOFT_ASSERT(!actuator || enableTask, SFP(HStr_Err_AllocationFailure));
    taskid_t retVal = enableTask ? taskManager.scheduleOnce(0, enableTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (enableTask ? (enableTask->taskId = retVal) : retVal);
}

#endif // /ifndef HYDRUINO_DISABLE_MULTITASKING


#ifdef HYDRUINO_USE_DEBUG_ASSERTIONS

static String fileFromFullPath(String fullPath)
{
    int index = fullPath.lastIndexOf(HYDRUINO_BLDPATH_SEPARATOR);
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
        getLoggerInstance()->logWarning(SFP(HStr_Err_AssertionFailure), SFP(HStr_ColonSpace), assertMsg);
        getLoggerInstance()->logWarning(SFP(HStr_DoubleSpace), msg);
        getLoggerInstance()->flush();
    }
}

void hardAssert(bool cond, String msg, const char *file, const char *func, int line)
{
    if (!cond) {
        String assertMsg = makeAssertMsg(file, func, line);
        getLoggerInstance()->logError(SFP(HStr_Err_AssertionFailure), String(F(" HARD")) + SFP(HStr_ColonSpace), assertMsg);
        getLoggerInstance()->logError(SFP(HStr_DoubleSpace), msg);
        getLoggerInstance()->flush();

        if (getHydroponicsInstance()) { getHydroponicsInstance()->suspend(); }
        yield(); delay(10);
        abort();
    }
}

#endif // /ifdef HYDRUINO_USE_DEBUG_ASSERTIONS


void publishData(HydroponicsSensor *sensor)
{
    HYDRUINO_HARD_ASSERT(sensor, SFP(HStr_Err_InvalidParameter));

    if (getPublisherInstance()) {
        auto measurement = sensor->getLatestMeasurement();
        Hydroponics_PositionIndex rows = getMeasurementRowCount(measurement);
        Hydroponics_PositionIndex columnIndexStart = getPublisherInstance()->getColumnIndexStart(sensor->getKey());

        if (columnIndexStart >= 0) {
            for (uint8_t measurementRow = 0; measurementRow < rows; ++measurementRow) {
                getPublisherInstance()->publishData(columnIndexStart + measurementRow, getAsSingleMeasurement(measurement, measurementRow));
            }
        }
    }
}

bool setCurrentTime(DateTime currTime)
{
    auto rtc = getHydroponicsInstance() ? getHydroponicsInstance()->getRealTimeClock() : nullptr;
    if (rtc) {
        rtc->adjust(currTime);
        getHydroponicsInstance()->notifyRTCTimeUpdated();
        return true;
    }
    return false;
}

String getYYMMDDFilename(String prefix, String ext)
{
    DateTime currTime = getCurrentTime();
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
    auto slashIndex = filename.indexOf(HYDRUINO_FSPATH_SEPARATOR);
    String directory = slashIndex != -1 ? filename.substring(0, slashIndex) : String();
    String dirWithSep = directory + String(HYDRUINO_FSPATH_SEPARATOR);
    if (directory.length() && !sd->exists(dirWithSep.c_str())) {
        sd->mkdir(directory.c_str());
    }
}

Hydroponics_KeyType stringHash(String string)
{
    Hydroponics_KeyType hash = 5381;
    for(int index = 0; index < string.length(); ++index) {
        hash = ((hash << 5) + hash) + (Hydroponics_KeyType)string[index]; // Good 'ol DJB2
    }
    return hash != (Hydroponics_KeyType)-1 ? hash : 5381;
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

extern String measurementToString(float value, Hydroponics_UnitsType units, unsigned int additionalDecPlaces)
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
#elif !defined(ESP32)
extern char *__brkval;
#endif  // __arm__

unsigned int freeMemory() {
    #ifdef ESP32
        return esp_get_free_heap_size();
    #else
        char top;
        #ifdef __arm__
            return &top - reinterpret_cast<char*>(sbrk(0));
        #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
            return &top - __brkval;
        #else  // __arm__
            return __brkval ? &top - __brkval : &top - __malloc_heap_start;
        #endif  // #ifdef __arm__
        return 0;
    #endif
}

void delayFine(time_t timeMillis) {
    time_t startMillis = millis();
    time_t endMillis = startMillis + timeMillis;

    {   time_t delayMillis = max(0, timeMillis - HYDRUINO_SYS_DELAYFINE_SPINMILLIS);
        if (delayMillis > 0) { delay(delayMillis); }
    }

    {   time_t timeMillis = millis();
        while ((endMillis >= startMillis && (timeMillis < endMillis)) ||
                (endMillis < startMillis && (timeMillis >= startMillis || timeMillis < endMillis))) {
            timeMillis = millis();
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

bool tryConvertUnits(float valueIn, Hydroponics_UnitsType unitsIn, float *valueOut, Hydroponics_UnitsType unitsOut, float convertParam)
{
    if (!valueOut || unitsOut == Hydroponics_UnitsType_Undefined || unitsIn == unitsOut) return false;

    switch (unitsIn) {
        case Hydroponics_UnitsType_Raw_0_1:
            switch (unitsOut) {
                case Hydroponics_UnitsType_Alkalinity_pH_0_14:
                    *valueOut = valueIn * 14.0f;
                    return true;

                case Hydroponics_UnitsType_Concentration_EC:
                    if (isFPEqual(convertParam, FLT_UNDEF)) { // convertParam = aRef voltage (5 or 3.3) of meter -> typically 1v = 1EC, depending on calib
                        *valueOut = valueIn * 5.0f;
                    } else {
                        *valueOut = valueIn * convertParam;
                    }
                    return true;

                case Hydroponics_UnitsType_Concentration_PPM500:
                    if (isFPEqual(convertParam, FLT_UNDEF)) { // convertParam = aRef voltage
                        *valueOut = valueIn * (5.0f * 500.0f);
                    } else {
                        *valueOut = valueIn * (convertParam * 500.0f);
                    }
                    return true;

                case Hydroponics_UnitsType_Concentration_PPM640:
                    if (isFPEqual(convertParam, FLT_UNDEF)) { // convertParam = aRef voltage
                        *valueOut = valueIn * (5.0f * 640.0f);
                    } else {
                        *valueOut = valueIn * (convertParam * 640.0f);
                    }
                    return true;

                case Hydroponics_UnitsType_Concentration_PPM700:
                    if (isFPEqual(convertParam, FLT_UNDEF)) { // convertParam = aRef voltage
                        *valueOut = valueIn * (5.0f * 700.0f);
                    } else {
                        *valueOut = valueIn * (convertParam * 700.0f);
                    }
                    return true;

                case Hydroponics_UnitsType_Percentile_0_100:
                    *valueOut = valueIn * 100.0f;
                    return true;

                default:
                    if (!isFPEqual(convertParam, FLT_UNDEF)) {
                        *valueOut = valueIn * convertParam;
                        return true;
                    }
                    break;
            }
            break;

        case Hydroponics_UnitsType_Percentile_0_100:
            switch (unitsOut) {
                case Hydroponics_UnitsType_Raw_0_1:
                    *valueOut = valueIn / 100.0f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Alkalinity_pH_0_14:
            switch (unitsOut) {
                case Hydroponics_UnitsType_Raw_0_1:
                    *valueOut = valueIn / 14.0f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Concentration_EC:
            switch (unitsOut) {
                case Hydroponics_UnitsType_Raw_0_1:
                    if (isFPEqual(convertParam, FLT_UNDEF)) { // convertParam = aRef voltage
                        *valueOut = valueIn / 5.0f;
                    } else {
                        *valueOut = valueIn / convertParam;
                    }
                    return true;

                case Hydroponics_UnitsType_Concentration_PPM500:
                    *valueOut = valueIn * 500.0f;
                    return true;

                case Hydroponics_UnitsType_Concentration_PPM640:
                    *valueOut = valueIn * 640.0f;
                    return true;

                case Hydroponics_UnitsType_Concentration_PPM700:
                    *valueOut = valueIn * 700.0f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Temperature_Celsius:
            switch (unitsOut) {
                case Hydroponics_UnitsType_Temperature_Fahrenheit:
                    *valueOut = valueIn * 1.8 + 32.0;
                    return true;

                case Hydroponics_UnitsType_Temperature_Kelvin:
                    *valueOut = valueIn + 273.15;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Temperature_Fahrenheit:
            switch (unitsOut) {
                case Hydroponics_UnitsType_Temperature_Celsius:
                    *valueOut = (valueIn - 32.0) / 1.8;
                    return true;

                case Hydroponics_UnitsType_Temperature_Kelvin:
                    *valueOut = ((valueIn + 459.67f) * 5.0f) / 9.0f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Temperature_Kelvin:
            switch (unitsOut) {
                case Hydroponics_UnitsType_Temperature_Celsius:
                    *valueOut = valueIn - 273.15;
                    return true;

                case Hydroponics_UnitsType_Temperature_Fahrenheit:
                    *valueOut = ((valueIn * 9.0) / 5.0) - 459.67;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_LiqVolume_Liters:
            switch (unitsOut) {
                case Hydroponics_UnitsType_LiqVolume_Gallons:
                    *valueOut = valueIn * 0.264172f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_LiqVolume_Gallons:
            switch (unitsOut) {
                case Hydroponics_UnitsType_LiqVolume_Liters:
                    *valueOut = valueIn * 3.78541f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_LiqFlowRate_LitersPerMin:
            switch (unitsOut) {
                case Hydroponics_UnitsType_LiqFlowRate_GallonsPerMin:
                    *valueOut = valueIn * 0.264172f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_LiqFlowRate_GallonsPerMin:
            switch (unitsOut) {
                case Hydroponics_UnitsType_LiqFlowRate_LitersPerMin:
                    *valueOut = valueIn * 3.78541f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_LiqDilution_MilliLiterPerLiter:
            switch (unitsOut) {
                case Hydroponics_UnitsType_LiqDilution_MilliLiterPerGallon:
                    *valueOut = valueIn * 3.78541f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_LiqDilution_MilliLiterPerGallon:
            switch (unitsOut) {
                case Hydroponics_UnitsType_LiqDilution_MilliLiterPerLiter:
                    *valueOut = valueIn * 0.264172f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Concentration_PPM500:
            switch (unitsOut) {
                case Hydroponics_UnitsType_Raw_0_1:
                    if (isFPEqual(convertParam, FLT_UNDEF)) { // convertParam = aRef voltage
                        *valueOut = valueIn / (5.0f * 500.0f);
                    } else {
                        *valueOut = valueIn / (convertParam * 500.0f);
                    }
                    return true;

                case Hydroponics_UnitsType_Concentration_EC:
                    *valueOut = valueIn / 500.0f;
                    return true;

                case Hydroponics_UnitsType_Concentration_PPM640:
                case Hydroponics_UnitsType_Concentration_PPM700:
                    if (tryConvertUnits(valueIn, unitsIn, valueOut, Hydroponics_UnitsType_Concentration_EC)) {
                        return tryConvertUnits(*valueOut, Hydroponics_UnitsType_Concentration_EC, valueOut, unitsOut);
                    }
                    return false;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Concentration_PPM640:
            switch (unitsOut) {
                case Hydroponics_UnitsType_Raw_0_1:
                    if (isFPEqual(convertParam, FLT_UNDEF)) { // convertParam = aRef voltage
                        *valueOut = valueIn / (5.0f * 640.0f);
                    } else {
                        *valueOut = valueIn / (convertParam * 640.0f);
                    }
                    return true;

                case Hydroponics_UnitsType_Concentration_EC:
                    *valueOut = valueIn / 640.0f;
                    return true;

                case Hydroponics_UnitsType_Concentration_PPM500:
                case Hydroponics_UnitsType_Concentration_PPM700:
                    if (tryConvertUnits(valueIn, unitsIn, valueOut, Hydroponics_UnitsType_Concentration_EC)) {
                        return tryConvertUnits(*valueOut, Hydroponics_UnitsType_Concentration_EC, valueOut, unitsOut);
                    }
                    return false;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Concentration_PPM700:
            switch (unitsOut) {
                case Hydroponics_UnitsType_Raw_0_1:
                    if (isFPEqual(convertParam, FLT_UNDEF)) { // convertParam = aRef voltage
                        *valueOut = valueIn / (5.0f * 700.0f);
                    } else {
                        *valueOut = valueIn / (convertParam * 700.0f);
                    }
                    return true;

                case Hydroponics_UnitsType_Concentration_EC:
                    *valueOut = valueIn / 700.0f;
                    return true;

                case Hydroponics_UnitsType_Concentration_PPM500:
                case Hydroponics_UnitsType_Concentration_PPM700:
                    if (tryConvertUnits(valueIn, unitsIn, valueOut, Hydroponics_UnitsType_Concentration_EC)) {
                        return tryConvertUnits(*valueOut, Hydroponics_UnitsType_Concentration_EC, valueOut, unitsOut);
                    }
                    return false;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Distance_Meters:
            switch (unitsOut) {
                case Hydroponics_UnitsType_Distance_Feet:
                    *valueOut = valueIn * 3.28084f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Distance_Feet:
            switch (unitsOut) {
                case Hydroponics_UnitsType_Distance_Meters:
                    *valueOut = valueIn * 0.3048;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Weight_Kilogram:
            switch (unitsOut) {
                case Hydroponics_UnitsType_Weight_Pounds:
                    *valueOut = valueIn * 2.20462f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Weight_Pounds:
            switch (unitsOut) {
                case Hydroponics_UnitsType_Weight_Kilogram:
                    *valueOut = valueIn * 0.453592f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Power_Wattage:
            switch (unitsOut) {
                case Hydroponics_UnitsType_Power_Amperage:
                    if (!isFPEqual(convertParam, FLT_UNDEF)) { // convertParam = rail voltage
                        *valueOut = valueIn / convertParam;
                        return true;
                    }
                break;
            }
            break;

        case Hydroponics_UnitsType_Power_Amperage:
            switch (unitsOut) {
                case Hydroponics_UnitsType_Power_Wattage:
                    if (!isFPEqual(convertParam, FLT_UNDEF)) { // convertParam = rail voltage
                        *valueOut = valueIn * convertParam;
                        return true;
                    }
                break;
            }
            break;

        case Hydroponics_UnitsType_Undefined:
            *valueOut = valueIn;
            return true;

        default:
            break;
    }

    return false;
}

bool convertUnits(float *valueInOut, Hydroponics_UnitsType *unitsInOut, Hydroponics_UnitsType outUnits, float convertParam)
{
    if (tryConvertUnits(*valueInOut, *unitsInOut, valueInOut, outUnits, convertParam)) {
        *unitsInOut = outUnits;
        return true;
    }
    return false;
}

bool convertUnits(float valueIn, float *valueOut, Hydroponics_UnitsType unitsIn, Hydroponics_UnitsType outUnits, Hydroponics_UnitsType *unitsOut, float convertParam)
{
    if (tryConvertUnits(valueIn, unitsIn, valueOut, outUnits, convertParam)) {
        if (unitsOut) { *unitsOut = outUnits; }
        return true;
    }
    return false;
}

Hydroponics_UnitsType baseUnitsFromRate(Hydroponics_UnitsType units)
{
    switch (units) {
        case Hydroponics_UnitsType_LiqFlowRate_LitersPerMin:
            return Hydroponics_UnitsType_LiqVolume_Liters;
        case Hydroponics_UnitsType_LiqFlowRate_GallonsPerMin:
            return Hydroponics_UnitsType_LiqVolume_Gallons;
        default:
            break;
    }
    return Hydroponics_UnitsType_Undefined;
}

Hydroponics_UnitsType baseUnitsFromDilution(Hydroponics_UnitsType units)
{
    switch (units) {
        case Hydroponics_UnitsType_LiqDilution_MilliLiterPerLiter:
            return Hydroponics_UnitsType_LiqVolume_Liters;
        case Hydroponics_UnitsType_LiqDilution_MilliLiterPerGallon:
            return Hydroponics_UnitsType_LiqVolume_Gallons;
        default:
            break;
    }
    return Hydroponics_UnitsType_Undefined;
}

Hydroponics_UnitsType defaultTemperatureUnits(Hydroponics_MeasurementMode measureMode)
{
    if (measureMode == Hydroponics_MeasurementMode_Undefined) {
        measureMode = (getHydroponicsInstance() ? getHydroponicsInstance()->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
    }

    switch (measureMode) {
        case Hydroponics_MeasurementMode_Imperial:
            return Hydroponics_UnitsType_Temperature_Fahrenheit;
        case Hydroponics_MeasurementMode_Metric:
            return Hydroponics_UnitsType_Temperature_Celsius;
        case Hydroponics_MeasurementMode_Scientific:
            return Hydroponics_UnitsType_Temperature_Kelvin;
        default:
            return Hydroponics_UnitsType_Undefined;
    }
}

Hydroponics_UnitsType defaultDistanceUnits(Hydroponics_MeasurementMode measureMode)
{
    if (measureMode == Hydroponics_MeasurementMode_Undefined) {
        measureMode = (getHydroponicsInstance() ? getHydroponicsInstance()->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
    }

    switch (measureMode) {
        case Hydroponics_MeasurementMode_Imperial:
            return Hydroponics_UnitsType_Distance_Meters;
        case Hydroponics_MeasurementMode_Metric:
        case Hydroponics_MeasurementMode_Scientific:
            return Hydroponics_UnitsType_Distance_Meters;
        default:
            return Hydroponics_UnitsType_Undefined;
    }
}

Hydroponics_UnitsType defaultWeightUnits(Hydroponics_MeasurementMode measureMode)
{
    if (measureMode == Hydroponics_MeasurementMode_Undefined) {
        measureMode = (getHydroponicsInstance() ? getHydroponicsInstance()->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
    }

    switch (measureMode) {
        case Hydroponics_MeasurementMode_Imperial:
            return Hydroponics_UnitsType_Weight_Pounds;
        case Hydroponics_MeasurementMode_Metric:
        case Hydroponics_MeasurementMode_Scientific:
            return Hydroponics_UnitsType_Weight_Kilogram;
        default:
            return Hydroponics_UnitsType_Undefined;
    }
}

Hydroponics_UnitsType defaultLiquidVolumeUnits(Hydroponics_MeasurementMode measureMode)
{
    if (measureMode == Hydroponics_MeasurementMode_Undefined) {
        measureMode = (getHydroponicsInstance() ? getHydroponicsInstance()->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
    }

    switch (measureMode) {
        case Hydroponics_MeasurementMode_Imperial:
            return Hydroponics_UnitsType_LiqVolume_Gallons;
        case Hydroponics_MeasurementMode_Metric:
        case Hydroponics_MeasurementMode_Scientific:
            return Hydroponics_UnitsType_LiqVolume_Liters;
        default:
            return Hydroponics_UnitsType_Undefined;
    }
}

Hydroponics_UnitsType defaultLiquidFlowUnits(Hydroponics_MeasurementMode measureMode)
{
    if (measureMode == Hydroponics_MeasurementMode_Undefined) {
        measureMode = (getHydroponicsInstance() ? getHydroponicsInstance()->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
    }

    switch (measureMode) {
        case Hydroponics_MeasurementMode_Imperial:
            return Hydroponics_UnitsType_LiqFlowRate_GallonsPerMin;
        case Hydroponics_MeasurementMode_Metric:
        case Hydroponics_MeasurementMode_Scientific:
            return Hydroponics_UnitsType_LiqFlowRate_LitersPerMin;
        default:
            return Hydroponics_UnitsType_Undefined;
    }
}

Hydroponics_UnitsType defaultLiquidDilutionUnits(Hydroponics_MeasurementMode measureMode)
{
    if (measureMode == Hydroponics_MeasurementMode_Undefined) {
        measureMode = (getHydroponicsInstance() ? getHydroponicsInstance()->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
    }

    switch (measureMode) {
        case Hydroponics_MeasurementMode_Imperial:
            return Hydroponics_UnitsType_LiqDilution_MilliLiterPerGallon;
        case Hydroponics_MeasurementMode_Metric:
        case Hydroponics_MeasurementMode_Scientific:
            return Hydroponics_UnitsType_LiqDilution_MilliLiterPerLiter;
        default:
            return Hydroponics_UnitsType_Undefined;
    }
}

int defaultDecimalPlaces(Hydroponics_MeasurementMode measureMode)
{
    if (measureMode == Hydroponics_MeasurementMode_Undefined) {
        measureMode = (getHydroponicsInstance() ? getHydroponicsInstance()->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
    }

    switch (measureMode) {
        case Hydroponics_MeasurementMode_Scientific:
            return 2;
        default:
            return 1;
    }
}


int linksCountCrops(Pair<uint8_t, Pair<HydroponicsObject *, int8_t> *> links)
{
    int retVal = 0;

    for (int linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
        if (links.second[linksIndex].first->isCropType()) {
            retVal++;
        }
    }

    return retVal;
}

int linksCountActuatorsByReservoirAndType(Pair<uint8_t, Pair<HydroponicsObject *, int8_t> *> links, HydroponicsReservoir *srcReservoir, Hydroponics_ActuatorType actuatorType)
{
    int retVal = 0;

    for (int linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
        if (links.second[linksIndex].first->isActuatorType()) {
            auto actuator = static_cast<HydroponicsActuator *>(links.second[linksIndex].first);

            if (actuator->getActuatorType() == actuatorType && actuator->getReservoir().get() == srcReservoir) {
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
            #if NUM_ANALOG_INPUTS > 1 && !defined(ESP32)
                case (pintype_t)A1:
            #endif
            #if NUM_ANALOG_INPUTS > 2 && !defined(ESP32)
                case (pintype_t)A2:
            #endif
            #if NUM_ANALOG_INPUTS > 3
                case (pintype_t)A3:
            #endif
            #if NUM_ANALOG_INPUTS > 4
                case (pintype_t)A4:
            #endif
            #if NUM_ANALOG_INPUTS > 5
                case (pintype_t)A5:
            #endif
            #if NUM_ANALOG_INPUTS > 6
                case (pintype_t)A6:
            #endif
            #if NUM_ANALOG_INPUTS > 7
                case (pintype_t)A7:
            #endif
            #if NUM_ANALOG_INPUTS > 8 && !defined(ESP32)
                case (pintype_t)A8:
            #endif
            #if NUM_ANALOG_INPUTS > 9 && !defined(ESP32)
                case (pintype_t)A9:
            #endif
            #if NUM_ANALOG_INPUTS > 10
                case (pintype_t)A10:
            #endif
            #if NUM_ANALOG_INPUTS > 11
                case (pintype_t)A11:
            #endif
            #if NUM_ANALOG_INPUTS > 12
                case (pintype_t)A12:
            #endif
            #if NUM_ANALOG_INPUTS > 13
                case (pintype_t)A13:
            #endif
            #if NUM_ANALOG_INPUTS > 14
                case (pintype_t)A14:
            #endif
            #if NUM_ANALOG_INPUTS > 15
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


String systemModeToString(Hydroponics_SystemMode systemMode, bool excludeSpecial)
{
    switch (systemMode) {
        case Hydroponics_SystemMode_Recycling:
            return SFP(HStr_Enum_Recycling);
        case Hydroponics_SystemMode_DrainToWaste:
            return SFP(HStr_Enum_DrainToWaste);
        case Hydroponics_SystemMode_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydroponics_SystemMode_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String measurementModeToString(Hydroponics_MeasurementMode measurementMode, bool excludeSpecial)
{
    switch (measurementMode) {
        case Hydroponics_MeasurementMode_Imperial:
            return SFP(HStr_Enum_Imperial);
        case Hydroponics_MeasurementMode_Metric:
            return SFP(HStr_Enum_Metric);
        case Hydroponics_MeasurementMode_Scientific:
            return SFP(HStr_Enum_Scientific);
        case Hydroponics_MeasurementMode_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydroponics_MeasurementMode_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String displayOutputModeToString(Hydroponics_DisplayOutputMode displayOutMode, bool excludeSpecial)
{
    switch (displayOutMode) {
        case Hydroponics_DisplayOutputMode_Disabled:
            return SFP(HStr_Disabled);
        case Hydroponics_DisplayOutputMode_20x4LCD:
            return SFP(HStr_Enum_20x4LCD);
        case Hydroponics_DisplayOutputMode_20x4LCD_Swapped:
            return SFP(HStr_Enum_20x4LCDSwapped);
        case Hydroponics_DisplayOutputMode_16x2LCD:
            return SFP(HStr_Enum_16x2LCD);
        case Hydroponics_DisplayOutputMode_16x2LCD_Swapped:
            return SFP(HStr_Enum_16x2LCDSwapped);
        case Hydroponics_DisplayOutputMode_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydroponics_DisplayOutputMode_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String controlInputModeToString(Hydroponics_ControlInputMode controlInMode, bool excludeSpecial)
{
    switch (controlInMode) {
        case Hydroponics_ControlInputMode_Disabled:
            return SFP(HStr_Disabled);
        case Hydroponics_ControlInputMode_2x2Matrix:
            return SFP(HStr_Enum_2x2Matrix);
        case Hydroponics_ControlInputMode_4xButton:
            return SFP(HStr_Enum_4xButton);
        case Hydroponics_ControlInputMode_6xButton:
            return SFP(HStr_Enum_6xButton);
        case Hydroponics_ControlInputMode_RotaryEncoder:
            return SFP(HStr_Enum_RotaryEncoder);
        case Hydroponics_ControlInputMode_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydroponics_ControlInputMode_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

bool getActuatorInWaterFromType(Hydroponics_ActuatorType actuatorType)
{
    switch (actuatorType) {
        case Hydroponics_ActuatorType_WaterPump:
        case Hydroponics_ActuatorType_WaterHeater:
        case Hydroponics_ActuatorType_WaterAerator:
            return true;

        default:
            return false;
    }
}

bool getActuatorIsPumpFromType(Hydroponics_ActuatorType actuatorType)
{
    switch (actuatorType) {
        case Hydroponics_ActuatorType_WaterPump:
        case Hydroponics_ActuatorType_PeristalticPump:
            return true;

        default:
            return false;
    }
}

String actuatorTypeToString(Hydroponics_ActuatorType actuatorType, bool excludeSpecial)
{
    switch (actuatorType) {
        case Hydroponics_ActuatorType_GrowLights:
            return SFP(HStr_Enum_GrowLights);
        case Hydroponics_ActuatorType_WaterPump:
            return SFP(HStr_Enum_WaterPump);
        case Hydroponics_ActuatorType_PeristalticPump:
            return SFP(HStr_Enum_PeristalticPump);
        case Hydroponics_ActuatorType_WaterHeater:
            return SFP(HStr_Enum_WaterHeater);
        case Hydroponics_ActuatorType_WaterAerator:
            return SFP(HStr_Enum_WaterAerator);
        case Hydroponics_ActuatorType_WaterSprayer:
            return SFP(HStr_Enum_WaterSprayer);
        case Hydroponics_ActuatorType_FanExhaust:
            return SFP(HStr_Enum_FanExhaust);
        case Hydroponics_ActuatorType_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydroponics_ActuatorType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String sensorTypeToString(Hydroponics_SensorType sensorType, bool excludeSpecial)
{
    switch (sensorType) {
        case Hydroponics_SensorType_PotentialHydrogen:
            return SFP(HStr_Enum_WaterPH);
        case Hydroponics_SensorType_TotalDissolvedSolids:
            return SFP(HStr_Enum_WaterTDS);
        case Hydroponics_SensorType_SoilMoisture:
            return SFP(HStr_Enum_SoilMoisture);
        case Hydroponics_SensorType_WaterTemperature:
            return SFP(HStr_Enum_WaterTemperature);
        case Hydroponics_SensorType_WaterPumpFlowSensor:
            return SFP(HStr_Enum_WaterPumpFlowSensor);
        case Hydroponics_SensorType_WaterLevelIndicator:
            return SFP(HStr_Enum_WaterLevelIndicator);
        case Hydroponics_SensorType_WaterHeightMeter:
            return SFP(HStr_Enum_WaterHeightMeter);
        case Hydroponics_SensorType_AirTempHumidity:
            return SFP(HStr_Enum_AirTemperatureHumidity);
        case Hydroponics_SensorType_AirCarbonDioxide:
            return SFP(HStr_Enum_AirCarbonDioxide);
        case Hydroponics_SensorType_PowerUsageMeter:
            return SFP(HStr_Enum_PowerUsageMeter);
        case Hydroponics_SensorType_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydroponics_SensorType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String cropTypeToString(Hydroponics_CropType cropType, bool excludeSpecial)
{
    switch (cropType) {
        case Hydroponics_CropType_AloeVera:
            return SFP(HStr_Enum_AloeVera);
        case Hydroponics_CropType_Anise:
            return SFP(HStr_Enum_Anise);
        case Hydroponics_CropType_Artichoke:
            return SFP(HStr_Enum_Artichoke);
        case Hydroponics_CropType_Arugula:
            return SFP(HStr_Enum_Arugula);
        case Hydroponics_CropType_Asparagus:
            return SFP(HStr_Enum_Asparagus);
        case Hydroponics_CropType_Basil:
            return SFP(HStr_Enum_Basil);
        case Hydroponics_CropType_Bean:
            return SFP(HStr_Enum_Bean);
        case Hydroponics_CropType_BeanBroad:
            return SFP(HStr_Enum_BeanBroad);
        case Hydroponics_CropType_Beetroot:
            return SFP(HStr_Enum_Beetroot);
        case Hydroponics_CropType_BlackCurrant:
            return SFP(HStr_Enum_BlackCurrant);
        case Hydroponics_CropType_Blueberry:
            return SFP(HStr_Enum_Blueberry);
        case Hydroponics_CropType_BokChoi:
            return SFP(HStr_Enum_BokChoi);
        case Hydroponics_CropType_Broccoli:
            return SFP(HStr_Enum_Broccoli);
        case Hydroponics_CropType_BrusselsSprout:
            return SFP(HStr_Enum_BrusselsSprout);
        case Hydroponics_CropType_Cabbage:
            return SFP(HStr_Enum_Cabbage);
        case Hydroponics_CropType_Cannabis:
            return SFP(HStr_Enum_Cannabis);
        case Hydroponics_CropType_Capsicum:
            return SFP(HStr_Enum_Capsicum);
        case Hydroponics_CropType_Carrots:
            return SFP(HStr_Enum_Carrots);
        case Hydroponics_CropType_Catnip:
            return SFP(HStr_Enum_Catnip);
        case Hydroponics_CropType_Cauliflower:
            return SFP(HStr_Enum_Cauliflower);
        case Hydroponics_CropType_Celery:
            return SFP(HStr_Enum_Celery);
        case Hydroponics_CropType_Chamomile:
            return SFP(HStr_Enum_Chamomile);
        case Hydroponics_CropType_Chicory:
            return SFP(HStr_Enum_Chicory);
        case Hydroponics_CropType_Chives:
            return SFP(HStr_Enum_Chives);
        case Hydroponics_CropType_Cilantro:
            return SFP(HStr_Enum_Cilantro);
        case Hydroponics_CropType_Coriander:
            return SFP(HStr_Enum_Coriander);
        case Hydroponics_CropType_CornSweet:
            return SFP(HStr_Enum_CornSweet);
        case Hydroponics_CropType_Cucumber:
            return SFP(HStr_Enum_Cucumber);
        case Hydroponics_CropType_Dill:
            return SFP(HStr_Enum_Dill);
        case Hydroponics_CropType_Eggplant:
            return SFP(HStr_Enum_Eggplant);
        case Hydroponics_CropType_Endive:
            return SFP(HStr_Enum_Endive);
        case Hydroponics_CropType_Fennel:
            return SFP(HStr_Enum_Fennel);
        case Hydroponics_CropType_Fodder:
            return SFP(HStr_Enum_Fodder);
        case Hydroponics_CropType_Flowers:
            return SFP(HStr_Enum_Flowers);
        case Hydroponics_CropType_Garlic:
            return SFP(HStr_Enum_Garlic);
        case Hydroponics_CropType_Ginger:
            return SFP(HStr_Enum_Ginger);
        case Hydroponics_CropType_Kale:
            return SFP(HStr_Enum_Kale);
        case Hydroponics_CropType_Lavender:
            return SFP(HStr_Enum_Lavender);
        case Hydroponics_CropType_Leek:
            return SFP(HStr_Enum_Leek);
        case Hydroponics_CropType_LemonBalm:
            return SFP(HStr_Enum_LemonBalm);
        case Hydroponics_CropType_Lettuce:
            return SFP(HStr_Enum_Lettuce);
        case Hydroponics_CropType_Marrow:
            return SFP(HStr_Enum_Marrow);
        case Hydroponics_CropType_Melon:
            return SFP(HStr_Enum_Melon);
        case Hydroponics_CropType_Mint:
            return SFP(HStr_Enum_Mint);
        case Hydroponics_CropType_MustardCress:
            return SFP(HStr_Enum_MustardCress);
        case Hydroponics_CropType_Okra:
            return SFP(HStr_Enum_Okra);
        case Hydroponics_CropType_Onions:
            return SFP(HStr_Enum_Onions);
        case Hydroponics_CropType_Oregano:
            return SFP(HStr_Enum_Oregano);
        case Hydroponics_CropType_PakChoi:
            return SFP(HStr_Enum_PakChoi);
        case Hydroponics_CropType_Parsley:
            return SFP(HStr_Enum_Parsley);
        case Hydroponics_CropType_Parsnip:
            return SFP(HStr_Enum_Parsnip);
        case Hydroponics_CropType_Pea:
            return SFP(HStr_Enum_Pea);
        case Hydroponics_CropType_PeaSugar:
            return SFP(HStr_Enum_PeaSugar);
        case Hydroponics_CropType_Pepino:
            return SFP(HStr_Enum_Pepino);
        case Hydroponics_CropType_PeppersBell:
            return SFP(HStr_Enum_PeppersBell);
        case Hydroponics_CropType_PeppersHot:
            return SFP(HStr_Enum_PeppersHot);
        case Hydroponics_CropType_Potato:
            return SFP(HStr_Enum_Potato);
        case Hydroponics_CropType_PotatoSweet:
            return SFP(HStr_Enum_PotatoSweet);
        case Hydroponics_CropType_Pumpkin:
            return SFP(HStr_Enum_Pumpkin);
        case Hydroponics_CropType_Radish:
            return SFP(HStr_Enum_Radish);
        case Hydroponics_CropType_Rhubarb:
            return SFP(HStr_Enum_Rhubarb);
        case Hydroponics_CropType_Rosemary:
            return SFP(HStr_Enum_Rosemary);
        case Hydroponics_CropType_Sage:
            return SFP(HStr_Enum_Sage);
        case Hydroponics_CropType_Silverbeet:
            return SFP(HStr_Enum_Silverbeet);
        case Hydroponics_CropType_Spinach:
            return SFP(HStr_Enum_Spinach);
        case Hydroponics_CropType_Squash:
            return SFP(HStr_Enum_Squash);
        case Hydroponics_CropType_Sunflower:
            return SFP(HStr_Enum_Sunflower);
        case Hydroponics_CropType_Strawberries:
            return SFP(HStr_Enum_Strawberries);
        case Hydroponics_CropType_SwissChard:
            return SFP(HStr_Enum_SwissChard);
        case Hydroponics_CropType_Taro:
            return SFP(HStr_Enum_Taro);
        case Hydroponics_CropType_Tarragon:
            return SFP(HStr_Enum_Tarragon);
        case Hydroponics_CropType_Thyme:
            return SFP(HStr_Enum_Thyme);
        case Hydroponics_CropType_Tomato:
            return SFP(HStr_Enum_Tomato);
        case Hydroponics_CropType_Turnip:
            return SFP(HStr_Enum_Turnip);
        case Hydroponics_CropType_Watercress:
            return SFP(HStr_Enum_Watercress);
        case Hydroponics_CropType_Watermelon:
            return SFP(HStr_Enum_Watermelon);
        case Hydroponics_CropType_Zucchini:
            return SFP(HStr_Enum_Zucchini);
        case Hydroponics_CropType_CustomCrop1:
            return SFP(HStr_Enum_CustomCrop1);
        case Hydroponics_CropType_CustomCrop2:
            return SFP(HStr_Enum_CustomCrop2);
        case Hydroponics_CropType_CustomCrop3:
            return SFP(HStr_Enum_CustomCrop3);
        case Hydroponics_CropType_CustomCrop4:
            return SFP(HStr_Enum_CustomCrop4);
        case Hydroponics_CropType_CustomCrop5:
            return SFP(HStr_Enum_CustomCrop5);
        case Hydroponics_CropType_CustomCrop6:
            return SFP(HStr_Enum_CustomCrop6);
        case Hydroponics_CropType_CustomCrop7:
            return SFP(HStr_Enum_CustomCrop7);
        case Hydroponics_CropType_CustomCrop8:
            return SFP(HStr_Enum_CustomCrop8);
        case Hydroponics_CropType_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydroponics_CropType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String substrateTypeToString(Hydroponics_SubstrateType substrateType, bool excludeSpecial)
{
    switch (substrateType) {
        case Hydroponics_SubstrateType_ClayPebbles:
            return SFP(HStr_Enum_ClayPebbles);
        case Hydroponics_SubstrateType_CoconutCoir:
            return SFP(HStr_Enum_CoconutCoir);
        case Hydroponics_SubstrateType_Rockwool:
            return SFP(HStr_Enum_Rockwool);
        case Hydroponics_SubstrateType_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydroponics_SubstrateType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String reservoirTypeToString(Hydroponics_ReservoirType reservoirType, bool excludeSpecial)
{
    switch (reservoirType) {
        case Hydroponics_ReservoirType_FeedWater:
            return SFP(HStr_Enum_FeedWater);
        case Hydroponics_ReservoirType_DrainageWater:
            return SFP(HStr_Enum_DrainageWater);
        case Hydroponics_ReservoirType_NutrientPremix:
            return SFP(HStr_Enum_NutrientPremix);
        case Hydroponics_ReservoirType_FreshWater:
            return SFP(HStr_Enum_FreshWater);
        case Hydroponics_ReservoirType_PhUpSolution:
            return SFP(HStr_Enum_PhUpSolution);
        case Hydroponics_ReservoirType_PhDownSolution:
            return SFP(HStr_Enum_PhDownSolution);
        case Hydroponics_ReservoirType_CustomAdditive1:
            return SFP(HStr_Enum_CustomAdditive1);
        case Hydroponics_ReservoirType_CustomAdditive2:
            return SFP(HStr_Enum_CustomAdditive2);
        case Hydroponics_ReservoirType_CustomAdditive3:
            return SFP(HStr_Enum_CustomAdditive3);
        case Hydroponics_ReservoirType_CustomAdditive4:
            return SFP(HStr_Enum_CustomAdditive4);
        case Hydroponics_ReservoirType_CustomAdditive5:
            return SFP(HStr_Enum_CustomAdditive5);
        case Hydroponics_ReservoirType_CustomAdditive6:
            return SFP(HStr_Enum_CustomAdditive6);
        case Hydroponics_ReservoirType_CustomAdditive7:
            return SFP(HStr_Enum_CustomAdditive7);
        case Hydroponics_ReservoirType_CustomAdditive8:
            return SFP(HStr_Enum_CustomAdditive8);
        case Hydroponics_ReservoirType_CustomAdditive9:
            return SFP(HStr_Enum_CustomAdditive9);
        case Hydroponics_ReservoirType_CustomAdditive10:
            return SFP(HStr_Enum_CustomAdditive10);
        case Hydroponics_ReservoirType_CustomAdditive11:
            return SFP(HStr_Enum_CustomAdditive11);
        case Hydroponics_ReservoirType_CustomAdditive12:
            return SFP(HStr_Enum_CustomAdditive12);
        case Hydroponics_ReservoirType_CustomAdditive13:
            return SFP(HStr_Enum_CustomAdditive13);
        case Hydroponics_ReservoirType_CustomAdditive14:
            return SFP(HStr_Enum_CustomAdditive14);
        case Hydroponics_ReservoirType_CustomAdditive15:
            return SFP(HStr_Enum_CustomAdditive15);
        case Hydroponics_ReservoirType_CustomAdditive16:
            return SFP(HStr_Enum_CustomAdditive16);
        case Hydroponics_ReservoirType_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydroponics_ReservoirType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

float getRailVoltageFromType(Hydroponics_RailType railType)
{
    switch (railType) {
        case Hydroponics_RailType_AC110V:
            return 110.0f;
        case Hydroponics_RailType_AC220V:
            return 220.0f;
        case Hydroponics_RailType_DC5V:
            return 5.0f;
        case Hydroponics_RailType_DC12V:
            return 12.0f;
        default:
            return 0.0f;
    }
}

String railTypeToString(Hydroponics_RailType railType, bool excludeSpecial)
{
    switch (railType) {
        case Hydroponics_RailType_AC110V:
            return SFP(HStr_Enum_AC110V);
        case Hydroponics_RailType_AC220V:
            return SFP(HStr_Enum_AC220V);
        case Hydroponics_RailType_DC5V:
            return SFP(HStr_Enum_DC5V);
        case Hydroponics_RailType_DC12V:
            return SFP(HStr_Enum_DC12V);
        case Hydroponics_RailType_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydroponics_RailType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String unitsCategoryToString(Hydroponics_UnitsCategory unitsCategory, bool excludeSpecial)
{
    switch (unitsCategory) {
        case Hydroponics_UnitsCategory_Alkalinity:
            return SFP(HStr_Enum_Alkalinity);
        case Hydroponics_UnitsCategory_DissolvedSolids:
            return SFP(HStr_Enum_DissolvedSolids);
        case Hydroponics_UnitsCategory_SoilMoisture:
            return SFP(HStr_Enum_SoilMoisture);
        case Hydroponics_UnitsCategory_LiqTemperature:
            return SFP(HStr_Enum_LiqTemperature);
        case Hydroponics_UnitsCategory_LiqVolume:
            return SFP(HStr_Enum_LiqVolume);
        case Hydroponics_UnitsCategory_LiqFlowRate:
            return SFP(HStr_Enum_LiqFlowRate);
        case Hydroponics_UnitsCategory_LiqDilution:
            return SFP(HStr_Enum_LiqDilution);
        case Hydroponics_UnitsCategory_AirTemperature:
            return SFP(HStr_Enum_AirTemperature);
        case Hydroponics_UnitsCategory_AirHumidity:
            return SFP(HStr_Enum_AirHumidity);
        case Hydroponics_UnitsCategory_AirHeatIndex:
            return SFP(HStr_Enum_AirHeatIndex);
        case Hydroponics_UnitsCategory_AirConcentration:
            return SFP(HStr_Enum_AirConcentration);
        case Hydroponics_UnitsCategory_Distance:
            return SFP(HStr_Enum_Distance);
        case Hydroponics_UnitsCategory_Weight:
            return SFP(HStr_Enum_Weight);
        case Hydroponics_UnitsCategory_Power:
            return SFP(HStr_Enum_Power);
        case Hydroponics_UnitsCategory_Count:
            return !excludeSpecial ? SFP(HStr_Count) : String();
        case Hydroponics_UnitsCategory_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Undefined) : String();
}

String unitsTypeToSymbol(Hydroponics_UnitsType unitsType, bool excludeSpecial)
{
    switch (unitsType) {
        case Hydroponics_UnitsType_Raw_0_1:
            return SFP(HStr_raw);
        case Hydroponics_UnitsType_Percentile_0_100:
            return SFP(HStr_Unit_Percentile);
        case Hydroponics_UnitsType_Alkalinity_pH_0_14:
            return !excludeSpecial ? SFP(HStr_Unit_pH) : String(); // technically unitless
        case Hydroponics_UnitsType_Concentration_EC:
            return SFP(HStr_Unit_EC); // alt: mS/cm, TDS
        case Hydroponics_UnitsType_Temperature_Celsius:
            return SFP(HStr_Unit_Celsius);
        case Hydroponics_UnitsType_Temperature_Fahrenheit:
            return SFP(HStr_Unit_Fahrenheit);
        case Hydroponics_UnitsType_Temperature_Kelvin:
            return SFP(HStr_Unit_Kelvin);
        case Hydroponics_UnitsType_LiqVolume_Liters:
            return SFP(HStr_Unit_Liters);
        case Hydroponics_UnitsType_LiqVolume_Gallons:
            return SFP(HStr_Unit_Gallons);
        case Hydroponics_UnitsType_LiqFlowRate_LitersPerMin:
            return SFP(HStr_Unit_LitersPerMin);
        case Hydroponics_UnitsType_LiqFlowRate_GallonsPerMin:
            return SFP(HStr_Unit_GallonsPerMin);
        case Hydroponics_UnitsType_LiqDilution_MilliLiterPerLiter:
            return SFP(HStr_Unit_MilliLiterPerLiter);
        case Hydroponics_UnitsType_LiqDilution_MilliLiterPerGallon:
            return SFP(HStr_Unit_MilliLiterPerGallon);
        case Hydroponics_UnitsType_Concentration_PPM:
            return SFP(HStr_Unit_PPM500);
        case Hydroponics_UnitsType_Concentration_PPM640:
            return SFP(HStr_Unit_PPM640);
        case Hydroponics_UnitsType_Concentration_PPM700:
            return SFP(HStr_Unit_PPM700);
        case Hydroponics_UnitsType_Distance_Meters:
            return SFP(HStr_Unit_Meters);
        case Hydroponics_UnitsType_Distance_Feet:
            return SFP(HStr_Unit_Feet);
        case Hydroponics_UnitsType_Weight_Kilogram:
            return SFP(HStr_Unit_Kilogram);
        case Hydroponics_UnitsType_Weight_Pounds:
            return SFP(HStr_Unit_Pounds);
        case Hydroponics_UnitsType_Power_Wattage:
            return SFP(HStr_Unit_Wattage); // alt: J/s
        case Hydroponics_UnitsType_Power_Amperage:
            return SFP(HStr_Unit_Amperage);
        case Hydroponics_UnitsType_Count:
            return !excludeSpecial ? SFP(HStr_Unit_Count) : String();
        case Hydroponics_UnitsType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(HStr_Unit_Undefined) : String();
}

String positionIndexToString(Hydroponics_PositionIndex positionIndex, bool excludeSpecial)
{
    if (positionIndex >= 0 && positionIndex < HYDRUINO_POS_MAXSIZE) {
        return String(positionIndex + HYDRUINO_POS_EXPORT_BEGFROM);
    } else if (!excludeSpecial) {
        if (positionIndex == HYDRUINO_POS_MAXSIZE) {
            return SFP(HStr_Count);
        } else {
            return SFP(HStr_Undefined);
        }
    }
    return String();
}

Hydroponics_PositionIndex positionIndexFromString(String positionIndexStr)
{
    if (positionIndexStr == positionIndexToString(HYDRUINO_POS_MAXSIZE)) {
        return HYDRUINO_POS_MAXSIZE;
    } else if (positionIndexStr == positionIndexToString(-1)) {
        return -1;
    } else {
        int8_t decode = positionIndexStr.toInt();
        return decode >= 0 && decode < HYDRUINO_POS_MAXSIZE ? decode : -1;
    }
}


// All remaining methods generated from minimum spanning trie

Hydroponics_SystemMode systemModeFromString(String systemModeStr)
{
    switch (systemModeStr.length() >= 1 ? systemModeStr[0] : '\0') {
        case 'R':
            return (Hydroponics_SystemMode)0;
        case 'D':
            return (Hydroponics_SystemMode)1;
        case 'C':
            return (Hydroponics_SystemMode)2;
    }
    return Hydroponics_SystemMode_Undefined;
}

Hydroponics_MeasurementMode measurementModeFromString(String measurementModeStr)
{
    switch (measurementModeStr.length() >= 1 ? measurementModeStr[0] : '\0') {
        case 'I':
            return (Hydroponics_MeasurementMode)0;
        case 'M':
            return (Hydroponics_MeasurementMode)1;
        case 'S':
            return (Hydroponics_MeasurementMode)2;
        case 'C':
            return (Hydroponics_MeasurementMode)3;
    }
    return Hydroponics_MeasurementMode_Undefined;
}

Hydroponics_DisplayOutputMode displayOutputModeFromString(String displayOutModeStr)
{
    switch (displayOutModeStr.length() >= 1 ? displayOutModeStr[0] : '\0') {
        case 'D':
            return (Hydroponics_DisplayOutputMode)0;
        case '2':
            switch (displayOutModeStr.length() >= 8 ? displayOutModeStr[7] : '\0') {
                case '\0':
                    return (Hydroponics_DisplayOutputMode)1;
                case 'S':
                    return (Hydroponics_DisplayOutputMode)2;
            }
            break;
        case '1':
            switch (displayOutModeStr.length() >= 8 ? displayOutModeStr[7] : '\0') {
                case '\0':
                    return (Hydroponics_DisplayOutputMode)3;
                case 'S':
                    return (Hydroponics_DisplayOutputMode)4;
            }
            break;
        case 'C':
            return (Hydroponics_DisplayOutputMode)5;
    }
    return Hydroponics_DisplayOutputMode_Undefined;
}

Hydroponics_ControlInputMode controlInputModeFromString(String controlInModeStr)
{
    switch (controlInModeStr.length() >= 1 ? controlInModeStr[0] : '\0') {
        case 'D':
            return (Hydroponics_ControlInputMode)0;
        case '2':
            return (Hydroponics_ControlInputMode)1;
        case '4':
            return (Hydroponics_ControlInputMode)2;
        case '6':
            return (Hydroponics_ControlInputMode)3;
        case 'R':
            return (Hydroponics_ControlInputMode)4;
        case 'C':
            return (Hydroponics_ControlInputMode)5;
    }
    return Hydroponics_ControlInputMode_Undefined;
}

Hydroponics_ActuatorType actuatorTypeFromString(String actuatorTypeStr)
{
    switch (actuatorTypeStr.length() >= 1 ? actuatorTypeStr[0] : '\0') {
        case 'G':
            return (Hydroponics_ActuatorType)0;
        case 'W':
            switch (actuatorTypeStr.length() >= 6 ? actuatorTypeStr[5] : '\0') {
                case 'P':
                    return (Hydroponics_ActuatorType)1;
                case 'H':
                    return (Hydroponics_ActuatorType)3;
                case 'A':
                    return (Hydroponics_ActuatorType)4;
                case 'S':
                    return (Hydroponics_ActuatorType)5;
            }
            break;
        case 'P':
            return (Hydroponics_ActuatorType)2;
        case 'F':
            return (Hydroponics_ActuatorType)6;
        case 'C':
            return (Hydroponics_ActuatorType)7;
    }
    return Hydroponics_ActuatorType_Undefined;
}

Hydroponics_SensorType sensorTypeFromString(String sensorTypeStr)
{
    switch (sensorTypeStr.length() >= 1 ? sensorTypeStr[0] : '\0') {
        case 'W':
            switch (sensorTypeStr.length() >= 6 ? sensorTypeStr[5] : '\0') {
                case 'P':
                    return (Hydroponics_SensorType)0;
                case 'T':
                    switch (sensorTypeStr.length() >= 7 ? sensorTypeStr[6] : '\0') {
                        case 'D':
                            return (Hydroponics_SensorType)1;
                        case 'e':
                            return (Hydroponics_SensorType)3;
                    }
                    break;
                case 'H':
                    return (Hydroponics_SensorType)6;
            }
            break;
        case 'S':
            return (Hydroponics_SensorType)2;
        case 'P':
            switch (sensorTypeStr.length() >= 2 ? sensorTypeStr[1] : '\0') {
                case 'u':
                    return (Hydroponics_SensorType)4;
                case 'o':
                    return (Hydroponics_SensorType)9;
            }
            break;
        case 'L':
            return (Hydroponics_SensorType)5;
        case 'A':
            switch (sensorTypeStr.length() >= 4 ? sensorTypeStr[3] : '\0') {
                case 'T':
                    return (Hydroponics_SensorType)7;
                case 'C':
                    return (Hydroponics_SensorType)8;
            }
            break;
        case 'C':
            return (Hydroponics_SensorType)10;
    }
    return Hydroponics_SensorType_Undefined;
}

Hydroponics_CropType cropTypeFromString(String cropTypeStr)
{
    switch (cropTypeStr.length() >= 1 ? cropTypeStr[0] : '\0') {
        case 'A':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'l':
                    return (Hydroponics_CropType)0;
                case 'n':
                    return (Hydroponics_CropType)1;
                case 'r':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 't':
                            return (Hydroponics_CropType)2;
                        case 'u':
                            return (Hydroponics_CropType)3;
                    }
                    break;
                case 's':
                    return (Hydroponics_CropType)4;
            }
            break;
        case 'B':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    return (Hydroponics_CropType)5;
                case 'e':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'a':
                            switch (cropTypeStr.length() >= 5 ? cropTypeStr[4] : '\0') {
                                case '\0':
                                    return (Hydroponics_CropType)6;
                                case 'B':
                                    return (Hydroponics_CropType)7;
                            }
                            break;
                        case 'e':
                            return (Hydroponics_CropType)8;
                    }
                    break;
                case 'l':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'a':
                            return (Hydroponics_CropType)9;
                        case 'u':
                            return (Hydroponics_CropType)10;
                    }
                    break;
                case 'o':
                    return (Hydroponics_CropType)11;
                case 'r':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'o':
                            return (Hydroponics_CropType)12;
                        case 'u':
                            return (Hydroponics_CropType)13;
                    }
                    break;
            }
            break;
        case 'C':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'b':
                            return (Hydroponics_CropType)14;
                        case 'n':
                            return (Hydroponics_CropType)15;
                        case 'p':
                            return (Hydroponics_CropType)16;
                        case 'r':
                            return (Hydroponics_CropType)17;
                        case 't':
                            return (Hydroponics_CropType)18;
                        case 'u':
                            return (Hydroponics_CropType)19;
                    }
                    break;
                case 'e':
                    return (Hydroponics_CropType)20;
                case 'h':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'a':
                            return (Hydroponics_CropType)21;
                        case 'i':
                            switch (cropTypeStr.length() >= 4 ? cropTypeStr[3] : '\0') {
                                case 'c':
                                    return (Hydroponics_CropType)22;
                                case 'v':
                                    return (Hydroponics_CropType)23;
                            }
                            break;
                    }
                    break;
                case 'i':
                    return (Hydroponics_CropType)24;
                case 'o':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'r':
                            switch (cropTypeStr.length() >= 4 ? cropTypeStr[3] : '\0') {
                                case 'i':
                                    return (Hydroponics_CropType)25;
                                case 'n':
                                    return (Hydroponics_CropType)26;
                            }
                            break;
                        case 'u':
                            return (Hydroponics_CropType)85;
                    }
                    break;
                case 'u':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'c':
                            return (Hydroponics_CropType)27;
                        case 's':
                            switch (cropTypeStr.length() >= 11 ? cropTypeStr[10] : '\0') {
                                case '1':
                                    return (Hydroponics_CropType)77;
                                case '2':
                                    return (Hydroponics_CropType)78;
                                case '3':
                                    return (Hydroponics_CropType)79;
                                case '4':
                                    return (Hydroponics_CropType)80;
                                case '5':
                                    return (Hydroponics_CropType)81;
                                case '6':
                                    return (Hydroponics_CropType)82;
                                case '7':
                                    return (Hydroponics_CropType)83;
                                case '8':
                                    return (Hydroponics_CropType)84;
                            }
                            break;
                    }
                    break;
            }
            break;
        case 'D':
            return (Hydroponics_CropType)28;
        case 'E':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'g':
                    return (Hydroponics_CropType)29;
                case 'n':
                    return (Hydroponics_CropType)30;
            }
            break;
        case 'F':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'e':
                    return (Hydroponics_CropType)31;
                case 'o':
                    return (Hydroponics_CropType)32;
                case 'l':
                    return (Hydroponics_CropType)33;
            }
            break;
        case 'G':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    return (Hydroponics_CropType)34;
                case 'i':
                    return (Hydroponics_CropType)35;
            }
            break;
        case 'K':
            return (Hydroponics_CropType)36;
        case 'L':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    return (Hydroponics_CropType)37;
                case 'e':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'e':
                            return (Hydroponics_CropType)38;
                        case 'm':
                            return (Hydroponics_CropType)39;
                        case 't':
                            return (Hydroponics_CropType)40;
                    }
                    break;
            }
            break;
        case 'M':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    return (Hydroponics_CropType)41;
                case 'e':
                    return (Hydroponics_CropType)42;
                case 'i':
                    return (Hydroponics_CropType)43;
                case 'u':
                    return (Hydroponics_CropType)44;
            }
            break;
        case 'O':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'k':
                    return (Hydroponics_CropType)45;
                case 'n':
                    return (Hydroponics_CropType)46;
                case 'r':
                    return (Hydroponics_CropType)47;
            }
            break;
        case 'P':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'k':
                            return (Hydroponics_CropType)48;
                        case 'r':
                            switch (cropTypeStr.length() >= 5 ? cropTypeStr[4] : '\0') {
                                case 'l':
                                    return (Hydroponics_CropType)49;
                                case 'n':
                                    return (Hydroponics_CropType)50;
                            }
                            break;
                    }
                    break;
                case 'e':
                    switch (cropTypeStr.length() >= 3 ? cropTypeStr[2] : '\0') {
                        case 'a':
                            switch (cropTypeStr.length() >= 4 ? cropTypeStr[3] : '\0') {
                                case '\0':
                                    return (Hydroponics_CropType)51;
                                case 'S':
                                    return (Hydroponics_CropType)52;
                            }
                            break;
                        case 'p':
                            switch (cropTypeStr.length() >= 4 ? cropTypeStr[3] : '\0') {
                                case 'i':
                                    return (Hydroponics_CropType)53;
                                case 'p':
                                    switch (cropTypeStr.length() >= 8 ? cropTypeStr[7] : '\0') {
                                        case 'B':
                                            return (Hydroponics_CropType)54;
                                        case 'H':
                                            return (Hydroponics_CropType)55;
                                    }
                                    break;
                            }
                            break;
                    }
                    break;
                case 'o':
                    switch (cropTypeStr.length() >= 7 ? cropTypeStr[6] : '\0') {
                        case '\0':
                            return (Hydroponics_CropType)56;
                        case 'S':
                            return (Hydroponics_CropType)57;
                    }
                    break;
                case 'u':
                    return (Hydroponics_CropType)58;
            }
            break;
        case 'R':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    return (Hydroponics_CropType)59;
                case 'h':
                    return (Hydroponics_CropType)60;
                case 'o':
                    return (Hydroponics_CropType)61;
            }
            break;
        case 'S':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    return (Hydroponics_CropType)62;
                case 'i':
                    return (Hydroponics_CropType)63;
                case 'p':
                    return (Hydroponics_CropType)64;
                case 'q':
                    return (Hydroponics_CropType)65;
                case 'u':
                    return (Hydroponics_CropType)66;
                case 't':
                    return (Hydroponics_CropType)67;
                case 'w':
                    return (Hydroponics_CropType)68;
            }
            break;
        case 'T':
            switch (cropTypeStr.length() >= 2 ? cropTypeStr[1] : '\0') {
                case 'a':
                    switch (cropTypeStr.length() >= 4 ? cropTypeStr[3] : '\0') {
                        case 'o':
                            return (Hydroponics_CropType)69;
                        case 'r':
                            return (Hydroponics_CropType)70;
                    }
                    break;
                case 'h':
                    return (Hydroponics_CropType)71;
                case 'o':
                    return (Hydroponics_CropType)72;
                case 'u':
                    return (Hydroponics_CropType)73;
            }
            break;
        case 'W':
            switch (cropTypeStr.length() >= 6 ? cropTypeStr[5] : '\0') {
                case 'c':
                    return (Hydroponics_CropType)74;
                case 'm':
                    return (Hydroponics_CropType)75;
            }
            break;
        case 'Z':
            return (Hydroponics_CropType)76;
    }
    return Hydroponics_CropType_Undefined;
}

Hydroponics_SubstrateType substrateTypeFromString(String substrateTypeStr)
{
    switch (substrateTypeStr.length() >= 1 ? substrateTypeStr[0] : '\0') {
        case 'C':
            switch (substrateTypeStr.length() >= 2 ? substrateTypeStr[1] : '\0') {
                case 'l':
                    return (Hydroponics_SubstrateType)0;
                case 'o':
                    switch (substrateTypeStr.length() >= 3 ? substrateTypeStr[2] : '\0') {
                        case 'c':
                            return (Hydroponics_SubstrateType)1;
                        case 'u':
                            return (Hydroponics_SubstrateType)3;
                    }
                    break;
            }
            break;
        case 'R':
            return (Hydroponics_SubstrateType)2;
    }
    return Hydroponics_SubstrateType_Undefined;
}

Hydroponics_ReservoirType reservoirTypeFromString(String reservoirTypeStr)
{
    switch (reservoirTypeStr.length() >= 1 ? reservoirTypeStr[0] : '\0') {
        case 'F':
            switch (reservoirTypeStr.length() >= 2 ? reservoirTypeStr[1] : '\0') {
                case 'e':
                    return (Hydroponics_ReservoirType)0;
                case 'r':
                    return (Hydroponics_ReservoirType)3;
            }
            break;
        case 'D':
            return (Hydroponics_ReservoirType)1;
        case 'N':
            return (Hydroponics_ReservoirType)2;
        case 'P':
            switch (reservoirTypeStr.length() >= 3 ? reservoirTypeStr[2] : '\0') {
                case 'U':
                    return (Hydroponics_ReservoirType)4;
                case 'D':
                    return (Hydroponics_ReservoirType)5;
            }
            break;
        case 'C':
            switch (reservoirTypeStr.length() >= 2 ? reservoirTypeStr[1] : '\0') {
                case 'u':
                    switch (reservoirTypeStr.length() >= 15 ? reservoirTypeStr[14] : '\0') {
                        case '1':
                            switch (reservoirTypeStr.length() >= 16 ? reservoirTypeStr[15] : '\0') {
                                case '\0':
                                    return (Hydroponics_ReservoirType)6;
                                case '0':
                                    return (Hydroponics_ReservoirType)15;
                                case '1':
                                    return (Hydroponics_ReservoirType)16;
                                case '2':
                                    return (Hydroponics_ReservoirType)17;
                                case '3':
                                    return (Hydroponics_ReservoirType)18;
                                case '4':
                                    return (Hydroponics_ReservoirType)19;
                                case '5':
                                    return (Hydroponics_ReservoirType)20;
                                case '6':
                                    return (Hydroponics_ReservoirType)21;
                            }
                            break;
                        case '2':
                            return (Hydroponics_ReservoirType)7;
                        case '3':
                            return (Hydroponics_ReservoirType)8;
                        case '4':
                            return (Hydroponics_ReservoirType)9;
                        case '5':
                            return (Hydroponics_ReservoirType)10;
                        case '6':
                            return (Hydroponics_ReservoirType)11;
                        case '7':
                            return (Hydroponics_ReservoirType)12;
                        case '8':
                            return (Hydroponics_ReservoirType)13;
                        case '9':
                            return (Hydroponics_ReservoirType)14;
                    }
                    break;
                case 'o':
                    return (Hydroponics_ReservoirType)22;
            }
            break;
    }
    return Hydroponics_ReservoirType_Undefined;
}

Hydroponics_RailType railTypeFromString(String railTypeStr) {
    switch (railTypeStr.length() >= 1 ? railTypeStr[0] : '\0') {
        case 'A':
            switch (railTypeStr.length() >= 3 ? railTypeStr[2] : '\0') {
                case '1':
                    return (Hydroponics_RailType)0;
                case '2':
                    return (Hydroponics_RailType)1;
            }
            break;
        case 'C':
            return (Hydroponics_RailType)4;
        case 'D':
            switch (railTypeStr.length() >= 3 ? railTypeStr[2] : '\0') {
                case '5':
                    return (Hydroponics_RailType)2;
                case '1':
                    return (Hydroponics_RailType)3;
            }
            break;
    }
    return Hydroponics_RailType_Undefined;
}

Hydroponics_UnitsCategory unitsCategoryFromString(String unitsCategoryStr)
{
    switch (unitsCategoryStr.length() >= 1 ? unitsCategoryStr[0] : '\0') {
        case 'A':
            switch (unitsCategoryStr.length() >= 2 ? unitsCategoryStr[1] : '\0') {
                case 'l':
                    return (Hydroponics_UnitsCategory)0;
                case 'i':
                    switch (unitsCategoryStr.length() >= 4 ? unitsCategoryStr[3] : '\0') {
                        case 'T':
                            return (Hydroponics_UnitsCategory)7;
                        case 'H':
                            switch (unitsCategoryStr.length() >= 5 ? unitsCategoryStr[4] : '\0') {
                                case 'u':
                                    return (Hydroponics_UnitsCategory)8;
                                case 'e':
                                    return (Hydroponics_UnitsCategory)9;
                            }
                            break;
                        case 'C':
                            return (Hydroponics_UnitsCategory)10;
                    }
                    break;
            }
            break;
        case 'D':
            switch (unitsCategoryStr.length() >= 4 ? unitsCategoryStr[3] : '\0') {
                case 's':
                    return (Hydroponics_UnitsCategory)1;
                case 't':
                    return (Hydroponics_UnitsCategory)11;
            }
            break;
        case 'S':
            return (Hydroponics_UnitsCategory)2;
        case 'L':
            switch (unitsCategoryStr.length() >= 4 ? unitsCategoryStr[3] : '\0') {
                case 'T':
                    return (Hydroponics_UnitsCategory)3;
                case 'V':
                    return (Hydroponics_UnitsCategory)4;
                case 'F':
                    return (Hydroponics_UnitsCategory)5;
                case 'D':
                    return (Hydroponics_UnitsCategory)6;
            }
            break;
        case 'W':
            return (Hydroponics_UnitsCategory)12;
        case 'P':
            return (Hydroponics_UnitsCategory)13;
        case 'C':
            return (Hydroponics_UnitsCategory)14;
    }
    return Hydroponics_UnitsCategory_Undefined;
}

Hydroponics_UnitsType unitsTypeFromSymbol(String unitsSymbolStr)
{
    switch (unitsSymbolStr.length() >= 1 ? unitsSymbolStr[0] : '\0') {
        case 'A':
            return (Hydroponics_UnitsType)21;
        case 'E':
            return (Hydroponics_UnitsType)3;
        case 'f':
            return (Hydroponics_UnitsType)17;
        case 'g':
            switch (unitsSymbolStr.length() >= 4 ? unitsSymbolStr[3] : '\0') {
                case '\0':
                    return (Hydroponics_UnitsType)8;
                case '/':
                    return (Hydroponics_UnitsType)10;
            }
            break;
        case 'J':
            return (Hydroponics_UnitsType)20;
        case 'K':
            return (Hydroponics_UnitsType)18;
        case 'L':
            switch (unitsSymbolStr.length() >= 2 ? unitsSymbolStr[1] : '\0') {
                case '\0':
                    return (Hydroponics_UnitsType)7;
                case '/':
                    return (Hydroponics_UnitsType)9;
            }
            break;
        case 'l':
            return (Hydroponics_UnitsType)19;
        case 'm':
            switch (unitsSymbolStr.length() >= 2 ? unitsSymbolStr[1] : '\0') {
                case 'L':
                    switch (unitsSymbolStr.length() >= 4 ? unitsSymbolStr[3] : '\0') {
                        case 'L':
                            return (Hydroponics_UnitsType)11;
                        case 'g':
                            return (Hydroponics_UnitsType)12;
                    }
                    break;
                case 'S':
                    return (Hydroponics_UnitsType)3;
                case '\0':
                    return (Hydroponics_UnitsType)16;
            }
            break;
        case 'p':
            switch (unitsSymbolStr.length() >= 2 ? unitsSymbolStr[1] : '\0') {
                case 'H':
                    return (Hydroponics_UnitsType)2;
                case 'p':
                    switch (unitsSymbolStr.length() >= 5 ? unitsSymbolStr[4] : '\0') {
                        case '\0':
                        case '5':
                            return (Hydroponics_UnitsType)13;
                        case '6':
                            return (Hydroponics_UnitsType)14;
                        case '7':
                            return (Hydroponics_UnitsType)15;
                    }
                    break;
            }
            break;
        case 'q':
            return (Hydroponics_UnitsType)22;
        case 'r':
            return (Hydroponics_UnitsType)0;
        case 'T':
            return (Hydroponics_UnitsType)3;
        case 'W':
            return (Hydroponics_UnitsType)20;
        case '%':
            return (Hydroponics_UnitsType)1;
        default:
            switch (unitsSymbolStr.length() >= 3 ? unitsSymbolStr[2] : '\0') {
                case 'C':
                    return (Hydroponics_UnitsType)4;
                case 'F':
                    return (Hydroponics_UnitsType)5;
                case 'K':
                    return (Hydroponics_UnitsType)6;
            }
            break;
    }
    return Hydroponics_UnitsType_Undefined;
}
