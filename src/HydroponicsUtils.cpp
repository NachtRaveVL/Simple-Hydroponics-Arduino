/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Utilities
*/

#include "Hydroponics.h"
#include <pins_arduino.h>

HydroponicsBitResolution::HydroponicsBitResolution(byte bitResIn, bool override)
    : // TODO: Determine which other architectures have variable bit res analog pins
      #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
          bitRes(constrain(bitResIn, 8, 12)), maxVal(1 << constrain(bitResIn, 8, 12))
      #else
          bitRes(8), maxVal(256)
      #endif
{
    if (override) {
        bitRes = bitResIn;
        maxVal = 1 << bitResIn;
    } else {
        HYDRUINO_SOFT_ASSERT(bitRes == bitResIn, F("Resolved resolution mismatch with passed resolution"));
    }
}


ActuatorTimedEnableTask::ActuatorTimedEnableTask(shared_ptr<HydroponicsActuator> actuator, float enableIntensity, time_t enableTimeMillis)
    : taskId(TASKMGR_INVALIDID), _actuator(actuator), _enableIntensity(enableIntensity), _enableTimeMillis(enableTimeMillis)
{ ; }

ActuatorTimedEnableTask::~ActuatorTimedEnableTask()
{ ; }

void ActuatorTimedEnableTask::exec()
{
    if (_actuator->enableActuator(_enableIntensity)) {
        delay(_enableTimeMillis);
        _actuator->disableActuator();

        disableRepeatingTask(taskId);
    } else {
        enableRepeatingTask(taskId);
    }
}

taskid_t scheduleActuatorTimedEnableOnce(shared_ptr<HydroponicsActuator> actuator, float enableIntensity, time_t enableTimeMillis)
{
    ActuatorTimedEnableTask *enableTask = actuator ? new ActuatorTimedEnableTask(actuator, enableIntensity, enableTimeMillis) : nullptr;
    HYDRUINO_SOFT_ASSERT(!actuator || enableTask, F("Failure allocating actuator timed enable task"));
    taskid_t retVal = enableTask ? taskManager.scheduleOnce(0, enableTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (enableTask ? (enableTask->taskId = retVal) : retVal);
}

taskid_t scheduleActuatorTimedEnableOnce(shared_ptr<HydroponicsActuator> actuator, time_t enableTimeMillis)
{
    ActuatorTimedEnableTask *enableTask = actuator ? new ActuatorTimedEnableTask(actuator, 1.0f, enableTimeMillis) : nullptr;
    HYDRUINO_SOFT_ASSERT(!actuator || enableTask, F("Failure allocating actuator timed enable task"));
    taskid_t retVal = enableTask ? taskManager.scheduleOnce(0, enableTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (enableTask ? (enableTask->taskId = retVal) : retVal);
}

void enableRepeatingTask(taskid_t taskId, time_t intervalMillis)
{
    auto task = taskId != TASKMGR_INVALIDID ? taskManager.getTask(taskId) : nullptr;
    if (task && !task->isRepeating()) {
        auto next = task->getNext();
        task->handleScheduling(intervalMillis, TIME_MILLIS, true);
        task->setNext(next);
    }
}

void disableRepeatingTask(taskid_t taskId, time_t intervalMillis)
{
    auto task = taskId != TASKMGR_INVALIDID ? taskManager.getTask(taskId) : nullptr;
    if (task && task->isRepeating()) {
        auto next = task->getNext();
        task->handleScheduling(intervalMillis, TIME_MILLIS, false);
        task->setNext(next);
    }
}


Hydroponics *getHydroponicsInstance()
{
    return Hydroponics::getActiveInstance();
}

HydroponicsScheduler *getSchedulerInstance()
{
    auto hydroponics = Hydroponics::getActiveInstance();
    return hydroponics ? &(hydroponics->_scheduler) : nullptr;
}

DateTime getCurrentTime()
{
    auto hydroponics = Hydroponics::getActiveInstance();
    return DateTime((uint32_t)(now() + ((hydroponics ? hydroponics->getTimeZoneOffset() : 0L) * SECS_PER_HOUR)));
}

time_t getCurrentDayStartTime()
{
    auto hydroponics = Hydroponics::getActiveInstance();
    long timeZoneSecs = (hydroponics ? hydroponics->getTimeZoneOffset() : 0L) * SECS_PER_HOUR;
    DateTime currTime = DateTime((uint32_t)(now() + timeZoneSecs));
    return DateTime(currTime.year(), currTime.month(), currTime.day()).unixtime() + timeZoneSecs;
}

Hydroponics_KeyType stringHash(String string)
{
    Hydroponics_KeyType hash = 5381;
    for(int index = 0; index < string.length(); ++index) {
        hash = ((hash << 5) + hash) + (Hydroponics_KeyType)string[index]; // Good 'ol DJB2
    }
    return hash != (Hydroponics_KeyType)-1 ? hash : 5381;
}

String stringFromChars(const char *charsIn, size_t length)
{
    if (!charsIn || !length) { return String(F("null")); }
    String retVal = "";
    for (size_t index = 0; index < length && charsIn[index] != '\0'; ++index) {
        retVal.concat(charsIn[index]);
    }
    return retVal.length() ? retVal : String(F("null"));
}

template<>
String commaStringFromArray<float>(const float *arrayIn, size_t length)
{
    if (!arrayIn || !length) { return String(F("null")); }
    String retVal = "";
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
    return retVal.length() ? retVal : String(F("null"));
}

template<>
String commaStringFromArray<double>(const double *arrayIn, size_t length)
{
    if (!arrayIn || !length) { return String(F("null")); }
    String retVal = "";
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
    return retVal.length() ? retVal : String(F("null"));
}

template<>
void commaStringToArray<float>(String stringIn, float *arrayOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(F("null"))) { return; }
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
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(F("null"))) { return; }
    int lastSepPos = -1;
    for (size_t index = 0; index < length; ++index) {
        int nextSepPos = stringIn.indexOf(',', lastSepPos+1);
        if (nextSepPos == -1) { nextSepPos = stringIn.length(); }
        String subString = stringIn.substring(lastSepPos+1, nextSepPos);
        if (nextSepPos < stringIn.length()) { lastSepPos = nextSepPos; }

        arrayOut[index] = subString.toDouble();
    }
}

String hexStringFromBytes(const byte *bytesIn, size_t length)
{
    if (!bytesIn || !length) { return String(F("null")); }
    String retVal = "";
    for (size_t index = 0; index < length; ++index) {
        String valStr = String(bytesIn[index], 16);
        if (valStr.length() == 1) { valStr = String('0') + valStr; }
        retVal += valStr;
    }
    return retVal.length() ? retVal : String(F("null"));
}

void hexStringToBytes(String stringIn, byte *bytesOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(F("null"))) { return; }
    for (size_t index = 0; index < length; ++index) {
        String valStr = stringIn.substring(index*2,(index+1)*2);
        if (valStr.length() == 2) { bytesOut[index] = strtoul(valStr.c_str(), nullptr, 16); }
        else { bytesOut[index] = 0; }
    }
}

void hexStringToBytes(JsonVariantConst &variantIn, byte *bytesOut, size_t length)
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
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
    char top;
    #ifdef __arm__
        return &top - reinterpret_cast<char*>(sbrk(0));
    #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
        return &top - __brkval;
    #else  // __arm__
        return __brkval ? &top - __brkval : &top - __malloc_heap_start;
    #endif  // __arm__
    return -1;
}

#ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT

void logMessage(String message, bool flushAfter)
{
    if (Serial) { Serial.println(message); }

    auto hydroponics = getHydroponicsInstance();
    if (hydroponics) {
        hydroponics->forwardLogMessage(message, flushAfter);
    }

    if (flushAfter) {
        if (Serial) { Serial.flush(); }
        yield();
    }
}

static String fileFromFullPath(String fullPath) {
    int index = fullPath.lastIndexOf(HYDRUINO_BLDPATH_SEPARATOR);
    return index != -1 ? fullPath.substring(index+1) : fullPath;
}

void softAssert(bool cond, String msg, const char *file, const char *func, int line)
{
    if (!cond) {
        msg = String(F("Assertion Failure: ")) + fileFromFullPath(String(file)) + String(F(":")) + String(line) + String(F(" in ")) + String(func) + String(F(": ")) + msg;    
        logMessage(msg);
        return;
    }
}

void hardAssert(bool cond, String msg, const char *file, const char *func, int line)
{
    if (!cond) {
        msg = String(F("Assertion Failure (HARD): ")) + fileFromFullPath(String(file)) + String(F(":")) + String(line) + String(F(" in ")) + String(func) + String(F(": ")) + msg;
        logMessage(msg, true);
        auto hydroponics = getHydroponicsInstance();
        if (hydroponics) { hydroponics->suspend(); }
        delay(1000);
        abort();
    }
}

#endif // /ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT

bool tryConvertStdUnits(float valueIn, Hydroponics_UnitsType unitsIn, float *valueOut, Hydroponics_UnitsType unitsOut)
{
    if (!valueOut || unitsOut == Hydroponics_UnitsType_Undefined || unitsIn == unitsOut) return false;

    switch (unitsIn) {
        case Hydroponics_UnitsType_Temperature_Celsius:
            switch(unitsOut) {
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
            switch(unitsOut) {
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
            switch(unitsOut) {
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

        case Hydroponics_UnitsType_Distance_Meters:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Distance_Feet:
                    *valueOut = valueIn * 3.28084f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Distance_Feet:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Distance_Meters:
                    *valueOut = valueIn * 0.3048;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Weight_Kilogram:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Weight_Pounds:
                    *valueOut = valueIn * 2.20462f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Weight_Pounds:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Weight_Kilogram:
                    *valueOut = valueIn * 0.453592f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_LiquidVolume_Liters:
            switch(unitsOut) {
                case Hydroponics_UnitsType_LiquidVolume_Gallons:
                    *valueOut = valueIn * 0.264172f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_LiquidVolume_Gallons:
            switch(unitsOut) {
                case Hydroponics_UnitsType_LiquidVolume_Liters:
                    *valueOut = valueIn * 3.78541f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_LiquidFlow_LitersPerMin:
            switch(unitsOut) {
                case Hydroponics_UnitsType_LiquidFlow_GallonsPerMin:
                    *valueOut = valueIn * 0.264172f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_LiquidFlow_GallonsPerMin:
            switch (unitsOut) {
                case Hydroponics_UnitsType_LiquidFlow_LitersPerMin:
                    *valueOut = valueIn * 3.78541f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_LiquidDilution_MilliLiterPerLiter:
            switch(unitsOut) {
                case Hydroponics_UnitsType_LiquidDilution_MilliLiterPerGallon:
                    *valueOut = valueIn * 3.78541f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_LiquidDilution_MilliLiterPerGallon:
            switch(unitsOut) {
                case Hydroponics_UnitsType_LiquidDilution_MilliLiterPerLiter:
                    *valueOut = valueIn * 0.264172f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_pHScale_0_14:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Raw_0_1:
                    *valueOut = valueIn / 14.0f; // FIXME no idea if this is correct
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Concentration_EC:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Raw_0_1:
                    *valueOut = valueIn / 5.0f;
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

        case Hydroponics_UnitsType_Concentration_PPM500:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Raw_0_1:
                    *valueOut = valueIn / (5.0f * 500.0f);
                    return true;

                case Hydroponics_UnitsType_Concentration_EC:
                    *valueOut = valueIn / 500.0f;
                    return true;

                case Hydroponics_UnitsType_Concentration_PPM640:
                case Hydroponics_UnitsType_Concentration_PPM700:
                    if (tryConvertStdUnits(valueIn, unitsIn, valueOut, Hydroponics_UnitsType_Concentration_EC)) {
                        return tryConvertStdUnits(*valueOut, Hydroponics_UnitsType_Concentration_EC, valueOut, unitsOut);
                    }
                    return false;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Concentration_PPM640:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Raw_0_1:
                    *valueOut = valueIn / (5.0f * 640.0f);
                    return true;

                case Hydroponics_UnitsType_Concentration_EC:
                    *valueOut = valueIn / 640.0f;
                    return true;

                case Hydroponics_UnitsType_Concentration_PPM500:
                case Hydroponics_UnitsType_Concentration_PPM700:
                    if (tryConvertStdUnits(valueIn, unitsIn, valueOut, Hydroponics_UnitsType_Concentration_EC)) {
                        return tryConvertStdUnits(*valueOut, Hydroponics_UnitsType_Concentration_EC, valueOut, unitsOut);
                    }
                    return false;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Concentration_PPM700:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Raw_0_1:
                    *valueOut = valueIn / (5.0f * 700.0f);
                    return true;

                case Hydroponics_UnitsType_Concentration_EC:
                    *valueOut = valueIn / 700.0f;
                    return true;

                case Hydroponics_UnitsType_Concentration_PPM500:
                case Hydroponics_UnitsType_Concentration_PPM700:
                    if (tryConvertStdUnits(valueIn, unitsIn, valueOut, Hydroponics_UnitsType_Concentration_EC)) {
                        return tryConvertStdUnits(*valueOut, Hydroponics_UnitsType_Concentration_EC, valueOut, unitsOut);
                    }
                    return false;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Percentile_0_100:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Raw_0_1:
                    *valueOut = valueIn / 100.0f;
                    return true;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Raw_0_1:
            switch(unitsOut) {
                case Hydroponics_UnitsType_pHScale_0_14:
                    *valueOut = valueIn * 14.0f;
                    return true;

                case Hydroponics_UnitsType_Concentration_EC:
                    *valueOut = valueIn * 5.0f; // FIXME: No idea if this is right
                    return true;

                case Hydroponics_UnitsType_Concentration_PPM500:
                    *valueOut = valueIn * (5.0f * 500.0f);
                    return true;

                case Hydroponics_UnitsType_Concentration_PPM640:
                    *valueOut = valueIn * (5.0f * 640.0f);
                    return true;

                case Hydroponics_UnitsType_Concentration_PPM700:
                    *valueOut = valueIn * (5.0f * 700.0f);
                    return true;

                case Hydroponics_UnitsType_Percentile_0_100:
                    *valueOut = valueIn * 100.0f;
                    return true;

                default:
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

bool convertStdUnits(float *valueInOut, Hydroponics_UnitsType *unitsInOut, Hydroponics_UnitsType unitsOut, int roundToDecPlaces)
{
    if (tryConvertStdUnits(*valueInOut, *unitsInOut, valueInOut, unitsOut)) {
        *unitsInOut = unitsOut;
        *valueInOut = roundToDecimalPlaces(*valueInOut, roundToDecPlaces);
        return true;
    }
    return false;
}

Hydroponics_UnitsType defaultTemperatureUnits(Hydroponics_MeasurementMode measureMode)
{
    if (measureMode == Hydroponics_MeasurementMode_Undefined) {
        auto hydroponics = getHydroponicsInstance();
        measureMode = (hydroponics ? hydroponics->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
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
        auto hydroponics = getHydroponicsInstance();
        measureMode = (hydroponics ? hydroponics->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
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
        auto hydroponics = getHydroponicsInstance();
        measureMode = (hydroponics ? hydroponics->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
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

Hydroponics_UnitsType defaultWaterVolumeUnits(Hydroponics_MeasurementMode measureMode)
{
    if (measureMode == Hydroponics_MeasurementMode_Undefined) {
        auto hydroponics = getHydroponicsInstance();
        measureMode = (hydroponics ? hydroponics->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
    }

    switch (measureMode) {
        case Hydroponics_MeasurementMode_Imperial:
            return Hydroponics_UnitsType_LiquidVolume_Gallons;
        case Hydroponics_MeasurementMode_Metric:
        case Hydroponics_MeasurementMode_Scientific:
            return Hydroponics_UnitsType_LiquidVolume_Liters;
        default:
            return Hydroponics_UnitsType_Undefined;
    }
}

Hydroponics_UnitsType defaultLiquidFlowUnits(Hydroponics_MeasurementMode measureMode)
{
    if (measureMode == Hydroponics_MeasurementMode_Undefined) {
        auto hydroponics = getHydroponicsInstance();
        measureMode = (hydroponics ? hydroponics->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
    }

    switch (measureMode) {
        case Hydroponics_MeasurementMode_Imperial:
            return Hydroponics_UnitsType_LiquidFlow_GallonsPerMin;
        case Hydroponics_MeasurementMode_Metric:
        case Hydroponics_MeasurementMode_Scientific:
            return Hydroponics_UnitsType_LiquidFlow_LitersPerMin;
        default:
            return Hydroponics_UnitsType_Undefined;
    }
}

Hydroponics_UnitsType defaultLiquidDilutionUnits(Hydroponics_MeasurementMode measureMode)
{
    if (measureMode == Hydroponics_MeasurementMode_Undefined) {
        auto hydroponics = getHydroponicsInstance();
        measureMode = (hydroponics ? hydroponics->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
    }

    switch (measureMode) {
        case Hydroponics_MeasurementMode_Imperial:
            return Hydroponics_UnitsType_LiquidDilution_MilliLiterPerGallon;
        case Hydroponics_MeasurementMode_Metric:
        case Hydroponics_MeasurementMode_Scientific:
            return Hydroponics_UnitsType_LiquidDilution_MilliLiterPerLiter;
        default:
            return Hydroponics_UnitsType_Undefined;
    }
}

Hydroponics_UnitsType defaultConcentrationUnits(Hydroponics_MeasurementMode measureMode)
{
    if (measureMode == Hydroponics_MeasurementMode_Undefined) {
        auto hydroponics = getHydroponicsInstance();
        measureMode = (hydroponics ? hydroponics->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
    }

    switch (measureMode) {
        case Hydroponics_MeasurementMode_Imperial:
        case Hydroponics_MeasurementMode_Metric:
        case Hydroponics_MeasurementMode_Scientific:
            return Hydroponics_UnitsType_Concentration_EC;
        default:
            return Hydroponics_UnitsType_Undefined;
    }
}

int defaultDecimalPlacesRounding(Hydroponics_MeasurementMode measureMode)
{
    if (measureMode == Hydroponics_MeasurementMode_Undefined) {
        auto hydroponics = getHydroponicsInstance();
        measureMode = (hydroponics ? hydroponics->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
    }

    switch (measureMode) {
        case Hydroponics_MeasurementMode_Imperial:
        case Hydroponics_MeasurementMode_Metric:
            return 1;
        case Hydroponics_MeasurementMode_Scientific:
            return 2;
        default:
            return -1;
    }
}

bool checkPinIsAnalogInput(byte pin)
{
    #if !defined(NUM_ANALOG_INPUTS) || NUM_ANALOG_INPUTS == 0
        return false;
    #else
        switch (pin) {
            #if NUM_ANALOG_INPUTS > 0
                case (int)A0:
            #endif
            #if NUM_ANALOG_INPUTS > 1
                case (int)A1:
            #endif
            #if NUM_ANALOG_INPUTS > 2
                case (int)A2:
            #endif
            #if NUM_ANALOG_INPUTS > 3
                case (int)A3:
            #endif
            #if NUM_ANALOG_INPUTS > 4
                case (int)A4:
            #endif
            #if NUM_ANALOG_INPUTS > 5
                case (int)A5:
            #endif
            #if NUM_ANALOG_INPUTS > 6
                case (int)A6:
            #endif
            #if NUM_ANALOG_INPUTS > 7
                case (int)A7:
            #endif
            #if NUM_ANALOG_INPUTS > 8
                case (int)A8:
            #endif
            #if NUM_ANALOG_INPUTS > 9
                case (int)A9:
            #endif
            #if NUM_ANALOG_INPUTS > 10
                case (int)A10:
            #endif
            #if NUM_ANALOG_INPUTS > 11
                case (int)A11:
            #endif
            #if NUM_ANALOG_INPUTS > 12
                case (int)A12:
            #endif
            #if NUM_ANALOG_INPUTS > 13
                case (int)A13:
            #endif
            #if NUM_ANALOG_INPUTS > 14
                case (int)A14:
            #endif
            #if NUM_ANALOG_INPUTS > 15
                case (int)A15:
            #endif
                return true;

            default:
                return false;
        }
    #endif
}

bool checkPinIsAnalogOutput(byte pin)
{
    #if !defined(NUM_ANALOG_OUTPUTS) || NUM_ANALOG_OUTPUTS == 0
        return false;
    #else
        switch (pin) {
            #if NUM_ANALOG_OUTPUTS > 0
                case (int)DAC0:
            #endif
            #if NUM_ANALOG_OUTPUTS > 1
                case (int)DAC1:
            #endif
            #if NUM_ANALOG_OUTPUTS > 2
                case (int)DAC2:
            #endif
            #if NUM_ANALOG_OUTPUTS > 3
                case (int)DAC3:
            #endif
            #if NUM_ANALOG_OUTPUTS > 4
                case (int)DAC4:
            #endif
            #if NUM_ANALOG_OUTPUTS > 5
                case (int)DAC5:
            #endif
            #if NUM_ANALOG_OUTPUTS > 6
                case (int)DAC6:
            #endif
            #if NUM_ANALOG_OUTPUTS > 7
                case (int)DAC7:
            #endif
                return true;

            default:
                return false;
        }
    #endif
}

bool checkPinIsDigital(byte pin)
{
    return !checkPinIsAnalogInput(pin) && !checkPinIsAnalogOutput(pin);
}

bool checkPinIsPWMOutput(byte pin)
{
    return digitalPinHasPWM(pin);
}

bool checkPinCanInterrupt(byte pin)
{
    return isValidPin(digitalPinToInterrupt(pin));
}

void setRandomSeed()
{
    {   auto time = rtcNow();
        if (time > 0) { randomSeed(time); return; }
    }
    #if NUM_ANALOG_INPUTS >= 16
        randomSeed(((analogRead(A15) & 0xFF) << 24) | ((analogRead(A15) & 0xFF) << 16) | ((analogRead(A15) & 0xFF) << 8) | (analogRead(A15) & 0xFF)); return;
    #elif NUM_ANALOG_INPUTS >= 15
        randomSeed(((analogRead(A14) & 0xFF) << 24) | ((analogRead(A14) & 0xFF) << 16) | ((analogRead(A14) & 0xFF) << 8) | (analogRead(A14) & 0xFF)); return;
    #elif NUM_ANALOG_INPUTS >= 14
        randomSeed(((analogRead(A13) & 0xFF) << 24) | ((analogRead(A13) & 0xFF) << 16) | ((analogRead(A13) & 0xFF) << 8) | (analogRead(A13) & 0xFF)); return;
    #elif NUM_ANALOG_INPUTS >= 13
        randomSeed(((analogRead(A12) & 0xFF) << 24) | ((analogRead(A12) & 0xFF) << 16) | ((analogRead(A12) & 0xFF) << 8) | (analogRead(A12) & 0xFF)); return;
    #elif NUM_ANALOG_INPUTS >= 12
        randomSeed(((analogRead(A11) & 0xFF) << 24) | ((analogRead(A11) & 0xFF) << 16) | ((analogRead(A11) & 0xFF) << 8) | (analogRead(A11) & 0xFF)); return;
    #elif NUM_ANALOG_INPUTS >= 11
        randomSeed(((analogRead(A10) & 0xFF) << 24) | ((analogRead(A10) & 0xFF) << 16) | ((analogRead(A10) & 0xFF) << 8) | (analogRead(A10) & 0xFF)); return;
    #elif NUM_ANALOG_INPUTS >= 10
        randomSeed(((analogRead(A9) & 0xFF) << 24) | ((analogRead(A9) & 0xFF) << 16) | ((analogRead(A9) & 0xFF) << 8) | (analogRead(A9) & 0xFF)); return;
    #elif NUM_ANALOG_INPUTS >= 9
        randomSeed(((analogRead(A8) & 0xFF) << 24) | ((analogRead(A8) & 0xFF) << 16) | ((analogRead(A8) & 0xFF) << 8) | (analogRead(A8) & 0xFF)); return;
    #elif NUM_ANALOG_INPUTS >= 8
        randomSeed(((analogRead(A7) & 0xFF) << 24) | ((analogRead(A7) & 0xFF) << 16) | ((analogRead(A7) & 0xFF) << 8) | (analogRead(A7) & 0xFF)); return;
    #elif NUM_ANALOG_INPUTS >= 7
        randomSeed(((analogRead(A6) & 0xFF) << 24) | ((analogRead(A6) & 0xFF) << 16) | ((analogRead(A6) & 0xFF) << 8) | (analogRead(A6) & 0xFF)); return;
    #elif NUM_ANALOG_INPUTS >= 6
        randomSeed(((analogRead(A5) & 0xFF) << 24) | ((analogRead(A5) & 0xFF) << 16) | ((analogRead(A5) & 0xFF) << 8) | (analogRead(A5) & 0xFF)); return;
    #elif NUM_ANALOG_INPUTS >= 5
        randomSeed(((analogRead(A4) & 0xFF) << 24) | ((analogRead(A4) & 0xFF) << 16) | ((analogRead(A4) & 0xFF) << 8) | (analogRead(A4) & 0xFF)); return;
    #elif NUM_ANALOG_INPUTS >= 4
        randomSeed(((analogRead(A3) & 0xFF) << 24) | ((analogRead(A3) & 0xFF) << 16) | ((analogRead(A3) & 0xFF) << 8) | (analogRead(A3) & 0xFF)); return;
    #elif NUM_ANALOG_INPUTS >= 3
        randomSeed(((analogRead(A2) & 0xFF) << 24) | ((analogRead(A2) & 0xFF) << 16) | ((analogRead(A2) & 0xFF) << 8) | (analogRead(A2) & 0xFF)); return;
    #elif NUM_ANALOG_INPUTS >= 2
        randomSeed(((analogRead(A1) & 0xFF) << 24) | ((analogRead(A1) & 0xFF) << 16) | ((analogRead(A1) & 0xFF) << 8) | (analogRead(A1) & 0xFF)); return;
    #elif NUM_ANALOG_INPUTS >= 1
        randomSeed(((analogRead(A0) & 0xFF) << 24) | ((analogRead(A0) & 0xFF) << 16) | ((analogRead(A0) & 0xFF) << 8) | (analogRead(A0) & 0xFF)); return;
    #endif
    randomSeed(micros());
}

String actuatorTypeToString(Hydroponics_ActuatorType actuatorType, bool excludeSpecial)
{
    switch (actuatorType) {
        case Hydroponics_ActuatorType_GrowLights:
            return F("GrowLights");
        case Hydroponics_ActuatorType_WaterPump:
            return F("WaterPump");
        case Hydroponics_ActuatorType_PeristalticPump:
            return F("PeristalticPump");
        case Hydroponics_ActuatorType_WaterHeater:
            return F("WaterHeater");
        case Hydroponics_ActuatorType_WaterAerator:
            return F("WaterAerator");
        case Hydroponics_ActuatorType_FanExhaust:
            return F("FanExhaust");
        case Hydroponics_ActuatorType_Count:
            return !excludeSpecial ? F("ActuatorCount") : F("");
        case Hydroponics_ActuatorType_Undefined:
            break;
    }
    return !excludeSpecial ? F("ActuatorUndefined") : F("");
}

String sensorTypeToString(Hydroponics_SensorType sensorType, bool excludeSpecial)
{
    switch (sensorType) {
        case Hydroponics_SensorType_AirTempHumidity:
            return F("AirTempHumid");
        case Hydroponics_SensorType_AirCarbonDioxide:
            return F("AirCO2");
        case Hydroponics_SensorType_PotentialHydrogen:
            return F("WaterPH");
        case Hydroponics_SensorType_TotalDissolvedSolids:
            return F("WaterTDS");
        case Hydroponics_SensorType_WaterTemperature:
            return F("WaterTemp");
        case Hydroponics_SensorType_SoilMoisture:
            return F("SoilMoisture");
        case Hydroponics_SensorType_WaterPumpFlowSensor:
            return F("PumpFlow");
        case Hydroponics_SensorType_WaterLevelIndicator:
            return F("LevelIndicator");
        case Hydroponics_SensorType_WaterHeightMeter:
            return F("WaterHeight");
        case Hydroponics_SensorType_PowerUsageMeter:
            return F("PowerUsage");
        case Hydroponics_SensorType_Count:
            return !excludeSpecial ? F("SensorCount") : F("");
        case Hydroponics_SensorType_Undefined:
            break;
    }
    return !excludeSpecial ? F("SensorUndefined") : F("");
}

String cropTypeToString(Hydroponics_CropType cropType, bool excludeSpecial)
{
    switch (cropType) {
        case Hydroponics_CropType_AloeVera:
            return F("AloeVera");
        case Hydroponics_CropType_Anise:
            return F("Anise");
        case Hydroponics_CropType_Artichoke:
            return F("Artichoke");
        case Hydroponics_CropType_Arugula:
            return F("Arugula");
        case Hydroponics_CropType_Asparagus:
            return F("Asparagus");
        case Hydroponics_CropType_Basil:
            return F("Basil");
        case Hydroponics_CropType_Bean:
            return F("Bean");
        case Hydroponics_CropType_BeanBroad:
            return F("BeanBroad");
        case Hydroponics_CropType_Beetroot:
            return F("Beetroot");
        case Hydroponics_CropType_BlackCurrant:
            return F("BlackCurrant");
        case Hydroponics_CropType_Blueberry:
            return F("Blueberry");
        case Hydroponics_CropType_BokChoi:
            return F("BokChoi");
        case Hydroponics_CropType_Broccoli:
            return F("Broccoli");
        case Hydroponics_CropType_BrusselsSprout:
            return F("BrusselsSprout");
        case Hydroponics_CropType_Cabbage:
            return F("Cabbage");
        case Hydroponics_CropType_Cannabis:
            return F("Cannabis");
        case Hydroponics_CropType_Capsicum:
            return F("Capsicum");
        case Hydroponics_CropType_Carrots:
            return F("Carrots");
        case Hydroponics_CropType_Catnip:
            return F("Catnip");
        case Hydroponics_CropType_Cauliflower:
            return F("Cauliflower");
        case Hydroponics_CropType_Celery:
            return F("Celery");
        case Hydroponics_CropType_Chamomile:
            return F("Chamomile");
        case Hydroponics_CropType_Chicory:
            return F("Chicory");
        case Hydroponics_CropType_Chives:
            return F("Chives");
        case Hydroponics_CropType_Cilantro:
            return F("Cilantro");
        case Hydroponics_CropType_Coriander:
            return F("Coriander");
        case Hydroponics_CropType_CornSweet:
            return F("CornSweet");
        case Hydroponics_CropType_Cucumber:
            return F("Cucumber");
        case Hydroponics_CropType_Dill:
            return F("Dill");
        case Hydroponics_CropType_Eggplant:
            return F("Eggplant");
        case Hydroponics_CropType_Endive:
            return F("Endive");
        case Hydroponics_CropType_Fennel:
            return F("Fennel");
        case Hydroponics_CropType_Fodder:
            return F("Fodder");
        case Hydroponics_CropType_Flowers:
            return F("Flowers");
        case Hydroponics_CropType_Garlic:
            return F("Garlic");
        case Hydroponics_CropType_Ginger:
            return F("Ginger");
        case Hydroponics_CropType_Kale:
            return F("Kale");
        case Hydroponics_CropType_Lavender:
            return F("Lavender");
        case Hydroponics_CropType_Leek:
            return F("Leek");
        case Hydroponics_CropType_LemonBalm:
            return F("LemonBalm");
        case Hydroponics_CropType_Lettuce:
            return F("Lettuce");
        case Hydroponics_CropType_Marrow:
            return F("Marrow");
        case Hydroponics_CropType_Melon:
            return F("Melon");
        case Hydroponics_CropType_Mint:
            return F("Mint");
        case Hydroponics_CropType_MustardCress:
            return F("MustardCress");
        case Hydroponics_CropType_Okra:
            return F("Okra");
        case Hydroponics_CropType_Onions:
            return F("Onions");
        case Hydroponics_CropType_Oregano:
            return F("Oregano");
        case Hydroponics_CropType_PakChoi:
            return F("PakChoi");
        case Hydroponics_CropType_Parsley:
            return F("Parsley");
        case Hydroponics_CropType_Parsnip:
            return F("Parsnip");
        case Hydroponics_CropType_Pea:
            return F("Pea");
        case Hydroponics_CropType_PeaSugar:
            return F("PeaSugar");
        case Hydroponics_CropType_Pepino:
            return F("Pepino");
        case Hydroponics_CropType_PeppersBell:
            return F("PeppersBell");
        case Hydroponics_CropType_PeppersHot:
            return F("PeppersHot");
        case Hydroponics_CropType_Potato:
            return F("Potato");
        case Hydroponics_CropType_PotatoSweet:
            return F("PotatoSweet");
        case Hydroponics_CropType_Pumpkin:
            return F("Pumpkin");
        case Hydroponics_CropType_Radish:
            return F("Radish");
        case Hydroponics_CropType_Rhubarb:
            return F("Rhubarb");
        case Hydroponics_CropType_Rosemary:
            return F("Rosemary");
        case Hydroponics_CropType_Sage:
            return F("Sage");
        case Hydroponics_CropType_Silverbeet:
            return F("Silverbeet");
        case Hydroponics_CropType_Spinach:
            return F("Spinach");
        case Hydroponics_CropType_Squash:
            return F("Squash");
        case Hydroponics_CropType_Sunflower:
            return F("Sunflower");
        case Hydroponics_CropType_Strawberries:
            return F("Strawberries");
        case Hydroponics_CropType_SwissChard:
            return F("SwissChard");
        case Hydroponics_CropType_Taro:
            return F("Taro");
        case Hydroponics_CropType_Tarragon:
            return F("Tarragon");
        case Hydroponics_CropType_Thyme:
            return F("Thyme");
        case Hydroponics_CropType_Tomato:
            return F("Tomato");
        case Hydroponics_CropType_Turnip:
            return F("Turnip");
        case Hydroponics_CropType_Watercress:
            return F("Watercress");
        case Hydroponics_CropType_Watermelon:
            return F("Watermelon");
        case Hydroponics_CropType_Zucchini:
            return F("Zucchini");
        case Hydroponics_CropType_CustomCrop1:
            return F("CustomCrop1");
        case Hydroponics_CropType_CustomCrop2:
            return F("CustomCrop2");
        case Hydroponics_CropType_CustomCrop3:
            return F("CustomCrop3");
        case Hydroponics_CropType_CustomCrop4:
            return F("CustomCrop4");
        case Hydroponics_CropType_CustomCrop5:
            return F("CustomCrop5");
        case Hydroponics_CropType_CustomCrop6:
            return F("CustomCrop6");
        case Hydroponics_CropType_CustomCrop7:
            return F("CustomCrop7");
        case Hydroponics_CropType_CustomCrop8:
            return F("CustomCrop8");
        case Hydroponics_CropType_Count:
            return !excludeSpecial ? F("CropCount") : F("");
        case Hydroponics_CropType_Undefined:
            break;
    }
    return !excludeSpecial ? F("CropUndefined") : F("");
}

String reservoirTypeToString(Hydroponics_ReservoirType reservoirType, bool excludeSpecial)
{
    switch (reservoirType) {
        case Hydroponics_ReservoirType_FeedWater:
            return F("FeedWater");
        case Hydroponics_ReservoirType_DrainageWater:
            return F("DrainageWater");
        case Hydroponics_ReservoirType_NutrientPremix:
            return F("NutrientPremix");
        case Hydroponics_ReservoirType_FreshWater:
            return F("FreshWater");
        case Hydroponics_ReservoirType_PhUpSolution:
            return F("phUpSolution");
        case Hydroponics_ReservoirType_PhDownSolution:
            return F("pHDownSolution");
        case Hydroponics_ReservoirType_CustomAdditive1:
            return F("CustomAdditive1");
        case Hydroponics_ReservoirType_CustomAdditive2:
            return F("CustomAdditive2");
        case Hydroponics_ReservoirType_CustomAdditive3:
            return F("CustomAdditive3");
        case Hydroponics_ReservoirType_CustomAdditive4:
            return F("CustomAdditive4");
        case Hydroponics_ReservoirType_CustomAdditive5:
            return F("CustomAdditive5");
        case Hydroponics_ReservoirType_CustomAdditive6:
            return F("CustomAdditive6");
        case Hydroponics_ReservoirType_CustomAdditive7:
            return F("CustomAdditive7");
        case Hydroponics_ReservoirType_CustomAdditive8:
            return F("CustomAdditive8");
        case Hydroponics_ReservoirType_CustomAdditive9:
            return F("CustomAdditive9");
        case Hydroponics_ReservoirType_CustomAdditive10:
            return F("CustomAdditive10");
        case Hydroponics_ReservoirType_CustomAdditive11:
            return F("CustomAdditive11");
        case Hydroponics_ReservoirType_CustomAdditive12:
            return F("CustomAdditive12");
        case Hydroponics_ReservoirType_CustomAdditive13:
            return F("CustomAdditive13");
        case Hydroponics_ReservoirType_CustomAdditive14:
            return F("CustomAdditive14");
        case Hydroponics_ReservoirType_CustomAdditive15:
            return F("CustomAdditive15");
        case Hydroponics_ReservoirType_CustomAdditive16:
            return F("CustomAdditive16");
        case Hydroponics_ReservoirType_Count:
            return !excludeSpecial ? F("ReservoirCount") : F("");
        case Hydroponics_ReservoirType_Undefined:
            break;
    }
    return !excludeSpecial ? F("ReservoirUndefined") : F("");
}

String railTypeToString(Hydroponics_RailType railType, bool excludeSpecial)
{
    switch (railType) {
        case Hydroponics_RailType_AC110V:
            return F("AC110V");
        case Hydroponics_RailType_AC220V:
            return F("AC220V");
        case Hydroponics_RailType_DC5V:
            return F("DC5V");
        case Hydroponics_RailType_DC12V:
            return F("DC12V");
        case Hydroponics_ReservoirType_Count:
            return !excludeSpecial ? F("RailCount") : F("");
        case Hydroponics_ReservoirType_Undefined:
            break;
    }
    return !excludeSpecial ? F("RailUndefined") : F("");
}

String unitsTypeToSymbol(Hydroponics_UnitsType unitsType, bool excludeSpecial)
{
    switch (unitsType) {
        case Hydroponics_UnitsType_Temperature_Celsius:
            return F("°C");
        case Hydroponics_UnitsType_Temperature_Fahrenheit:
            return F("°F");
        case Hydroponics_UnitsType_Temperature_Kelvin:
            return F("°K");
        case Hydroponics_UnitsType_Distance_Meters:
            return F("M");
        case Hydroponics_UnitsType_Distance_Feet:
            return F("ft");
        case Hydroponics_UnitsType_Weight_Kilogram:
            return F("Kg");
        case Hydroponics_UnitsType_Weight_Pounds:
            return F("lbs");
        case Hydroponics_UnitsType_LiquidVolume_Liters:
            return F("L");
        case Hydroponics_UnitsType_LiquidVolume_Gallons:
            return F("gal");
        case Hydroponics_UnitsType_LiquidFlow_LitersPerMin:
            return F("L/min");
        case Hydroponics_UnitsType_LiquidFlow_GallonsPerMin:
            return F("gal/min");
        case Hydroponics_UnitsType_LiquidDilution_MilliLiterPerLiter:
            return F("mL/L");
        case Hydroponics_UnitsType_LiquidDilution_MilliLiterPerGallon:
            return F("mL/gal");
        case Hydroponics_UnitsType_Power_Wattage:
            return F("W");
        case Hydroponics_UnitsType_pHScale_0_14:
            return F("pH");
        case Hydroponics_UnitsType_Concentration_EC:
            return F("mS/cm"); // alt: EC, TDS
        case Hydroponics_UnitsType_Concentration_PPM500:
            return F("ppm(500)"); // alt: PPM
        case Hydroponics_UnitsType_Concentration_PPM640:
            return F("ppm(640)");
        case Hydroponics_UnitsType_Concentration_PPM700:
            return F("ppm(700)");
        case Hydroponics_UnitsType_Percentile_0_100:
            return F("%");
        case Hydroponics_UnitsType_Raw_0_1:
            return F("raw(01)"); // alt: raw
        case Hydroponics_UnitsType_Count:
            return !excludeSpecial ? F("qty") : F("");
        case Hydroponics_UnitsType_Undefined:
            break;
    }
    return !excludeSpecial ? F("undef") : F("");
}

String positionIndexToString(Hydroponics_PositionIndex positionIndex, bool excludeSpecial)
{
    if (positionIndex >= 0 && positionIndex < HYDRUINO_POS_MAXSIZE) {
        return String(positionIndex + HYDRUINO_POS_EXPORT_BEGFROM);
    } else if (!excludeSpecial) {
        if (positionIndex == HYDRUINO_POS_MAXSIZE) {
            return F("PositionCount");
        } else {
            return F("PositionUndefined");
        }
    }
    return String();
}

Hydroponics_ActuatorType actuatorTypeFromString(String actuatorTypeStr)
{
    for (int typeIndex = 0; typeIndex <= Hydroponics_ActuatorType_Count; ++typeIndex) {
        if (actuatorTypeStr == actuatorTypeToString((Hydroponics_ActuatorType)typeIndex)) {
            return (Hydroponics_ActuatorType)typeIndex;
        }
    }
    return Hydroponics_ActuatorType_Undefined;
}

Hydroponics_SensorType sensorTypeFromString(String sensorTypeStr)
{
    for (int typeIndex = 0; typeIndex <= Hydroponics_SensorType_Count; ++typeIndex) {
        if (sensorTypeStr == sensorTypeToString((Hydroponics_SensorType)typeIndex)) {
            return (Hydroponics_SensorType)typeIndex;
        }
    }
    return Hydroponics_SensorType_Undefined;
}

Hydroponics_CropType cropTypeFromString(String cropTypeStr)
{
    for (int typeIndex = 0; typeIndex <= Hydroponics_CropType_Count; ++typeIndex) {
        if (cropTypeStr == cropTypeToString((Hydroponics_CropType)typeIndex)) {
            return (Hydroponics_CropType)typeIndex;
        }
    }
    return Hydroponics_CropType_Undefined;
}

Hydroponics_ReservoirType reservoirTypeFromString(String reservoirTypeStr)
{
    for (int typeIndex = 0; typeIndex <= Hydroponics_ReservoirType_Count; ++typeIndex) {
        if (reservoirTypeStr == reservoirTypeToString((Hydroponics_ReservoirType)typeIndex)) {
            return (Hydroponics_ReservoirType)typeIndex;
        }
    }
    return Hydroponics_ReservoirType_Undefined;
}

Hydroponics_RailType railTypeFromString(String railTypeStr) {
    for (int typeIndex = 0; typeIndex <= Hydroponics_RailType_Count; ++typeIndex) {
        if (railTypeStr == railTypeToString((Hydroponics_RailType)typeIndex)) {
            return (Hydroponics_RailType)typeIndex;
        }
    }
    return Hydroponics_RailType_Undefined;
}

Hydroponics_UnitsType unitsTypeFromSymbol(String unitsSymbolStr)
{
    for (int typeIndex = 0; typeIndex <= Hydroponics_UnitsType_Count; ++typeIndex) {
        if (unitsSymbolStr == unitsTypeToSymbol((Hydroponics_UnitsType)typeIndex)) {
            return (Hydroponics_UnitsType)typeIndex;
        }
    }
    if (unitsSymbolStr.equalsIgnoreCase(F("EC")) || unitsSymbolStr.equalsIgnoreCase(F("TDS"))) { return Hydroponics_UnitsType_Concentration_EC; }
    if (unitsSymbolStr.equalsIgnoreCase(F("PPM"))) { return Hydroponics_UnitsType_Concentration_PPM500; }
    if (unitsSymbolStr.equalsIgnoreCase(F("J/s"))) { return Hydroponics_UnitsType_Power_Wattage; }
    if (unitsSymbolStr.equalsIgnoreCase(F("raw"))) { return Hydroponics_UnitsType_Raw_0_1; }
    return Hydroponics_UnitsType_Undefined;
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
