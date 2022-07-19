/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Utilities
*/

#ifndef HydroponicsUtils_HPP
#define HydroponicsUtils_HPP

#include "HydroponicsUtils.h"

// Delay/Dynamic Loaded/Linked Object Reference
// Simple class for delay loading objects that get references to others during system
// load. T should be a derived class type of HydroponicsObject, with getId() method.
template<class T>
struct HydroponicsDLinkObject {
    HydroponicsIdentity id;
    shared_ptr<T> obj;

    HydroponicsDLinkObject()
        : id(), obj(nullptr) { ; }
    template<class U>
    HydroponicsDLinkObject(const HydroponicsDLinkObject<U> &rhs)
        : id(rhs.id), obj(static_pointer_cast<T>(rhs.obj)) { ; }
    HydroponicsDLinkObject(HydroponicsIdentity idIn)
        : id(idIn), obj(nullptr) { ; }
    template<class U>
    HydroponicsDLinkObject(shared_ptr<U> objIn)
        : id(objIn->getId()), obj(static_pointer_cast<T>(objIn)) { ; }
    HydroponicsDLinkObject(const char *keyStrIn)
        : HydroponicsDLinkObject(HydroponicsIdentity(keyStrIn)) { ; }
    ~HydroponicsDLinkObject() { ; }

    inline bool isId() const { return !obj; }
    inline bool isObj() const { return (bool)obj; }
    inline bool needsResolved() const { return (!obj && (bool)id); }
    inline bool resolveIfNeeded() { return (!obj ? (bool)getObj() : false); }
    inline operator bool() const { return isObj(); }

    HydroponicsIdentity getId() const { return obj ? obj->getId() : id;  }
    Hydroponics_KeyType getKey() const { return obj ? obj->getId().key : id.key; }

    shared_ptr<T> getObj() { if (!obj && (bool)id) { auto hydroponics = getHydroponicsInstance();
                                                     if (hydroponics) { obj = hydroponics->objectById(id); }
                                                     if (isObj()) { id = obj->getId(); } }
                             return obj; }
    inline T* operator->() { return getObj().get(); }

    template<class U>
    HydroponicsDLinkObject<T> &operator=(const HydroponicsDLinkObject<U> &rhs) { id = rhs.id; obj = static_pointer_cast<T>(rhs.obj); }
    HydroponicsDLinkObject<T> &operator=(const HydroponicsIdentity rhs) { id = rhs; obj = nullptr; }
    template<class U>
    HydroponicsDLinkObject<T> &operator=(shared_ptr<U> rhs) { obj = static_pointer_cast<T>(rhs); id = obj->getId(); }

    template<class U>
    bool operator==(const HydroponicsDLinkObject<U> &rhs) const { return id.key == rhs->getId().key; }
    bool operator==(const HydroponicsIdentity rhs) const { return id.key == rhs.key; }
    template<class U>
    bool operator==(shared_ptr<U> rhs) const { return id.key == rhs->getId().key; }
    bool operator==(HydroponicsObject *rhs) const { return id.key == rhs->getId().key; }

    template<class U>
    bool operator!=(const HydroponicsDLinkObject<U> &rhs) const { return id.key != rhs->getId().key; }
    bool operator!=(const HydroponicsIdentity rhs) const { return id.key != rhs.key; }
    template<class U>
    bool operator!=(shared_ptr<U> rhs) const { return id.key != rhs->getId().key; }
    bool operator!=(HydroponicsObject *rhs) const { return id.key != rhs->getId().key; }
};


// Signal Fire Task
// This class holds onto the passed signal and parameter to pass it along to the signal's
// fire method upon task execution.
template<typename ParameterType, int Slots>
class SignalFireTask : public Executable {
public:
    taskid_t taskId;
    SignalFireTask(shared_ptr<HydroponicsObject> object,
                   Signal<ParameterType,Slots> &signal,
                   ParameterType &param)
        : taskId(TASKMGR_INVALIDID), _object(object), _signal(&signal), _param(param) { ; }
    virtual ~SignalFireTask() { ; }

    void exec() override { _signal->fire(_param); }
private:
    shared_ptr<HydroponicsObject> _object;
    Signal<ParameterType, Slots> *_signal;
    ParameterType _param;
};

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


// Method Slot Task
// This class holds onto a MethodSlot to call once executed.
template<class ObjectType, typename ParameterType>
class MethodSlotCallTask : public Executable {
public:
    typedef void (ObjectType::*FunctPtr)(ParameterType);
    taskid_t taskId;

    MethodSlotCallTask(shared_ptr<ObjectType> object, FunctPtr method, ParameterType callParam) : taskId(TASKMGR_INVALIDID), _object(object), _methodSlot(object.get(), method), _callParam(callParam) { ; }
    MethodSlotCallTask(ObjectType *object, FunctPtr method, ParameterType callParam) : taskId(TASKMGR_INVALIDID), _object(nullptr), _methodSlot(object, method), _callParam(callParam) { ; }
    virtual ~MethodSlotCallTask() { ; }

    void exec() override { _methodSlot(_callParam); }
private:
    shared_ptr<ObjectType> _object;
    MethodSlot<ObjectType,ParameterType> _methodSlot;
    ParameterType _callParam;

    friend taskid_t scheduleObjectMethodCallWithTaskIdOnce<ObjectType>(shared_ptr<ObjectType> object, void (ObjectType::*method)(taskid_t));
    friend taskid_t scheduleObjectMethodCallWithTaskIdOnce<ObjectType>(ObjectType *object, void (ObjectType::*method)(taskid_t));
};

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


template<typename T>
String commaStringFromArray(const T *arrayIn, size_t length)
{
    if (!arrayIn || !length) { return String(SFP(HS_Null)); }
    String retVal; retVal.reserve(length << 1);
    for (size_t index = 0; index < length; ++index) {
        if (retVal.length()) { retVal.concat(','); }
        retVal += String(arrayIn[index]);
    }
    return retVal.length() ? retVal : String(SFP(HS_Null));
}

template<typename T>
void commaStringToArray(String stringIn, T *arrayOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(SFP(HS_Null))) { return; }
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


template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterActuators(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links)
{
    typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isActuatorType()) {
            retVal.insert(iter->first, iter->second);
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterActuatorsByType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, Hydroponics_ActuatorType actuatorType)
{
    typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isActuatorType()) {
            auto actuatorObj = (HydroponicsActuator *)(iter->second);
            if (actuatorObj && actuatorObj->getActuatorType() == actuatorType) {
                retVal.insert(iter->first, actuatorObj);
            }
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterSensors(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links)
{
    typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isSensorType()) {
            retVal.insert(iter->first, iter->second);
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterSensorsByType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, Hydroponics_SensorType sensorType)
{
    typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isSensorType()) {
            auto sensorObj = (HydroponicsSensor *)(iter->second);
            if (sensorObj->getSensorType() == sensorType) {
                retVal.insert(iter->first, sensorObj);
            }
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterCrops(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links)
{
    typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isCropType()) {
            retVal.insert(iter->first, iter->second);
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterCropsByType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, Hydroponics_CropType cropType)
{
    typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isCropType()) {
            auto cropObj = (HydroponicsCrop *)(iter->second);
            if (cropObj->getCropType() == cropType) {
                retVal.insert(iter->first, cropObj);
            }
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterReservoirs(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links)
{
    typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isReservoirType()) {
            retVal.insert(iter->first, iter->second);
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterReservoirsByType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, Hydroponics_ReservoirType reservoirType)
{
    typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isReservoirType()) {
            auto reservoirObj = (HydroponicsReservoir *)(iter->second);
            if (reservoirObj->getReservoirType() == reservoirType) {
                retVal.insert(iter->first, reservoirObj);
            }
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterRails(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links)
{
    typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isRailType()) {
            retVal.insert(iter->first, iter->second);
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterRailsByType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, Hydroponics_RailType railType)
{
    typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isRailType()) {
            auto railObj = (HydroponicsRail *)(iter->second);
            if (railObj->getRailType() == railType) {
                retVal.insert(iter->first, railObj);
            }
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterPumpActuatorsByInputReservoir(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, HydroponicsReservoir *inputReservoir)
{
    typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isActuatorType()) {
            auto actuatorObj = (HydroponicsActuator *)(iter->second);
            if (actuatorObj->isAnyPumpClass() && ((HydroponicsPumpObjectInterface *)actuatorObj)->getReservoir().get() == inputReservoir) {
                retVal.insert(iter->first, actuatorObj);
            }
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterPumpActuatorsByInputReservoirType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, Hydroponics_ReservoirType reservoirType)
{
    typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isActuatorType()) {
            auto actuatorObj = (HydroponicsActuator *)(iter->second);
            if (actuatorObj->isAnyPumpClass() && ((HydroponicsPumpObjectInterface *)actuatorObj)->getReservoir() && ((HydroponicsPumpObjectInterface *)actuatorObj)->getReservoir()->getReservoirType() == reservoirType) {
                retVal.insert(iter->first, actuatorObj);
            }
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterPumpActuatorsByOutputReservoir(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, HydroponicsReservoir *outputReservoir)
{
    typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isActuatorType()) {
            auto actuatorObj = (HydroponicsActuator *)(iter->second);
            if (actuatorObj->isAnyPumpClass() && ((HydroponicsPumpObjectInterface *)actuatorObj)->getOutputReservoir().get() == outputReservoir) {
                retVal.insert(iter->first, actuatorObj);
            }
        }
    }

    return retVal;
}

template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE>
typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterPumpActuatorsByOutputReservoirType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, Hydroponics_ReservoirType reservoirType)
{
    typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type retVal;

    for (auto iter = links.begin(); iter != links.end(); ++iter) {
        if (iter->second && iter->second->isActuatorType()) {
            auto actuatorObj = (HydroponicsActuator *)(iter->second);
            if (actuatorObj->isAnyPumpClass() && ((HydroponicsPumpObjectInterface *)actuatorObj)->getOutputReservoir() && ((HydroponicsPumpObjectInterface *)actuatorObj)->getOutputReservoir()->getReservoirType() == reservoirType) {
                retVal.insert(iter->first, actuatorObj);
            }
        }
    }

    return retVal;
}

#endif // /ifndef HydroponicsUtils_HPP
