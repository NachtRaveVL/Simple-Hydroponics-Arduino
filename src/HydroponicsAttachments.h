/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Attachment Points
*/

#ifndef HydroponicsAttachments_H
#define HydroponicsAttachments_H

template<class T> class HydroponicsDLinkObject;
template<class T> class HydroponicsAttachment;
template<class T, class ParameterType, int Slots> class HydroponicsSignalAttachment;
class HydroponicsSensorAttachment;
class HydroponicsTriggerAttachment;
class HydroponicsBalancerAttachment;

#include "Hydroponics.h"
#include "HydroponicsObject.h"
#include "HydroponicsMeasurements.h"

// Delay/Dynamic Loaded/Linked Object Reference
// Simple class for delay loading objects that get references to others during system
// load. T should be a derived class type of HydroponicsObject, with getId() method.
template<class T>
class HydroponicsDLinkObject {
public:
    HydroponicsDLinkObject();
    HydroponicsDLinkObject(const HydroponicsIdentity &id);
    HydroponicsDLinkObject(const char *idKeyStr);

    inline bool isId() const { return !_obj && !_id.isSubObject(); }
    inline bool isObject() const { return (bool)_obj; }
    inline bool isResolved() const { return isObject(); }
    inline bool needsResolved() const { return isId() && (bool)_id; }
    inline bool resolveIfNeeded() { return needsResolved() && (bool)getObject(); }

    template<class U> inline void setObject(U obj) { this->operator=(obj); }
    shared_ptr<T> getObject();

    inline T* get() { return getObject().get(); }
    inline const HydroponicsIdentity &getId() const { return _id; }
    inline Hydroponics_KeyType getKey() const { return getId().key; }
    inline const String &getKeyString() const { return getId().keyString; }

    inline operator bool() const { return (bool)_obj; }
    inline T* operator->() { return get(); }
    inline T* operator*() { return get(); }

    template<class U> inline HydroponicsDLinkObject<T> &operator=(const HydroponicsDLinkObject<U> &rhs) { _id = rhs._id; _obj = static_pointer_cast<T>(rhs._obj); return *this; }
    inline HydroponicsDLinkObject<T> &operator=(const HydroponicsIdentity &rhs) { _id = rhs; _obj = nullptr; return *this; }
    inline HydroponicsDLinkObject<T> &operator=(const char *rhs) { _id = HydroponicsIdentity(rhs); _obj = nullptr; return *this; }
    template<class U> inline HydroponicsDLinkObject<T> &operator=(shared_ptr<U> &rhs) { _id = (rhs ? rhs->getId() : HydroponicsIdentity()); _obj = static_pointer_cast<T>(rhs); return *this; }
    template<class U> inline HydroponicsDLinkObject<T> &operator=(const U *rhs);

    template<class U> inline bool operator==(const HydroponicsDLinkObject<U> &rhs) const { return _id.key == rhs.getKey(); }
    inline bool operator==(const HydroponicsIdentity &rhs) const { return _id.key == rhs.key; }
    inline bool operator==(const char *rhs) const { return _id.key == HydroponicsIdentity(rhs).key; }
    template<class U> inline bool operator==(const shared_ptr<U> &rhs) const { return _id.key == (rhs ? rhs->getKey() : (Hydroponics_KeyType)-1); }
    template<class U> inline bool operator==(const U *rhs) const { return _id.key == (rhs ? rhs->getKey() : (Hydroponics_KeyType)-1); }

    template<class U> inline bool operator!=(const HydroponicsDLinkObject<U> &rhs) const { return _id.key != rhs.getKey(); }
    inline bool operator!=(const HydroponicsIdentity &rhs) const { return _id.key != rhs.key; }
    inline bool operator!=(const char *rhs) const { return _id.key != HydroponicsIdentity(rhs).key; }
    template<class U> inline bool operator!=(const shared_ptr<U> &rhs) const { return _id.key != (rhs ? rhs->getKey() : (Hydroponics_KeyType)-1); }
    template<class U> inline bool operator!=(const U *rhs) const { return _id.key != (rhs ? rhs->getKey() : (Hydroponics_KeyType)-1); }

protected:
    HydroponicsIdentity _id;                                // Object identity
    shared_ptr<T> _obj;                                     // Shared pointer to object
};


// Simple Attachment Point Base
// This attachment registers the parent object with the linked object's linkages upon
// dereference, and unregisters the parent object at time of destruction or reassignment.
template<class T>
class HydroponicsAttachment {
public:
    HydroponicsAttachment(HydroponicsObject *parent);
    virtual ~HydroponicsAttachment();

    virtual void attachObject();
    virtual void detachObject();

    inline bool isId() const { return _obj.isId(); }
    inline bool isObject() const { return _obj.isObject(); }
    inline bool isResolved() const { return _obj.isResolved(); }
    inline bool needsResolved() const { return _obj.needsResolved(); }
    inline bool resolveIfNeeded() { return needsResolved() && (bool)getObject(); }

    template<class U> inline void setObject(U obj) { this->operator=(obj); }
    inline shared_ptr<T> getObject() { if (needsResolved() && _obj.getObject()) { attachObject(); } return _obj.getObject(); }

    inline T* get() { return getObject().get(); }
    inline const HydroponicsIdentity &getId() const { return _obj.getId(); }
    inline Hydroponics_KeyType getKey() const { return _obj.getKey(); }
    inline const String &getKeyString() const { return _obj.getKeyString(); }

    inline operator bool() const { return (bool)_obj; }
    inline T* operator->() { return getObject().get(); }
    inline T* operator*() { return getObject().get(); }

    inline HydroponicsAttachment<T> &operator=(const HydroponicsIdentity &rhs) { if (_obj != rhs) { if (isResolved()) { detachObject(); } _obj = rhs; if (isResolved()) { attachObject(); } } return *this; }
    inline HydroponicsAttachment<T> &operator=(const char *rhs) { *this = HydroponicsIdentity(rhs); return *this; }
    template<class U> inline HydroponicsAttachment<T> &operator=(shared_ptr<U> rhs) { if (_obj != rhs) { if (isResolved()) { detachObject(); } _obj = rhs; if (isResolved()) { attachObject(); } } return *this; }
    template<class U> inline HydroponicsAttachment<T> &operator=(const U *rhs) { if (_obj != rhs) { if (isResolved()) { detachObject(); } _obj = rhs; if (isResolved()) { attachObject(); } } return *this; }

    inline bool operator==(const HydroponicsIdentity &rhs) const { return _obj == rhs; }
    inline bool operator==(const char *rhs) { return *this == HydroponicsIdentity(rhs); }
    template<class U> inline bool operator==(const shared_ptr<U> &rhs) const { return _obj == rhs; }
    template<class U> inline bool operator==(const U *rhs) const { return _obj == rhs; }

    inline bool operator!=(const HydroponicsIdentity &rhs) const { return _obj != rhs; }
    inline bool operator!=(const char *rhs) { return *this != HydroponicsIdentity(rhs); }
    template<class U> inline bool operator!=(const shared_ptr<U> &rhs) const { return _obj != rhs; }
    template<class U> inline bool operator!=(const U *rhs) const { return _obj != rhs; }

protected:
    HydroponicsDLinkObject<T> _obj;                         // Dynamic link object
    HydroponicsObject *_parent;                             // Parent object pointer (strong due to reverse ownership)
};


// Signal Attachment Point
// This attachment registers the parent object with a Signal getter off the linked object
// upon dereference, and unregisters the parent object from the Signal at time of
// destruction or reassignment.
template<class T, class ParameterType, int Slots = 8>
class HydroponicsSignalAttachment : public HydroponicsAttachment<T> {
public:
    typedef Signal<ParameterType,Slots> &(T::*SignalGetterPtr)(void);

    template<class U> HydroponicsSignalAttachment(HydroponicsObject *parent, SignalGetterPtr signalGetter, MethodSlot<U,ParameterType> handleMethod);
    virtual ~HydroponicsSignalAttachment();

    virtual void attachObject() override;
    virtual void detachObject() override;

    inline HydroponicsSignalAttachment<T,ParameterType,Slots> &operator=(const HydroponicsIdentity &rhs) { if (HydroponicsAttachment<T>::_obj != rhs) { if (isResolved()) { detachObject(); } HydroponicsAttachment<T>::_obj = rhs; if (isResolved()) { attachObject(); } } return *this; }
    inline HydroponicsSignalAttachment<T,ParameterType,Slots> &operator=(const char *rhs) { *this = HydroponicsIdentity(rhs); return *this; }
    template<class U> inline HydroponicsSignalAttachment<T,ParameterType,Slots> &operator=(shared_ptr<U> rhs) { if (HydroponicsAttachment<T>::_obj != rhs) { if (isResolved()) { detachObject(); } HydroponicsAttachment<T>::_obj = rhs; if (isResolved()) { attachObject(); } } return *this; }
    template<class U> inline HydroponicsSignalAttachment<T,ParameterType,Slots> &operator=(const U *rhs) { if (HydroponicsAttachment<T>::_obj != rhs) { if (isResolved()) { detachObject(); } HydroponicsAttachment<T>::_obj = rhs; if (isResolved()) { attachObject(); } } return *this; }

protected:
    SignalGetterPtr _signalGetter;                          // Signal getter method ptr
    MethodSlot<HydroponicsObject,ParameterType> _handleMethod; // Handler method slot (handler target stored as HydroponicsObject since type doesn't matter here)
};


// Sensor Measurement Attachment Point
// This attachment registers the parent object with a Sensor's new measurement Signal
// upon dereference, and unregisters the parent object from the Sensor at time of
// destruction or reassignment.
// Custom process method can handle new raw measurements, while custom update method
// can handle updates to the single result measure (read as: called upon every new
// measure). Custom process method must call setMeasurement to set resolved measure.
class HydroponicsSensorAttachment : public HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS> {
public:
    typedef void (HydroponicsObject::*ProcessMethodPtr)(const HydroponicsMeasurement *measurement);
    typedef void (HydroponicsObject::*UpdateMethodPtr)(const HydroponicsSingleMeasurement &measurement);

    HydroponicsSensorAttachment(HydroponicsObject *parent, Hydroponics_PositionIndex measurementRow = 0);
    virtual ~HydroponicsSensorAttachment();

    virtual void attachObject() override;
    virtual void detachObject() override;

    void updateMeasurementIfNeeded(bool force = false);

    template<class T> inline void setProcessMethod(void (T::*processMethod)(const HydroponicsMeasurement *)) { _processMethod = (ProcessMethodPtr)processMethod; setNeedsMeasurement(); }
    template<class T> inline void setUpdateMethod(void (T::*updateMethod)(const HydroponicsSingleMeasurement &measurement))  { _updateMethod = (UpdateMethodPtr)updateMethod; }

    inline void setProcessMethod(ProcessMethodPtr processMethod) { _processMethod = processMethod; setNeedsMeasurement(); }
    inline void setUpdateMethod(UpdateMethodPtr updateMethod)  { _updateMethod = updateMethod; }
    inline ProcessMethodPtr getProcessMethod() const { return _processMethod; }
    inline UpdateMethodPtr getUpdateMethod() const { return _updateMethod; }

    void setMeasurement(float value, Hydroponics_UnitsType units = Hydroponics_UnitsType_Undefined);
    void setMeasurement(HydroponicsSingleMeasurement measurement);
    void setMeasurementRow(Hydroponics_PositionIndex measurementRow);
    void setMeasurementUnits(Hydroponics_UnitsType units, float convertParam = FLT_UNDEF);

    inline void setNeedsMeasurement() { _needsMeasurement = true; }
    inline bool needsMeasurement() { return _needsMeasurement; }

    inline const HydroponicsSingleMeasurement &getMeasurement(bool force = false) { updateMeasurementIfNeeded(force); return _measurement; }
    inline uint16_t getMeasurementFrame(bool force = false) { updateMeasurementIfNeeded(force); return _measurement.frame; }
    inline float getMeasurementValue(bool force = false) { updateMeasurementIfNeeded(force); return _measurement.value; }
    inline Hydroponics_UnitsType getMeasurementUnits(bool force = false) { updateMeasurementIfNeeded(force); return _measurement.units; }

    inline Hydroponics_PositionIndex getMeasurementRow() const { return _measurementRow; }
    inline float getMeasurementConvertParam() const { return _convertParam; }

    inline HydroponicsSensorAttachment &operator=(const HydroponicsIdentity &rhs) { if (_obj != rhs) { if (isResolved()) { detachObject(); } _obj = rhs; if (isResolved()) { attachObject(); } } return *this; }
    inline HydroponicsSensorAttachment &operator=(const char *rhs) { *this = HydroponicsIdentity(rhs); return *this; }
    template<class U> inline HydroponicsSensorAttachment &operator=(shared_ptr<U> rhs) { if (_obj != rhs) { if (isResolved()) { detachObject(); } _obj = rhs; if (isResolved()) { attachObject(); } } return *this; }
    template<class U> inline HydroponicsSensorAttachment &operator=(const U *rhs) { if (_obj != rhs) { if (isResolved()) { detachObject(); } _obj = rhs; if (isResolved()) { attachObject(); } } return *this; }

protected:
    HydroponicsSingleMeasurement _measurement;              // Local measurement
    Hydroponics_PositionIndex _measurementRow;              // Measurement row
    float _convertParam;                                    // Convert param (default: FLT_UNDEF)
    bool _needsMeasurement;                                 // Stale measurement tracking flag
    ProcessMethodPtr _processMethod;                        // Custom process method
    UpdateMethodPtr _updateMethod;                          // Custom update method

    void handleMeasurement(const HydroponicsMeasurement *measurement);
};


// Trigger State Attachment Point
// This attachment registers the parent object with a Triggers's trigger Signal
// upon dereference, and unregisters the parent object from the Trigger at time of
// destruction or reassignment.
// Custom process method can handle new raw state, while custom update method can
// handle changes to the trigger state (read as: only called upon change). Custom
// process method must call setTriggerState to set resolved state.
class HydroponicsTriggerAttachment  : public HydroponicsSignalAttachment<HydroponicsTrigger, Hydroponics_TriggerState, HYDRUINO_TRIGGER_STATE_SLOTS> {
public:
    typedef void (HydroponicsObject::*ProcessMethodPtr)(Hydroponics_TriggerState triggerState);
    typedef void (HydroponicsObject::*UpdateMethodPtr)(Hydroponics_TriggerState triggerState);

    HydroponicsTriggerAttachment(HydroponicsObject *parent);
    virtual ~HydroponicsTriggerAttachment();

    virtual void attachObject() override;
    virtual void detachObject() override;

    void updateTriggerIfNeeded(bool force = false);

    template<class T> inline void setProcessMethod(void (T::*processMethod)(Hydroponics_TriggerState)) { _processMethod = (ProcessMethodPtr)processMethod; setNeedsTriggerState(); }
    template<class T> inline void setUpdateMethod(void (T::*updateMethod)(Hydroponics_TriggerState))  { _updateMethod = (UpdateMethodPtr)updateMethod; }
    inline ProcessMethodPtr getProcessMethod() const { return _processMethod; }
    inline UpdateMethodPtr getUpdateMethod() const { return _updateMethod; }

    inline void setNeedsTriggerState() { _needsTriggerState = true; }
    inline bool needsTriggerState() { return _needsTriggerState; }

    void setTriggerState(Hydroponics_TriggerState triggerState);

    inline Hydroponics_TriggerState getTriggerState(bool force = false) { updateTriggerIfNeeded(force); return _triggerState; }

    inline HydroponicsTriggerAttachment &operator=(const HydroponicsIdentity &rhs) { if (_obj != rhs) { if (isResolved()) { detachObject(); } _obj = rhs; if (isResolved()) { attachObject(); } } return *this; }
    inline HydroponicsTriggerAttachment &operator=(const char *rhs) { *this = HydroponicsIdentity(rhs); return *this; }
    template<class U> inline HydroponicsTriggerAttachment &operator=(shared_ptr<U> rhs) { if (_obj != rhs) { if (isResolved()) { detachObject(); } _obj = rhs; if (isResolved()) { attachObject(); } } return *this; }
    template<class U> inline HydroponicsTriggerAttachment &operator=(const U *rhs) { if (_obj != rhs) { if (isResolved()) { detachObject(); } _obj = rhs; if (isResolved()) { attachObject(); } } return *this; }

protected:
    Hydroponics_TriggerState _triggerState;                 // Tracked trigger state
    bool _needsTriggerState;                                // Stale data tracking flag
    ProcessMethodPtr _processMethod;                        // Custom process method
    UpdateMethodPtr _updateMethod;                          // Custom update method

    void handleTrigger(Hydroponics_TriggerState triggerState);
};


// Balancer State Attachment Point
// This attachment registers the parent object with a Balancer's balance Signal
// upon dereference, and unregisters the parent object from the Balancer at time of
// destruction or reassignment.
// Custom process method can handle new raw state, while custom update method can
// handle changes to the balancer state (read as: only called upon change). Custom
// process method must call setBalancerState to set resolved state.
class HydroponicsBalancerAttachment : public HydroponicsSignalAttachment<HydroponicsBalancer, Hydroponics_BalancerState, HYDRUINO_BALANCER_STATE_SLOTS> {
public:
    typedef void (HydroponicsObject::*ProcessMethodPtr)(Hydroponics_BalancerState balancerState);
    typedef void (HydroponicsObject::*UpdateMethodPtr)(Hydroponics_BalancerState balancerState);

    HydroponicsBalancerAttachment(HydroponicsObject *parent);
    virtual ~HydroponicsBalancerAttachment();

    virtual void attachObject() override;
    virtual void detachObject() override;

    void updateBalancerIfNeeded(bool force = false);

    template<class T> inline void setProcessMethod(void (T::*processMethod)(Hydroponics_BalancerState)) { _processMethod = (ProcessMethodPtr)processMethod; setNeedsBalancerState(); }
    template<class T> inline void setUpdateMethod(void (T::*updateMethod)(Hydroponics_BalancerState))  { _updateMethod = (UpdateMethodPtr)updateMethod; }
    inline ProcessMethodPtr getProcessMethod() const { return _processMethod; }
    inline UpdateMethodPtr getUpdateMethod() const { return _updateMethod; }

    inline void setNeedsBalancerState() { _needsBalancerState = true; }
    inline bool needsBalancerState() { return _needsBalancerState; }

    void setBalancerState(Hydroponics_BalancerState balancerState);

    inline Hydroponics_BalancerState getBalancerState(bool force = false) { updateBalancerIfNeeded(force); return _balancerState; }

    inline HydroponicsBalancerAttachment &operator=(const HydroponicsIdentity &rhs) { if (_obj != rhs) { if (isResolved()) { detachObject(); } _obj = rhs; if (isResolved()) { attachObject(); } } return *this; }
    inline HydroponicsBalancerAttachment &operator=(const char *rhs) { *this = HydroponicsIdentity(rhs); return *this; }
    template<class U> inline HydroponicsBalancerAttachment &operator=(shared_ptr<U> rhs) { if (_obj != rhs) { if (isResolved()) { detachObject(); } _obj = rhs; if (isResolved()) { attachObject(); } } return *this; }
    template<class U> inline HydroponicsBalancerAttachment &operator=(const U *rhs) { if (_obj != rhs) { if (isResolved()) { detachObject(); } _obj = rhs; if (isResolved()) { attachObject(); } } return *this; }

protected:
    Hydroponics_BalancerState _balancerState;               // Tracked balancer state
    bool _needsBalancerState;                               // Stale date tracking flag
    ProcessMethodPtr _processMethod;                        // Custom process method
    UpdateMethodPtr _updateMethod;                          // Custom update method

    void handleBalancer(Hydroponics_BalancerState balancerState);
};

#endif // /ifndef HydroponicsAttachments_H
