/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Utilities
*/

#ifndef HydroponicsUtils_HPP
#define HydroponicsUtils_HPP

#include "Hydroponics.h"

template<class T>
shared_ptr<T> HydroponicsDLinkObject<T>::getObj() {
    if (!obj && (bool)id) {
        if (getHydroponicsInstance()) { obj = getHydroponicsInstance()->objectById(id); }
        if ((bool)obj) { id = obj->getId(); }
    }
    return obj;
}

#ifndef HYDRUINO_DISABLE_MULTITASKING

template<typename ParameterType, int Slots>
taskid_t scheduleSignalFireOnce(shared_ptr<HydroponicsObject> object, Signal<ParameterType,Slots> &signal, ParameterType fireParam)
{
    SignalFireTask<ParameterType,Slots> *fireTask = object ? new SignalFireTask<ParameterType,Slots>(object, signal, fireParam) : nullptr;
    HYDRUINO_SOFT_ASSERT(!object || fireTask, SFP(HS_Err_AllocationFailure));
    taskid_t retVal = fireTask ? taskManager.scheduleOnce(0, fireTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (fireTask ? (fireTask->taskId = retVal) : retVal);
}

template<typename ParameterType, int Slots>
taskid_t scheduleSignalFireOnce(Signal<ParameterType,Slots> &signal, ParameterType fireParam)
{
    SignalFireTask<ParameterType,Slots> *fireTask = new SignalFireTask<ParameterType,Slots>(nullptr, signal, fireParam);
    HYDRUINO_SOFT_ASSERT(fireTask, SFP(HS_Err_AllocationFailure));
    taskid_t retVal = fireTask ? taskManager.scheduleOnce(0, fireTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (fireTask ? (fireTask->taskId = retVal) : retVal);
}

template<class ObjectType, typename ParameterType>
taskid_t scheduleObjectMethodCallOnce(shared_ptr<ObjectType> object, void (ObjectType::*method)(ParameterType), ParameterType callParam)
{
    MethodSlotCallTask<ObjectType,ParameterType> *callTask = object ? new MethodSlotCallTask<ObjectType,ParameterType>(object, method, callParam) : nullptr;
    HYDRUINO_SOFT_ASSERT(!object || callTask, SFP(HS_Err_AllocationFailure));
    taskid_t retVal = callTask ? taskManager.scheduleOnce(0, callTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (callTask ? (callTask->taskId = retVal) : retVal);
}

template<class ObjectType, typename ParameterType>
taskid_t scheduleObjectMethodCallOnce(ObjectType *object, void (ObjectType::*method)(ParameterType), ParameterType callParam)
{
    MethodSlotCallTask<ObjectType,ParameterType> *callTask = object ? new MethodSlotCallTask<ObjectType,ParameterType>(object, method, callParam) : nullptr;
    HYDRUINO_SOFT_ASSERT(!object || callTask, SFP(HS_Err_AllocationFailure));
    taskid_t retVal = callTask ? taskManager.scheduleOnce(0, callTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (callTask ? (callTask->taskId = retVal) : retVal);
}

template<class ObjectType>
taskid_t scheduleObjectMethodCallWithTaskIdOnce(shared_ptr<ObjectType> object, void (ObjectType::*method)(taskid_t))
{
    MethodSlotCallTask<ObjectType,taskid_t> *callTask = object ? new MethodSlotCallTask<ObjectType,taskid_t>(object, method, (taskid_t)0) : nullptr;
    HYDRUINO_SOFT_ASSERT(!object || callTask, SFP(HS_Err_AllocationFailure));
    taskid_t retVal = callTask ? taskManager.scheduleOnce(0, callTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (callTask ? (callTask->taskId = (callTask->_callParam = retVal)) : retVal);
}

template<class ObjectType>
taskid_t scheduleObjectMethodCallWithTaskIdOnce(ObjectType *object, void (ObjectType::*method)(taskid_t))
{
    MethodSlotCallTask<ObjectType,taskid_t> *callTask = object ? new MethodSlotCallTask<ObjectType,taskid_t>(object, method, (taskid_t)0) : nullptr;
    HYDRUINO_SOFT_ASSERT(!object || callTask, SFP(HS_Err_AllocationFailure));
    taskid_t retVal = callTask ? taskManager.scheduleOnce(0, callTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
    return (callTask ? (callTask->taskId = (callTask->_callParam = retVal)) : retVal);
}

#endif // /ifndef HYDRUINO_DISABLE_MULTITASKING


template<typename T>
String commaStringFromArray(const T *arrayIn, size_t length)
{
    if (!arrayIn || !length) { return String(SFP(HS_null)); }
    String retVal; retVal.reserve(length << 1);
    for (size_t index = 0; index < length; ++index) {
        if (retVal.length()) { retVal.concat(','); }
        retVal += String(arrayIn[index]);
    }
    return retVal.length() ? retVal : String(SFP(HS_null));
}

template<typename T>
void commaStringToArray(String stringIn, T *arrayOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(SFP(HS_null))) { return; }
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

template<typename T>
T mapValue(T value, T inMin, T inMax, T outMin, T outMax)
{
    return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}


template<size_t N = HYDRUINO_OBJ_LINKSFILTER_DEFSIZE, size_t M = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Vector<HydroponicsObject *, N>::type linksFilterActuators(const typename Map<Hydroponics_KeyType, HydroponicsObject *, M>::type &links)
{
    typename Vector<HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isActuatorType()) {
            retVal.push_back(iter->second);
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKSFILTER_DEFSIZE, size_t M = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Vector<HydroponicsObject *, N>::type linksFilterCrops(const typename Map<Hydroponics_KeyType, HydroponicsObject *, M>::type &links)
{
    typename Vector<HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isCropType()) {
            retVal.push_back(iter->second);
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKSFILTER_DEFSIZE, size_t M = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Vector<HydroponicsObject *, N>::type linksFilterActuatorsByReservoirAndType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, M>::type &links, HydroponicsReservoir *srcReservoir, Hydroponics_ActuatorType actuatorType)
{
    typename Vector<HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isActuatorType()) {
            auto actuator = (HydroponicsActuator *)(iter->second);

            if (actuator->getActuatorType() == actuatorType && actuator->getReservoir().get() == srcReservoir) {
                retVal.push_back(iter->second);
            }
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKSFILTER_DEFSIZE, size_t M = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Vector<HydroponicsObject *, N>::type linksFilterPumpActuatorsByInputReservoirAndOutputReservoirType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, M>::type &links, HydroponicsReservoir *srcReservoir, Hydroponics_ReservoirType destReservoirType)
{
    typename Vector<HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isActuatorType()) {
            auto actuator = (HydroponicsActuator *)(iter->second);

            if (actuator->isAnyPumpClass() && actuator->getReservoir().get() == srcReservoir) {
                auto outputReservoir = ((HydroponicsPumpObjectInterface *)actuator)->getOutputReservoir();

                if (outputReservoir && outputReservoir->getReservoirType() == destReservoirType) {
                    retVal.push_back(iter->second);
                }
            }
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKSFILTER_DEFSIZE, size_t M = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Vector<HydroponicsObject *, N>::type linksFilterPumpActuatorsByOutputReservoirAndInputReservoirType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, M>::type &links, HydroponicsReservoir *destReservoir, Hydroponics_ReservoirType srcReservoirType)
{
    typename Vector<HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isActuatorType()) {
            auto actuator = (HydroponicsActuator *)(iter->second);

            if (actuator->isAnyPumpClass() && ((HydroponicsPumpObjectInterface *)actuator)->getOutputReservoir().get() == destReservoir) {
                auto inputReservoir = ((HydroponicsPumpObjectInterface *)actuator)->getReservoir().get();

                if (inputReservoir && inputReservoir->getReservoirType() == srcReservoirType) {
                    retVal.push_back(iter->second);
                }
            }
        }
    }

    return retVal;
}

template<size_t M = HYDRUINO_OBJ_LINKS_MAXSIZE>
int linksCountCrops(const typename Map<Hydroponics_KeyType, HydroponicsObject *, M>::type &links)
{
    int retVal = 0;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isCropType()) {
            retVal++;
        }
    }

    return retVal;
}

template<size_t M = HYDRUINO_OBJ_LINKS_MAXSIZE>
int linksCountActuatorsByReservoirAndType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, M>::type &links, HydroponicsReservoir *srcReservoir, Hydroponics_ActuatorType actuatorType)
{
    int retVal = 0;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isActuatorType()) {
            auto actuator = (HydroponicsActuator *)(iter->second);

            if (actuator->getActuatorType() == actuatorType && actuator->getReservoir().get() == srcReservoir) {
                retVal++;
            }
        }
    }

    return retVal;
}

#endif // /ifndef HydroponicsUtils_HPP
