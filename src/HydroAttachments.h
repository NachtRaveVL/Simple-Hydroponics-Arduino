/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Attachment Points
*/

#ifndef HydroAttachments_H
#define HydroAttachments_H

class HydroDLinkObject;
class HydroAttachment;
template<class ParameterType, int Slots> class HydroSignalAttachment;
class HydroSensorAttachment;
class HydroTriggerAttachment;
class HydroBalancerAttachment;

#include "Hydruino.h"
#include "HydroObject.h"
#include "HydroMeasurements.h"

// forward decls
extern Hydro_KeyType stringHash(String);
extern String addressToString(uintptr_t);

// Delay/Dynamic Loaded/Linked Object Reference
// Simple class for delay loading objects that get references to others during system
// load. T should be a derived class of HydroObjInterface, with getId() method.
class HydroDLinkObject {
public:
    HydroDLinkObject();
    HydroDLinkObject(const HydroDLinkObject &obj);
    virtual ~HydroDLinkObject();

    inline bool isUnresolved() const { return !_obj; }
    inline bool isResolved() const { return (bool)_obj; }
    inline bool needsResolved() const { return isUnresolved() && _key != (Hydro_KeyType)-1; }
    inline bool resolve() { return isResolved() || (bool)getObject(); }
    void unresolve();

    template<class U> inline void setObject(U obj) { (*this) = obj; }
    template<class U = HydroObjInterface> inline SharedPtr<U> getObject() { return reinterpret_pointer_cast<U>(_getObject()); }
    template<class U = HydroObjInterface> inline U* get() { return getObject<U>().get(); }

    inline HydroIdentity getId() const { return _obj ? _obj->getId() : (_keyStr ? HydroIdentity(_keyStr) : HydroIdentity(_key)); }
    inline Hydro_KeyType getKey() const { return _key; }
    inline String getKeyString() const { return _keyStr ? String(_keyStr) : (_obj ? _obj->getKeyString() : addressToString((uintptr_t)_key)); }

    inline operator bool() const { return isResolved(); }
    inline HydroObjInterface *operator->() { return get(); }

    inline HydroDLinkObject &operator=(HydroIdentity rhs);
    inline HydroDLinkObject &operator=(const char *rhs);
    template<class U> inline HydroDLinkObject &operator=(SharedPtr<U> &rhs);
    inline HydroDLinkObject &operator=(const HydroObjInterface *rhs);
    inline HydroDLinkObject &operator=(nullptr_t) { return this->operator=((HydroObjInterface *)nullptr); }

    inline bool operator==(const HydroIdentity &rhs) const { return _key == rhs.key; }
    inline bool operator==(const char *rhs) const { return _key == stringHash(rhs); }
    template<class U> inline bool operator==(const SharedPtr<U> &rhs) const { return _key == (rhs ? rhs->getKey() : (Hydro_KeyType)-1); }
    inline bool operator==(const HydroObjInterface *rhs) const { return _key == (rhs ? rhs->getKey() : (Hydro_KeyType)-1); }
    inline bool operator==(nullptr_t) const { return _key == (Hydro_KeyType)-1; }

protected:
    Hydro_KeyType _key;                                     // Object key
    SharedPtr<HydroObjInterface> _obj;                      // Shared pointer to object
    const char *_keyStr;                                    // Copy of id.keyString (if not resolved, or unresolved)

private:
    SharedPtr<HydroObjInterface> _getObject();
    friend class Hydruino;
    friend class HydroAttachment;
};

// Simple Attachment Point Base
// This attachment registers the parent object with the linked object's linkages upon
// dereference, and unregisters the parent object at time of destruction or reassignment.
class HydroAttachment : public HydroSubObject {
public:
    HydroAttachment(HydroObjInterface *parent);
    virtual ~HydroAttachment();

    virtual void attachObject();
    virtual void detachObject();

    inline bool isUnresolved() const { return !_obj; }
    inline bool isResolved() const { return (bool)_obj; }
    inline bool needsResolved() const { return _obj.needsResolved(); }
    inline bool resolve() { return isResolved() || (bool)getObject(); }

    template<class U> void setObject(U obj);
    template<class U = HydroObjInterface> SharedPtr<U> getObject();
    template<class U = HydroObjInterface> inline U* get() { return getObject<U>().get(); }

    inline HydroIdentity getId() const { return _obj.getId(); }
    inline Hydro_KeyType getKey() const { return _obj.getKey(); }
    inline String getKeyString() const { return _obj.getKeyString(); }

    inline operator bool() const { return isResolved(); }
    inline HydroObjInterface* operator->() { return get<HydroObjInterface>(); }

    inline HydroAttachment &operator=(const HydroIdentity &rhs) { setObject(rhs); return *this; }
    inline HydroAttachment &operator=(const char *rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroAttachment &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }

    inline bool operator==(const HydroIdentity &rhs) const { return _obj == rhs; }
    inline bool operator==(const char *rhs) { return *this == HydroIdentity(rhs); }
    template<class U> inline bool operator==(const SharedPtr<U> &rhs) const { return _obj == rhs; }
    template<class U> inline bool operator==(const U *rhs) const { return _obj == rhs; }

protected:
    HydroDLinkObject _obj;                                  // Dynamic link object
    HydroObjInterface *_parent;                             // Parent object pointer (strong due to reverse ownership)
};


// Signal Attachment Point
// This attachment registers the parent object with a Signal getter off the linked object
// upon dereference, and unregisters the parent object from the Signal at time of
// destruction or reassignment.
template<class ParameterType, int Slots = 8>
class HydroSignalAttachment : public HydroAttachment {
public:
    typedef Signal<ParameterType,Slots> &(HydroObjInterface::*SignalGetterPtr)(void);
    typedef void (HydroObjInterface::*HandleMethodPtr)(ParameterType);
    typedef MethodSlot<HydroObjInterface,ParameterType> *HandleMethodSlotPtr;

    template<class U> HydroSignalAttachment(HydroObjInterface *parent, Signal<ParameterType,Slots> &(U::*signalGetter)(void));
    HydroSignalAttachment(const HydroSignalAttachment<ParameterType,Slots> &attachment);
    virtual ~HydroSignalAttachment();

    virtual void attachObject() override;
    virtual void detachObject() override;

    template<class U> void setHandleMethod(MethodSlot<U,ParameterType> handleMethod);
    template<class U> inline void setHandleMethod(void (U::*handleMethodPtr)(ParameterType)) { setHandleMethod<U>(MethodSlot<U,ParameterType>(reinterpret_cast<U *>(_parent), handleMethodPtr)); }

    inline HydroSignalAttachment<ParameterType,Slots> &operator=(const HydroIdentity &rhs) { setObject(rhs); return *this; }
    inline HydroSignalAttachment<ParameterType,Slots> &operator=(const char *rhs) { setObject(HydroIdentity(rhs)); return *this; }
    template<class U> inline HydroSignalAttachment<ParameterType,Slots> &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroSignalAttachment<ParameterType,Slots> &operator=(const U *rhs) { setObject(rhs); return *this; }

protected:
    SignalGetterPtr _signalGetter;                          // Signal getter method ptr (weak)
    HandleMethodSlotPtr _handleMethod;                      // Handler method slot (owned)
};


// Sensor Measurement Attachment Point
// This attachment registers the parent object with a Sensor's new measurement Signal
// upon dereference, and unregisters the parent object from the Sensor at time of
// destruction or reassignment.
// Custom handle method will require a call into setMeasurement.
class HydroSensorAttachment : public HydroSignalAttachment<const HydroMeasurement *, HYDRO_SENSOR_MEASUREMENT_SLOTS> {
public:
    typedef void (HydroObjInterface::*HandleMethodPtr)(const HydroMeasurement *);

    HydroSensorAttachment(HydroObjInterface *parent, uint8_t measurementRow = 0);

    virtual void attachObject() override;
    virtual void detachObject() override;

    inline void updateIfNeeded(bool poll = false);

    void setMeasurement(float value, Hydro_UnitsType units = Hydro_UnitsType_Undefined);
    void setMeasurement(HydroSingleMeasurement measurement);
    void setMeasurementRow(uint8_t measurementRow);
    void setMeasurementUnits(Hydro_UnitsType units, float convertParam = FLT_UNDEF);

    inline void setNeedsMeasurement() { _needsMeasurement = true; }
    inline bool needsMeasurement() { return _needsMeasurement; }

    inline const HydroSingleMeasurement &getMeasurement(bool poll = false) { updateIfNeeded(poll); return _measurement; }
    inline uint16_t getMeasurementFrame(bool poll = false) { updateIfNeeded(poll); return _measurement.frame; }
    inline float getMeasurementValue(bool poll = false) { updateIfNeeded(poll); return _measurement.value; }
    inline Hydro_UnitsType getMeasurementUnits() const { return _measurement.units; }

    inline uint8_t getMeasurementRow() const { return _measurementRow; }
    inline float getMeasurementConvertParam() const { return _convertParam; }

    inline SharedPtr<HydroSensor> getObject() { return HydroAttachment::getObject<HydroSensor>(); }
    inline HydroSensor *get() { return HydroAttachment::get<HydroSensor>(); }

    inline HydroSensor &operator*() { return *HydroAttachment::getObject<HydroSensor>().get(); }
    inline HydroSensor *operator->() { return HydroAttachment::getObject<HydroSensor>().get(); }

    inline HydroSensorAttachment &operator=(const HydroIdentity &rhs) { setObject(rhs); return *this; }
    inline HydroSensorAttachment &operator=(const char *rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroSensorAttachment &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroSensorAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }

protected:
    HydroSingleMeasurement _measurement;                    // Local measurement (converted to measure units)
    uint8_t _measurementRow;                                // Measurement row
    float _convertParam;                                    // Convert param (default: FLT_UNDEF)
    bool _needsMeasurement;                                 // Stale measurement tracking flag

    void handleMeasurement(const HydroMeasurement *measurement);
};


// Trigger State Attachment Point
// This attachment registers the parent object with a Triggers's trigger Signal
// upon dereference, and unregisters the parent object from the Trigger at time of
// destruction or reassignment.
class HydroTriggerAttachment  : public HydroSignalAttachment<Hydro_TriggerState, HYDRO_TRIGGER_STATE_SLOTS> {
public:
    typedef void (HydroObjInterface::*HandleMethodPtr)(Hydro_TriggerState);

    HydroTriggerAttachment(HydroObjInterface *parent);

    inline void updateIfNeeded();

    inline Hydro_TriggerState getTriggerState();

    inline SharedPtr<HydroTrigger> getObject() { return HydroAttachment::getObject<HydroTrigger>(); }
    inline HydroTrigger *get() { return HydroAttachment::get<HydroTrigger>(); }

    inline HydroTrigger &operator*() { return *HydroAttachment::getObject<HydroTrigger>().get(); }
    inline HydroTrigger *operator->() { return HydroAttachment::getObject<HydroTrigger>().get(); }

    inline HydroTriggerAttachment &operator=(const HydroIdentity &rhs) { setObject(rhs); return *this; }
    inline HydroTriggerAttachment &operator=(const char *rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroTriggerAttachment &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroTriggerAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }
};


// Balancer State Attachment Point
// This attachment registers the parent object with a Balancer's balance Signal
// upon dereference, and unregisters the parent object from the Balancer at time of
// destruction or reassignment.
class HydroBalancerAttachment : public HydroSignalAttachment<Hydro_BalancerState, HYDRO_BALANCER_STATE_SLOTS> {
public:
    typedef void (HydroObjInterface::*HandleMethodPtr)(Hydro_BalancerState);

    HydroBalancerAttachment(HydroObjInterface *parent);

    inline void updateIfNeeded();

    inline Hydro_BalancerState getBalancerState();

    inline SharedPtr<HydroBalancer> getObject() { return HydroAttachment::getObject<HydroBalancer>(); }
    inline HydroBalancer *get() { return HydroAttachment::get<HydroBalancer>(); }

    inline HydroBalancer &operator*() { return *HydroAttachment::getObject<HydroBalancer>().get(); }
    inline HydroBalancer *operator->() { return HydroAttachment::getObject<HydroBalancer>().get(); }

    inline HydroBalancerAttachment &operator=(const HydroIdentity &rhs) { setObject(rhs); return *this; }
    inline HydroBalancerAttachment &operator=(const char *rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroBalancerAttachment &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroBalancerAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }
};

#endif // /ifndef HydroAttachments_H
