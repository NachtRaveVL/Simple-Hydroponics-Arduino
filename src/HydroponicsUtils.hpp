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

    HydroponicsDLinkObject() : id(), obj(nullptr) { ; }
    template<class U>
    HydroponicsDLinkObject(const HydroponicsDLinkObject<U> &rhs) : id(rhs.id), obj(rhs.obj) { ; }
    HydroponicsDLinkObject(HydroponicsIdentity idIn) : id(idIn), obj(nullptr) { ; }
    template<class U>
    HydroponicsDLinkObject(shared_ptr<T> objIn) : id(objIn->getId()), obj(reinterpret_pointer_cast<T>(objIn)) { ; }
    ~HydroponicsDLinkObject() { obj = nullptr; }

    inline bool isId() const { return !obj; }
    inline bool isObj() const { return (bool)obj; }
    inline bool needsResolved() const { return !obj && (bool)id; }
    inline bool resolveIfNeeded() { return !obj ? (bool)getObj() : false; }
    operator bool() const { return (bool)obj; }

    HydroponicsIdentity getId() const { return obj ? obj->getId() : id;  }
    Hydroponics_KeyType getKey() const { return obj ? obj->getId().key : id.key; }

    shared_ptr<T> getObj() { if (!obj && (bool)id) { auto hydroponics = getHydroponicsInstance();
                                                     obj = (hydroponics ? hydroponics->objectById(id) : nullptr); }
                             return obj; }
    T* operator->() { return getObj().get(); }

    template<class U>
    HydroponicsDLinkObject<T> &operator=(const HydroponicsDLinkObject<U> &rhs) { id = rhs.id; obj = reinterpret_pointer_cast<T>(rhs.obj); }
    HydroponicsDLinkObject<T> &operator=(const HydroponicsIdentity rhs) { id = rhs; obj = nullptr; }
    template<class U>
    HydroponicsDLinkObject<T> &operator=(shared_ptr<U> rhs) { obj = reinterpret_pointer_cast<T>(rhs); id = obj->getId(); }

    template<class U>
    bool operator==(const HydroponicsDLinkObject<U> &rhs) const { return id.key == rhs->getId().key; }
    bool operator==(const HydroponicsIdentity rhs) const { return id.key == rhs.key; }
    template<class U>
    bool operator==(shared_ptr<U> rhs) const { return id.key == rhs->getId().key; }

    template<class U>
    bool operator!=(const HydroponicsDLinkObject<U> &rhs) const { return id.key != rhs->getId().key; }
    bool operator!=(const HydroponicsIdentity rhs) const { return id.key != rhs.key; }
    template<class U>
    bool operator!=(shared_ptr<U> rhs) const { return id.key != rhs->getId().key; }
};

// Signal Fire Task
// This class holds onto the passed signal and parameter to pass it along to the signal's
// fire method upon task execution. This effectively links Callback into TaskManagerIO.
template<typename ParameterType, int Slots>
class SignalFireTask : public Executable {
public:
    SignalFireTask(Signal<ParameterType,Slots> &signal, ParameterType &param) : _signal(&signal), _param(param) { ; }
    virtual ~SignalFireTask() { ; }
    void exec() override { _signal->fire(_param); }
private:
    Signal<ParameterType, Slots> *_signal;
    ParameterType _param;
};

template<typename ParameterType, int Slots>
taskid_t scheduleSignalFireOnce(Signal<ParameterType,Slots> &signal, ParameterType fireParam)
{
    SignalFireTask<ParameterType,Slots> *fireTask = new SignalFireTask<ParameterType,Slots>(signal, fireParam);
    HYDRUINO_SOFT_ASSERT(fireTask, "Failure allocating signal fire task");
    return fireTask != nullptr ? taskManager.scheduleOnce(0, fireTask, TIME_MILLIS, true) : TASKMGR_INVALIDID;
}

#endif // /ifndef HydroponicsUtils_HPP
