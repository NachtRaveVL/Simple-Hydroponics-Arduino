/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Attachment Points
*/

#ifndef HydroponicsAttachments_H
#define HydroponicsAttachments_H

class HydroponicsDLinkObject;
class HydroponicsAttachment;
template<class ParameterType, int Slots> class HydroponicsSignalAttachment;
class HydroponicsSensorAttachment;
class HydroponicsTriggerAttachment;
class HydroponicsBalancerAttachment;

#include "Hydroponics.h"
#include "HydroponicsObject.h"
#include "HydroponicsMeasurements.h"

extern Hydroponics_KeyType stringHash(String string); // forward decl


// Delay/Dynamic Loaded/Linked Object Reference
// Simple class for delay loading objects that get references to others during system
// load. T should be a derived class of HydroponicsObjInterface, with getId() method.
class HydroponicsDLinkObject {
public:
    HydroponicsDLinkObject();
    HydroponicsDLinkObject(const HydroponicsDLinkObject &obj);
    virtual ~HydroponicsDLinkObject();

    inline bool isId() const { return !_obj; }
    inline bool isObject() const { return (bool)_obj; }
    inline bool isResolved() const { return isObject(); }
    inline bool needsResolved() const { return isId() && _key != (Hydroponics_KeyType)-1; }
    inline bool resolve() { return (bool)getObject<HydroponicsObjInterface>(); }

    template<class U> inline void setObject(U obj) { *this = obj; }
    template<class U = HydroponicsObjInterface> shared_ptr<U> getObject();
    void detachObject();

    template<class U = HydroponicsObjInterface> inline U* get() { return getObject<U>().get(); }
    inline HydroponicsIdentity getId() const { return _obj ? _obj->getId() : (_keyStr ? HydroponicsIdentity(_keyStr) : HydroponicsIdentity(_key)); }
    inline Hydroponics_KeyType getKey() const { return _key; }
    inline String getKeyString() const { return _keyStr ? String(_keyStr) : getId().keyString; }

    inline operator bool() const { isResolved(); }
    inline HydroponicsObjInterface* operator->() { return get<HydroponicsObjInterface>(); }
    inline HydroponicsObjInterface* operator*() { return get<HydroponicsObjInterface>(); }

    HydroponicsDLinkObject &operator=(const HydroponicsDLinkObject &rhs);
    HydroponicsDLinkObject &operator=(HydroponicsIdentity rhs);
    HydroponicsDLinkObject &operator=(const char *rhs);
    template<class U> inline HydroponicsDLinkObject &operator=(shared_ptr<U> &rhs) { _key = (rhs ? rhs->getKey() : (Hydroponics_KeyType)-1); _obj = static_pointer_cast<HydroponicsObjInterface>(rhs); if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; } return *this; }
    inline HydroponicsDLinkObject &operator=(const HydroponicsObjInterface *rhs) { _key = (rhs ? rhs->getKey() : (Hydroponics_KeyType)-1); _obj = getSharedPtr(rhs); if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; } return *this; }

    inline bool operator==(const HydroponicsDLinkObject &rhs) const { return _key == rhs.getKey(); }
    inline bool operator==(const HydroponicsIdentity &rhs) const { return _key == rhs.key; }
    inline bool operator==(const char *rhs) const { return _key == stringHash(rhs); }
    template<class U> inline bool operator==(const shared_ptr<U> &rhs) const { return _key == (rhs ? rhs->getKey() : (Hydroponics_KeyType)-1); }
    inline bool operator==(const HydroponicsObjInterface *rhs) const { return _key == (rhs ? rhs->getKey() : (Hydroponics_KeyType)-1); }

    inline bool operator!=(const HydroponicsDLinkObject &rhs) const { return _key != rhs.getKey(); }
    inline bool operator!=(const HydroponicsIdentity &rhs) const { return _key != rhs.key; }
    inline bool operator!=(const char *rhs) const { return _key != stringHash(rhs); }
    template<class U> inline bool operator!=(const shared_ptr<U> &rhs) const { return _key != (rhs ? rhs->getKey() : (Hydroponics_KeyType)-1); }
    inline bool operator!=(const HydroponicsObjInterface *rhs) const { return _key != (rhs ? rhs->getKey() : (Hydroponics_KeyType)-1); }

protected:
    Hydroponics_KeyType _key;                               // Object key
    shared_ptr<HydroponicsObjInterface> _obj;               // Shared pointer to object
    const char *_keyStr;                                    // Copy of keystring (iff not resolved)
};

// Simple Attachment Point Base

// This attachment registers the parent object with the linked object's linkages upon
// dereference, and unregisters the parent object at time of destruction or reassignment.
class HydroponicsAttachment {
public:
    HydroponicsAttachment(HydroponicsObjInterface *parent);
    virtual ~HydroponicsAttachment();

    virtual void attachObject();
    virtual void detachObject();

    inline bool isId() const { return _obj.isId(); }
    inline bool isObject() const { return _obj.isObject(); }
    inline bool isResolved() const { return _obj.isResolved(); }
    inline bool needsResolved() const { return _obj.needsResolved(); }
    inline bool resolve() { return (bool)getObject<HydroponicsObjInterface>(); }

    template<class U> void setObject(U obj);
    template<class U = HydroponicsObjInterface> shared_ptr<U> getObject();

    template<class U = HydroponicsObjInterface> inline U* get() { return _obj.get<U>(); }
    inline HydroponicsIdentity getId() const { return _obj.getId(); }
    inline Hydroponics_KeyType getKey() const { return _obj.getKey(); }
    inline String getKeyString() const { return _obj.getKeyString(); }

    inline operator bool() const { return isResolved(); }
    inline HydroponicsObjInterface* operator->() { return get<HydroponicsObjInterface>(); }
    inline HydroponicsObjInterface* operator*() { return get<HydroponicsObjInterface>(); }

    inline HydroponicsAttachment &operator=(const HydroponicsIdentity &rhs) { setObject(rhs); return *this; }
    inline HydroponicsAttachment &operator=(const char *rhs) { setObject(HydroponicsIdentity(rhs)); return *this; }
    template<class U> inline HydroponicsAttachment &operator=(shared_ptr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroponicsAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }

    inline bool operator==(const HydroponicsIdentity &rhs) const { return _obj == rhs; }
    inline bool operator==(const char *rhs) { return *this == HydroponicsIdentity(rhs); }
    template<class U> inline bool operator==(const shared_ptr<U> &rhs) const { return _obj == rhs; }
    template<class U> inline bool operator==(const U *rhs) const { return _obj == rhs; }

    inline bool operator!=(const HydroponicsIdentity &rhs) const { return _obj != rhs; }
    inline bool operator!=(const char *rhs) { return *this != HydroponicsIdentity(rhs); }
    template<class U> inline bool operator!=(const shared_ptr<U> &rhs) const { return _obj != rhs; }
    template<class U> inline bool operator!=(const U *rhs) const { return _obj != rhs; }

protected:
    HydroponicsDLinkObject _obj;                            // Dynamic link object
    HydroponicsObjInterface *_parent;                       // Parent object pointer (strong due to reverse ownership)
};


// Signal Attachment Point
// This attachment registers the parent object with a Signal getter off the linked object
// upon dereference, and unregisters the parent object from the Signal at time of
// destruction or reassignment.
template<class ParameterType, int Slots = 8>
class HydroponicsSignalAttachment : public HydroponicsAttachment {
public:
    typedef Signal<ParameterType,Slots> &(HydroponicsObjInterface::*SignalGetterPtr)(void);
    typedef void (HydroponicsObjInterface::*HandleMethodPtr)(ParameterType);

    template<class T> HydroponicsSignalAttachment(HydroponicsObjInterface *parent, Signal<ParameterType,Slots> &(T::*signalGetter)(void));
    virtual ~HydroponicsSignalAttachment();

    virtual void attachObject() override;
    virtual void detachObject() override;

    template<class U> void setHandleMethod(MethodSlot<U,ParameterType> handleMethod);
    template<class U> inline void setHandleMethod(void (U::*handleMethodPtr)(ParameterType)) { setHandleMethod(MethodSlot<HydroponicsObjInterface,ParameterType>(_parent, (HandleMethodPtr)handleMethodPtr)); }

    inline HydroponicsSignalAttachment<ParameterType,Slots> &operator=(const HydroponicsIdentity &rhs) { setObject(rhs); return *this; }
    inline HydroponicsSignalAttachment<ParameterType,Slots> &operator=(const char *rhs) { setObject(HydroponicsIdentity(rhs)); return *this; }
    template<class U> inline HydroponicsSignalAttachment<ParameterType,Slots> &operator=(shared_ptr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroponicsSignalAttachment<ParameterType,Slots> &operator=(const U *rhs) { setObject(rhs); return *this; }

protected:
    SignalGetterPtr _signalGetter;                          // Signal getter method ptr
    MethodSlot<HydroponicsObjInterface,ParameterType> _handleMethod; // Handler method slot
};


// Sensor Measurement Attachment Point
// This attachment registers the parent object with a Sensor's new measurement Signal
// upon dereference, and unregisters the parent object from the Sensor at time of
// destruction or reassignment.
// Custom handle method will require a call into setMeasurement.
class HydroponicsSensorAttachment : public HydroponicsSignalAttachment<const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS> {
public:
    typedef void (HydroponicsObjInterface::*HandleMethodPtr)(const HydroponicsSingleMeasurement &measurement);

    HydroponicsSensorAttachment(HydroponicsObjInterface *parent, byte measurementRow = 0);

    virtual void attachObject() override;
    virtual void detachObject() override;

    void updateIfNeeded(bool poll = false);

    void setMeasurement(float value, Hydroponics_UnitsType units = Hydroponics_UnitsType_Undefined);
    void setMeasurement(HydroponicsSingleMeasurement measurement);
    void setMeasurementRow(byte measurementRow);
    void setMeasurementUnits(Hydroponics_UnitsType units, float convertParam = FLT_UNDEF);

    inline void setNeedsMeasurement() { _needsMeasurement = true; }
    inline bool needsMeasurement() { return _needsMeasurement; }

    inline const HydroponicsSingleMeasurement &getMeasurement(bool poll = false) { updateIfNeeded(poll); return _measurement; }
    inline uint16_t getMeasurementFrame(bool poll = false) { updateIfNeeded(poll); return _measurement.frame; }
    inline float getMeasurementValue(bool poll = false) { updateIfNeeded(poll); return _measurement.value; }
    inline Hydroponics_UnitsType getMeasurementUnits() const { return _measurement.units; }

    inline byte getMeasurementRow() const { return _measurementRow; }
    inline float getMeasurementConvertParam() const { return _convertParam; }

    inline shared_ptr<HydroponicsSensor> getObject() { return HydroponicsAttachment::getObject<HydroponicsSensor>(); }
    inline HydroponicsSensor* get() { return HydroponicsAttachment::get<HydroponicsSensor>(); }

    inline HydroponicsSensor* operator->() { return HydroponicsAttachment::getObject<HydroponicsSensor>().get(); }
    inline HydroponicsSensor* operator*() { return HydroponicsAttachment::getObject<HydroponicsSensor>().get(); }

    inline HydroponicsSensorAttachment &operator=(const HydroponicsIdentity &rhs) { setObject(rhs); return *this; }
    inline HydroponicsSensorAttachment &operator=(const char *rhs) { setObject(HydroponicsIdentity(rhs)); return *this; }
    template<class U> inline HydroponicsSensorAttachment &operator=(shared_ptr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroponicsSensorAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }

protected:
    HydroponicsSingleMeasurement _measurement;              // Local measurement
    byte _measurementRow;                                   // Measurement row
    float _convertParam;                                    // Convert param (default: FLT_UNDEF)
    bool _needsMeasurement;                                 // Stale measurement tracking flag

    void handleMeasurement(const HydroponicsMeasurement *measurement);
};


// Trigger State Attachment Point
// This attachment registers the parent object with a Triggers's trigger Signal
// upon dereference, and unregisters the parent object from the Trigger at time of
// destruction or reassignment.
class HydroponicsTriggerAttachment  : public HydroponicsSignalAttachment<Hydroponics_TriggerState, HYDRUINO_TRIGGER_STATE_SLOTS> {
public:
    typedef void (HydroponicsObjInterface::*HandleMethodPtr)(Hydroponics_TriggerState triggerState);

    HydroponicsTriggerAttachment(HydroponicsObjInterface *parent);

    inline void updateIfNeeded() { if (resolve()) { _obj->update(); } }

    inline Hydroponics_TriggerState getTriggerState();

    inline shared_ptr<HydroponicsTrigger> getObject() { return HydroponicsAttachment::getObject<HydroponicsTrigger>(); }
    inline HydroponicsTrigger* get() { return HydroponicsAttachment::get<HydroponicsTrigger>(); }

    inline HydroponicsTrigger* operator->() { return HydroponicsAttachment::getObject<HydroponicsTrigger>().get(); }
    inline HydroponicsTrigger* operator*() { return HydroponicsAttachment::getObject<HydroponicsTrigger>().get(); }

    inline HydroponicsTriggerAttachment &operator=(const HydroponicsIdentity &rhs) { setObject(rhs); return *this; }
    inline HydroponicsTriggerAttachment &operator=(const char *rhs) { setObject(HydroponicsIdentity(rhs)); return *this; }
    template<class U> inline HydroponicsTriggerAttachment &operator=(shared_ptr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroponicsTriggerAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }
};


// Balancer State Attachment Point
// This attachment registers the parent object with a Balancer's balance Signal
// upon dereference, and unregisters the parent object from the Balancer at time of
// destruction or reassignment.
class HydroponicsBalancerAttachment : public HydroponicsSignalAttachment<Hydroponics_BalancerState, HYDRUINO_BALANCER_STATE_SLOTS> {
public:
    typedef void (HydroponicsObjInterface::*HandleMethodPtr)(Hydroponics_BalancerState balancerState);

    HydroponicsBalancerAttachment(HydroponicsObjInterface *parent);

    inline void updateIfNeeded() { if (resolve()) { _obj->update(); } }

    inline Hydroponics_BalancerState getBalancerState();

    inline shared_ptr<HydroponicsBalancer> getObject() { return HydroponicsAttachment::getObject<HydroponicsBalancer>(); }
    inline HydroponicsBalancer* get() { return HydroponicsAttachment::get<HydroponicsBalancer>(); }

    inline HydroponicsBalancer* operator->() { return HydroponicsAttachment::getObject<HydroponicsBalancer>().get(); }
    inline HydroponicsBalancer* operator*() { return HydroponicsAttachment::getObject<HydroponicsBalancer>().get(); }

    inline HydroponicsBalancerAttachment &operator=(const HydroponicsIdentity &rhs) { setObject(rhs); return *this; }
    inline HydroponicsBalancerAttachment &operator=(const char *rhs) { setObject(HydroponicsIdentity(rhs)); return *this; }
    template<class U> inline HydroponicsBalancerAttachment &operator=(shared_ptr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroponicsBalancerAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }
};

#endif // /ifndef HydroponicsAttachments_H
