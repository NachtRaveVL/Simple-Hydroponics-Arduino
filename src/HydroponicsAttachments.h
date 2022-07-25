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
    template<class U> HydroponicsDLinkObject(shared_ptr<U> obj);
    template<class U> HydroponicsDLinkObject(const U *obj);
    template<class U> HydroponicsDLinkObject(const HydroponicsDLinkObject<U> &obj);

    inline bool isId() const { return !_obj; }
    inline bool isObject() const { return (bool)_obj; }
    inline bool isResolved() const { return isObject(); }
    inline bool needsResolved() const { return isId() && (bool)_id; }
    inline bool resolveIfNeeded() { return needsResolved() && (bool)getObject(); }

    shared_ptr<T> getObject();

    inline T* get() { return getObject().get(); }
    inline const HydroponicsIdentity &getId() const { return _id; }
    inline Hydroponics_KeyType getKey() const { return getId().key; }
    inline const String &getKeyString() const { return getId().keyString; }

    inline operator bool() const { return (bool)_obj; }
    inline T* operator->() { return get(); }
    inline T* operator*() { return get(); }

    template<class U> inline HydroponicsDLinkObject<T> &operator=(const HydroponicsDLinkObject<U> &rhs) { _id = rhs.id; _obj = static_pointer_cast<T>(rhs.obj); return *this; }
    inline HydroponicsDLinkObject<T> &operator=(const HydroponicsIdentity &rhs) { _id = rhs; _obj = nullptr; return *this; }
    template<class U> inline HydroponicsDLinkObject<T> &operator=(shared_ptr<U> &rhs) { _id = (rhs ? rhs->getId() : HydroponicsIdentity()); _obj = static_pointer_cast<T>(rhs); return *this; }
    template<class U> inline HydroponicsDLinkObject<T> &operator=(const U *rhs);

    template<class U> inline bool operator==(const HydroponicsDLinkObject<U> &rhs) const { return _id.key == rhs.getKey(); }
    inline bool operator==(const HydroponicsIdentity &rhs) const { return _id.key == rhs.key; }
    template<class U> inline bool operator==(const shared_ptr<U> &rhs) const { return _id.key == (rhs ? rhs->getKey() : (Hydroponics_KeyType)-1); }
    template<class U> inline bool operator==(const U *rhs) const { return _id.key == (rhs ? rhs->getKey() : (Hydroponics_KeyType)-1); }

    template<class U> inline bool operator!=(const HydroponicsDLinkObject<U> &rhs) const { return _id.key != rhs.getKey(); }
    inline bool operator!=(const HydroponicsIdentity &rhs) const { return _id.key != rhs.key; }
    template<class U> inline bool operator!=(const shared_ptr<U> &rhs) const { return _id.key != (rhs ? rhs->getKey() : (Hydroponics_KeyType)-1); }
    template<class U> inline bool operator!=(const U *rhs) const { return _id.key != (rhs ? rhs->getKey() : (Hydroponics_KeyType)-1); }

protected:
    HydroponicsIdentity _id;                                // Object identity
    shared_ptr<T> _obj;                                     // Shared pointer to object
};


// Basic Attachment Point
// This attachment registers the parent object with the linked object's linkages upon
// dereference, and unregisters the parent object at time of destruction or reassignment.
template<class T>
class HydroponicsAttachment {
public:
    HydroponicsAttachment(HydroponicsObject *parent);
    HydroponicsAttachment(HydroponicsObject *parent, const HydroponicsIdentity &id);
    HydroponicsAttachment(HydroponicsObject *parent, const char *idKeyStr);
    HydroponicsAttachment(HydroponicsObject *parent, shared_ptr<T> obj);
    HydroponicsAttachment(HydroponicsObject *parent, const T *obj);
    virtual ~HydroponicsAttachment();

    inline bool isId() const { return _obj.isId(); }
    inline bool isObject() const { return _obj.isObject(); }
    inline bool isResolved() const { return _obj.isResolved(); }
    inline bool needsResolved() const { return _obj.needsResolved(); }
    inline bool resolveIfNeeded() { return needsResolved() && (bool)getObject(); }

    inline shared_ptr<T> getObject() { if (needsResolved()) { attachObject(); } return isResolved() ? _obj.getObject() : nullptr; }

    virtual void attachObject();
    virtual void detachObject();

    inline T* get() { return getObject().get(); }
    inline const HydroponicsIdentity &getId() const { return _obj.getId();  }
    inline Hydroponics_KeyType getKey() const { return _obj.getKey(); }
    inline const String &getKeyString() const { return _obj.getKeyString(); }

    inline operator bool() const { return (bool)_obj; }
    inline T* operator->() { return getObject().get(); }
    inline T* operator*() { return getObject().get(); }

    template<class U> inline HydroponicsAttachment<T> &operator=(const HydroponicsAttachment<U> &rhs) { if (_obj != rhs._obj) { detachObject(); _obj = rhs._obj; if (_obj) { attachObject(); } } return *this; }
    template<class U> inline HydroponicsAttachment<T> &operator=(const HydroponicsDLinkObject<U> &rhs) { if (_obj != rhs) { detachObject(); _obj = rhs; if (_obj) { attachObject(); } } return *this; }
    inline HydroponicsAttachment<T> &operator=(const HydroponicsIdentity &rhs) { if (_obj != rhs) { detachObject(); _obj = rhs; if (_obj) { attachObject(); } } return *this; }
    template<class U> inline HydroponicsAttachment<T> &operator=(shared_ptr<U> rhs) { if (_obj != rhs) { detachObject(); _obj = rhs; if (_obj) { attachObject(); } } return *this; }
    template<class U> inline HydroponicsAttachment<T> &operator=(const U *rhs) { if (_obj != rhs) { detachObject(); _obj = rhs; if (_obj) { attachObject(); } } return *this; }

    template<class U> inline bool operator==(const HydroponicsAttachment<U> &rhs) const { return _obj == rhs._obj; }
    template<class U> inline bool operator==(const HydroponicsDLinkObject<U> &rhs) const { return _obj == rhs; }
    inline bool operator==(const HydroponicsIdentity &rhs) const { return _obj == rhs; }
    template<class U> inline bool operator==(const shared_ptr<U> &rhs) const { return _obj == rhs; }
    template<class U> inline bool operator==(const U *rhs) const { return _obj == rhs; }

    template<class U> inline bool operator!=(const HydroponicsAttachment<U> &rhs) const { return _obj != rhs._obj; }
    template<class U> inline bool operator!=(const HydroponicsDLinkObject<U> &rhs) const { return _obj != rhs; }
    inline bool operator!=(const HydroponicsIdentity &rhs) const { return _obj != rhs; }
    template<class U> inline bool operator!=(const shared_ptr<U> &rhs) const { return _obj != rhs; }
    template<class U> inline bool operator!=(const U *rhs) const { return _obj != rhs; }

protected:
    HydroponicsDLinkObject<T> _obj;                         // Dynamic link object
    HydroponicsObject *_parent;                             // Parent object pointer (strong)
};


// Signal Attachment Point
// This attachment registers the parent object with a Signal getter off the linked object
// upon dereference, and unregisters the parent object from the Signal at time of
// destruction or reassignment.
template<class T, class ParameterType, int Slots = 8>
class HydroponicsSignalAttachment : public HydroponicsAttachment<T> {
public:
    typedef Signal<ParameterType> &(T::*SignalGetterPtr)(void);

    template<class U> HydroponicsSignalAttachment(HydroponicsObject *parent, SignalGetterPtr signalGetter, MethodSlot<U,ParameterType> handleMethod);
    template<class U> HydroponicsSignalAttachment(HydroponicsObject *parent, const HydroponicsIdentity &id, SignalGetterPtr signalGetter, MethodSlot<U,ParameterType> handleMethod);
    template<class U> HydroponicsSignalAttachment(HydroponicsObject *parent, const char *idKeyStr, SignalGetterPtr signalGetter, MethodSlot<U,ParameterType> handleMethod);
    template<class U> HydroponicsSignalAttachment(HydroponicsObject *parent, shared_ptr<T> obj, SignalGetterPtr signalGetter, MethodSlot<U,ParameterType> handleMethod);
    template<class U> HydroponicsSignalAttachment(HydroponicsObject *parent, const T *obj, SignalGetterPtr signalGetter, MethodSlot<U,ParameterType> handleMethod);
    virtual ~HydroponicsSignalAttachment();

    virtual void attachObject() override;
    virtual void detachObject() override;

protected:
    SignalGetterPtr _signalGetter;                          // Signal getter method ptr
    MethodSlot<HydroponicsObject,ParameterType> _handleMethod; // Handler method slot
};


// Sensor Measurement Attachment Point
// This attachment registers the parent object with a Sensor's new measurement Signal
// upon dereference, and unregisters the parent object from the Sensor at time of
// destruction or reassignment.
class HydroponicsSensorAttachment : public HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS> {
public:
    typedef void (HydroponicsObject::*ProcessMethodPtr)(const HydroponicsMeasurement *measurement);
    typedef void (HydroponicsObject::*UpdateMethodPtr)(const HydroponicsSingleMeasurement *measurement);

    HydroponicsSensorAttachment(HydroponicsObject *parent, Hydroponics_PositionIndex measurementRow = 0);
    HydroponicsSensorAttachment(HydroponicsObject *parent, const HydroponicsIdentity &sensorId, Hydroponics_PositionIndex measurementRow = 0);
    HydroponicsSensorAttachment(HydroponicsObject *parent, const char *sensorKeyStr, Hydroponics_PositionIndex measurementRow = 0);
    HydroponicsSensorAttachment(HydroponicsObject *parent, shared_ptr<HydroponicsSensor> sensor, Hydroponics_PositionIndex measurementRow = 0);
    HydroponicsSensorAttachment(HydroponicsObject *parent, const HydroponicsSensor *sensor, Hydroponics_PositionIndex measurementRow = 0);
    virtual ~HydroponicsSensorAttachment();

    virtual void attachObject() override;
    virtual void detachObject() override;

    void updateMeasurementIfNeeded(bool resolveIfNeeded = true);

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

    inline const HydroponicsSingleMeasurement &getMeasurement(bool resolveIfNeeded = true) { updateMeasurementIfNeeded(resolveIfNeeded); return _measurement; }
    inline uint16_t getMeasurementFrame(bool resolveIfNeeded = true) { updateMeasurementIfNeeded(resolveIfNeeded); return _measurement.frame; }
    inline float getMeasurementValue(bool resolveIfNeeded = true) { updateMeasurementIfNeeded(resolveIfNeeded); return _measurement.value; }
    inline Hydroponics_UnitsType getMeasurementUnits(bool resolveIfNeeded = true) { updateMeasurementIfNeeded(resolveIfNeeded); return _measurement.units; }

    inline Hydroponics_PositionIndex getMeasurementRow() const { return _measurementRow; }
    inline float getMeasurementConvertParam() const { return _convertParam; }

protected:
    HydroponicsSingleMeasurement _measurement;              // Local measurement
    Hydroponics_PositionIndex _measurementRow;              // Measurement row
    float _convertParam;                                    // Convert param (default: FLT_UNDEF)
    bool _needsMeasurement;                                 // Measurement data old tracking flag
    ProcessMethodPtr _processMethod;                        // Custom process method
    UpdateMethodPtr _updateMethod;                          // Custom update method

    void handleMeasurement(const HydroponicsMeasurement *measurement);
};

#endif // /ifndef HydroponicsAttachments_H
