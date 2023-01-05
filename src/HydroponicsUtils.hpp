/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydroponics Utilities
*/

#ifndef HydroponicsUtils_HPP
#define HydroponicsUtils_HPP

#include "Hydroponics.h"

#ifndef HYDRUINO_DISABLE_MULTITASKING

template<typename ParameterType, int Slots>
taskid_t scheduleSignalFireOnce(SharedPtr<HydroponicsObjInterface> object, Signal<ParameterType,Slots> &signal, ParameterType fireParam)
{
    SignalFireTask<ParameterType,Slots> *fireTask = object ? new SignalFireTask<ParameterType,Slots>(object, signal, fireParam) : nullptr;
    HYDRUINO_SOFT_ASSERT(!object || fireTask, SFP(HStr_Err_AllocationFailure));
    taskid_t retVal = fireTask ? taskManager.scheduleOnce(0, fireTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (fireTask ? (fireTask->taskId = retVal) : retVal);
}

template<typename ParameterType, int Slots>
taskid_t scheduleSignalFireOnce(Signal<ParameterType,Slots> &signal, ParameterType fireParam)
{
    SignalFireTask<ParameterType,Slots> *fireTask = new SignalFireTask<ParameterType,Slots>(nullptr, signal, fireParam);
    HYDRUINO_SOFT_ASSERT(fireTask, SFP(HStr_Err_AllocationFailure));
    taskid_t retVal = fireTask ? taskManager.scheduleOnce(0, fireTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (fireTask ? (fireTask->taskId = retVal) : retVal);
}

template<class ObjectType, typename ParameterType>
taskid_t scheduleObjectMethodCallOnce(SharedPtr<ObjectType> object, void (ObjectType::*method)(ParameterType), ParameterType callParam)
{
    MethodSlotCallTask<ObjectType,ParameterType> *callTask = object ? new MethodSlotCallTask<ObjectType,ParameterType>(object, method, callParam) : nullptr;
    HYDRUINO_SOFT_ASSERT(!object || callTask, SFP(HStr_Err_AllocationFailure));
    taskid_t retVal = callTask ? taskManager.scheduleOnce(0, callTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (callTask ? (callTask->taskId = retVal) : retVal);
}

template<class ObjectType, typename ParameterType>
taskid_t scheduleObjectMethodCallOnce(ObjectType *object, void (ObjectType::*method)(ParameterType), ParameterType callParam)
{
    MethodSlotCallTask<ObjectType,ParameterType> *callTask = object ? new MethodSlotCallTask<ObjectType,ParameterType>(object, method, callParam) : nullptr;
    HYDRUINO_SOFT_ASSERT(!object || callTask, SFP(HStr_Err_AllocationFailure));
    taskid_t retVal = callTask ? taskManager.scheduleOnce(0, callTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (callTask ? (callTask->taskId = retVal) : retVal);
}

template<class ObjectType>
taskid_t scheduleObjectMethodCallWithTaskIdOnce(SharedPtr<ObjectType> object, void (ObjectType::*method)(taskid_t))
{
    MethodSlotCallTask<ObjectType,taskid_t> *callTask = object ? new MethodSlotCallTask<ObjectType,taskid_t>(object, method, (taskid_t)0) : nullptr;
    HYDRUINO_SOFT_ASSERT(!object || callTask, SFP(HStr_Err_AllocationFailure));
    taskid_t retVal = callTask ? taskManager.scheduleOnce(0, callTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (callTask ? (callTask->taskId = (callTask->_callParam = retVal)) : retVal);
}

template<class ObjectType>
taskid_t scheduleObjectMethodCallWithTaskIdOnce(ObjectType *object, void (ObjectType::*method)(taskid_t))
{
    MethodSlotCallTask<ObjectType,taskid_t> *callTask = object ? new MethodSlotCallTask<ObjectType,taskid_t>(object, method, (taskid_t)0) : nullptr;
    HYDRUINO_SOFT_ASSERT(!object || callTask, SFP(HStr_Err_AllocationFailure));
    taskid_t retVal = callTask ? taskManager.scheduleOnce(0, callTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (callTask ? (callTask->taskId = (callTask->_callParam = retVal)) : retVal);
}

#endif // /ifndef HYDRUINO_DISABLE_MULTITASKING


template<typename T>
String commaStringFromArray(const T *arrayIn, size_t length)
{
    if (!arrayIn || !length) { return String(SFP(HStr_null)); }
    String retVal; retVal.reserve(length << 1);
    for (size_t index = 0; index < length; ++index) {
        if (retVal.length()) { retVal.concat(','); }
        retVal += String(arrayIn[index]);
    }
    return retVal.length() ? retVal : String(SFP(HStr_null));
}

template<typename T>
void commaStringToArray(String stringIn, T *arrayOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(SFP(HStr_null))) { return; }
    int lastSepPos = -1;
    for (size_t index = 0; index < length; ++index) {
        int nextSepPos = stringIn.indexOf(',', lastSepPos+1);
        if (nextSepPos == -1) { nextSepPos = stringIn.length(); }
        String subString = stringIn.substring(lastSepPos+1, nextSepPos);
        if (nextSepPos < stringIn.length()) { lastSepPos = nextSepPos; }

        arrayOut[index] = static_cast<T>(subString.toInt());
    }
}

template<typename T>
void commaStringToArray(JsonVariantConst &variantIn, T *arrayOut, size_t length)
{
    if (variantIn.isNull() || variantIn.is<JsonObjectConst>() || variantIn.is<JsonArrayConst>()) { return; }
    commaStringToArray<T>(variantIn.as<String>(), arrayOut, length);
}

template<typename T>
bool arrayElementsEqual(const T *arrayIn, size_t length, T value)
{
    for (size_t index = 0; index < length; ++index) {
        if (!(arrayIn[index] == value)) {
            return false;
        }
    }
    return true;
}


template<size_t N = HYDRUINO_OBJ_LINKSFILTER_DEFSIZE>
Vector<HydroponicsObject *, N> linksFilterActuators(Pair<uint8_t, Pair<HydroponicsObject *, int8_t> *> links)
{
    Vector<HydroponicsObject *, N> retVal;

    for (int linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
        if (links.second[linksIndex].first->isActuatorType()) {
            retVal.push_back(links.second[linksIndex].first);
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKSFILTER_DEFSIZE>
Vector<HydroponicsObject *, N> linksFilterCrops(Pair<uint8_t, Pair<HydroponicsObject *, int8_t> *> links)
{
    Vector<HydroponicsObject *, N> retVal;

    for (int linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
        if (links.second[linksIndex].first->isCropType()) {
            retVal.push_back(links.second[linksIndex].first);
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKSFILTER_DEFSIZE>
Vector<HydroponicsObject *, N> linksFilterActuatorsByReservoirAndType(Pair<uint8_t, Pair<HydroponicsObject *, int8_t> *> links, HydroponicsReservoir *srcReservoir, Hydroponics_ActuatorType actuatorType)
{
    Vector<HydroponicsObject *, N> retVal;

    for (int linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
        if (links.second[linksIndex].first->isActuatorType()) {
            auto actuator = static_cast<HydroponicsActuator *>(links.second[linksIndex].first);

            if (actuator->getActuatorType() == actuatorType && actuator->getReservoir().get() == srcReservoir) {
                retVal.push_back(links.second[linksIndex].first);
            }
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKSFILTER_DEFSIZE>
Vector<HydroponicsObject *, N> linksFilterPumpActuatorsByInputReservoirAndOutputReservoirType(Pair<uint8_t, Pair<HydroponicsObject *, int8_t> *> links, HydroponicsReservoir *srcReservoir, Hydroponics_ReservoirType destReservoirType)
{
    Vector<HydroponicsObject *, N> retVal;

    for (int linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
        if (links.second[linksIndex].first->isActuatorType()) {
            auto actuator = static_cast<HydroponicsActuator *>(links.second[linksIndex].first);

            if (actuator->isRelayPumpClass() && static_cast<HydroponicsPumpRelayActuator *>(actuator)->getInputReservoir().get() == srcReservoir) {
                auto outputReservoir = static_cast<HydroponicsPumpRelayActuator *>(actuator)->getOutputReservoir().get();

                if (outputReservoir && outputReservoir->getReservoirType() == destReservoirType) {
                    retVal.push_back(links.second[linksIndex].first);
                }
            }
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKSFILTER_DEFSIZE>
Vector<HydroponicsObject *, N> linksFilterPumpActuatorsByOutputReservoirAndInputReservoirType(Pair<uint8_t, Pair<HydroponicsObject *, int8_t> *> links, HydroponicsReservoir *destReservoir, Hydroponics_ReservoirType srcReservoirType)
{
    Vector<HydroponicsObject *, N> retVal;

    for (int linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
        if (links.second[linksIndex].first->isActuatorType()) {
            auto actuator = static_cast<HydroponicsActuator *>(links.second[linksIndex].first);

            if (actuator->isRelayPumpClass() && static_cast<HydroponicsPumpRelayActuator *>(actuator)->getOutputReservoir().get() == destReservoir) {
                auto inputReservoir = static_cast<HydroponicsPumpRelayActuator *>(actuator)->getInputReservoir().get();

                if (inputReservoir && inputReservoir->getReservoirType() == srcReservoirType) {
                    retVal.push_back(links.second[linksIndex].first);
                }
            }
        }
    }

    return retVal;
}

template<size_t N>
void linksResolveActuatorsByType(Vector<HydroponicsObject *, N> &actuatorsIn, Vector<SharedPtr<HydroponicsActuator>, N> &actuatorsOut, Hydroponics_ActuatorType actuatorType)
{
    for (auto actIter = actuatorsIn.begin(); actIter != actuatorsIn.end(); ++actIter) {
        auto actuator = getSharedPtr<HydroponicsActuator>(*actIter);
        HYDRUINO_HARD_ASSERT(actuator, SFP(HStr_Err_OperationFailure));
        if (actuator->getActuatorType() == actuatorType) {
            actuatorsOut.push_back(actuator);
        }
    }
}

template<size_t N>
void linksResolveActuatorsPairRateByType(Vector<HydroponicsObject *, N> &actuatorsIn, float rateValue, Vector<Pair<SharedPtr<HydroponicsActuator>, float>, N> &actuatorsOut, Hydroponics_ActuatorType actuatorType)
{
    for (auto actIter = actuatorsIn.begin(); actIter != actuatorsIn.end(); ++actIter) {
        auto actuator = getSharedPtr<HydroponicsActuator>(*actIter);
        HYDRUINO_HARD_ASSERT(actuator, SFP(HStr_Err_OperationFailure));
        if (actuator->getActuatorType() == actuatorType) {
            auto pair = make_pair(actuator, rateValue);
            actuatorsOut.push_back(pair);
        }
    }
}


inline Hydroponics *getHydroponicsInstance()
{
    return Hydroponics::_activeInstance;
}

inline HydroponicsScheduler *getSchedulerInstance()
{
    return Hydroponics::_activeInstance ? &Hydroponics::_activeInstance->scheduler : nullptr;
}

inline HydroponicsLogger *getLoggerInstance()
{
    return Hydroponics::_activeInstance ? &Hydroponics::_activeInstance->logger : nullptr;
}

inline HydroponicsPublisher *getPublisherInstance()
{
    return Hydroponics::_activeInstance ? &Hydroponics::_activeInstance->publisher : nullptr;
}

#ifdef HYDRUINO_USE_VIRTMEM

inline BaseVAlloc *getVirtualAllocator()
{
    return Hydroponics::_activeInstance ? &(Hydroponics::_activeInstance->_vAlloc) : nullptr;
}

#endif
#ifndef HYDRUINO_DISABLE_GUI

inline HydroponicsUIInterface *getUIInstance()
{
    return Hydroponics::_activeInstance ? Hydroponics::_activeInstance->_activeUIInstance : nullptr;
}

#endif

inline DateTime getCurrentTime()
{
    return DateTime((uint32_t)(unixNow() + (getHydroponicsInstance() ? getHydroponicsInstance()->getTimeZoneOffset() * SECS_PER_HOUR : 0L)));
}

inline time_t getCurrentDayStartTime()
{
    DateTime currTime = getCurrentTime();
    return DateTime(currTime.year(), currTime.month(), currTime.day()).unixtime();
}

inline bool checkPinIsDigital(pintype_t pin)
{
    #ifdef ESP32
        return true; // all digital pins are ADC capable
    #else // separate analog from digital pins
        return !checkPinIsAnalogInput(pin) && !checkPinIsAnalogOutput(pin);
    #endif
}

inline bool checkPinIsPWMOutput(pintype_t pin)
{
    #if defined(digitalPinHasPWM)
        return digitalPinHasPWM(pin);
    #else
        return checkPinIsDigital(pin); // all digital pins are PWM capable
    #endif
}

inline bool checkPinCanInterrupt(pintype_t pin)
{
    return isValidPin(digitalPinToInterrupt(pin));
}

#endif // /ifndef HydroponicsUtils_HPP
