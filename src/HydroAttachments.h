/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Attachment Points
*/

#ifndef HydroAttachments_H
#define HydroAttachments_H

class HydroDLinkObject;
class HydroAttachment;
template<class ParameterType, int Slots> class HydroSignalAttachment;
class HydroActuatorAttachment;
class HydroSensorAttachment;
class HydroTriggerAttachment;
class HydroBalancerAttachment;

#include "Hydruino.h"
#include "HydroObject.h"
#include "HydroMeasurements.h"
#include "HydroActivation.h"

// Delay/Dynamic Loaded/Linked Object Reference
// Simple class for delay loading objects that get references to others during object
// load. T should be a derived class of HydroObjInterface, with getId() method.
class HydroDLinkObject {
public:
    HydroDLinkObject();
    HydroDLinkObject(const HydroDLinkObject &obj);
    virtual ~HydroDLinkObject();

    inline bool isUnresolved() const { return !_obj; }
    inline bool isResolved() const { return (bool)_obj; }
    inline bool needsResolved() const { return isUnresolved() && isSet(); }
    inline bool resolve() { return isResolved() || (bool)getObject(); }
    void unresolve();
    template<class U> inline void unresolveIf(U obj) { if (operator==(obj)) { unresolve(); } }

    template<class U> inline void setObject(U obj) { operator=(obj); }
    template<class U = HydroObjInterface> inline SharedPtr<U> getObject() { return reinterpret_pointer_cast<U>(resolveObject()); }
    template<class U = HydroObjInterface> inline U *get() { return getObject<U>().get(); }

    inline HydroIdentity getId() const { return _obj ? _obj->getId() : (_keyStr ? HydroIdentity(_keyStr) : HydroIdentity(_key)); }
    inline hkey_t getKey() const { return _key; }
    inline String getKeyString() const { return _keyStr ? String(_keyStr) : (_obj ? _obj->getKeyString() : addressToString((uintptr_t)_key)); }
    inline bool isSet() const { return _key != hkey_none; }

    inline operator bool() const { return isResolved(); }
    inline HydroObjInterface *operator->() { return get(); }

    inline HydroDLinkObject &operator=(HydroIdentity rhs);
    inline HydroDLinkObject &operator=(const char *rhs);
    template<class U> inline HydroDLinkObject &operator=(SharedPtr<U> &rhs);
    inline HydroDLinkObject &operator=(const HydroObjInterface *rhs);
    inline HydroDLinkObject &operator=(const HydroAttachment *rhs);
    inline HydroDLinkObject &operator=(nullptr_t) { return operator=((HydroObjInterface *)nullptr); }

    inline bool operator==(const HydroIdentity &rhs) const { return _key == rhs.key; }
    inline bool operator==(const char *rhs) const { return _key == stringHash(rhs); }
    template<class U> inline bool operator==(const SharedPtr<U> &rhs) const { return _key == (rhs ? rhs->getKey() : hkey_none); }
    inline bool operator==(const HydroObjInterface *rhs) const { return _key == (rhs ? rhs->getKey() : hkey_none); }
    inline bool operator==(nullptr_t) const { return _key == hkey_none; }

protected:
    hkey_t _key;                                            // Object key
    SharedPtr<HydroObjInterface> _obj;                      // Shared pointer to object
    const char *_keyStr;                                    // Copy of id.keyString (if not resolved, or unresolved)

private:
    SharedPtr<HydroObjInterface> resolveObject();
    friend class Hydruino;
    friend class HydroAttachment;
};

// Simple Attachment Point Base
// This attachment registers the parent object with the linked object's linkages upon
// dereference / unregisters the parent object at time of destruction or reassignment.
class HydroAttachment : public HydroSubObject {
public:
    HydroAttachment(HydroObjInterface *parent = nullptr);
    HydroAttachment(const HydroAttachment &attachment);
    virtual ~HydroAttachment();

    // Attaches object and any relevant signaling mechanisms. Derived classes should call base class's method first.
    virtual void attachObject();
    // Detaches object from any relevant signaling mechanism. Derived classes should call base class's method last.
    virtual void detachObject();

    // Attachment updater. Overridden by derived classes. May only update owned sub-objects (main objects are owned/updated by run system).
    virtual void updateIfNeeded(bool poll = false);

    inline bool isUnresolved() const { return !_obj; }
    inline bool isResolved() const { return (bool)_obj; }
    inline bool needsResolved() const { return _obj.needsResolved(); }
    inline bool resolve() { return isResolved() || (bool)getObject(); }
    inline void unresolve() { _obj.unresolve(); } 
    template<class U> inline void unresolveIf(U obj) { _obj.unresolveIf(obj); }

    template<class U> void setObject(U obj, bool modify = true);
    template<class U> inline void initObject(U obj) { setObject(obj, false); }
    template<class U = HydroObjInterface> SharedPtr<U> getObject();
    template<class U = HydroObjInterface> inline U *get() { return getObject<U>().get(); }

    virtual void setParent(HydroObjInterface *parent) override;

    inline HydroIdentity getId() const { return _obj.getId(); }
    inline hkey_t getKey() const { return _obj.getKey(); }
    inline String getKeyString() const { return _obj.getKeyString(); }
    inline bool isSet() const { return _obj.isSet(); }
    virtual SharedPtr<HydroObjInterface> getSharedPtrFor(const HydroObjInterface *obj) const override;

    inline operator bool() const { return isResolved(); }
    inline HydroObjInterface *operator->() { return get<HydroObjInterface>(); }

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
};


// Signal Attachment Point
// This attachment registers the parent object with a signal getter off the linked object
// upon resolvement / unregisters the parent object from the signal at time of
// destruction or reassignment.
template<class ParameterType, int Slots = 8>
class HydroSignalAttachment : public HydroAttachment {
public:
    typedef Signal<ParameterType,Slots> &(HydroObjInterface::*SignalGetterPtr)(void);

    template<class U> HydroSignalAttachment(HydroObjInterface *parent = nullptr, Signal<ParameterType,Slots> &(U::*signalGetter)(void) = nullptr);
    HydroSignalAttachment(const HydroSignalAttachment<ParameterType,Slots> &attachment);
    virtual ~HydroSignalAttachment();

    virtual void attachObject() override;
    virtual void detachObject() override;

    // Sets the signal handler getter method to use
    template<class U> void setSignalGetter(Signal<ParameterType,Slots> &(U::*signalGetter)(void));

    // Sets a handle slot to run when attached signal fires
    void setHandleSlot(const Slot<ParameterType> &handleSlot);
    inline void setHandleFunction(void (*handleFunctionPtr)(ParameterType)) { setHandleSlot(FunctionSlot<ParameterType>(handleFunctionPtr)); }
    template<class U, class V = U> inline void setHandleMethod(void (U::*handleMethodPtr)(ParameterType), V *handleClassInst = nullptr) { setHandleSlot(MethodSlot<V,ParameterType>(handleClassInst ? handleClassInst : static_cast<V *>(_parent), handleMethodPtr)); }

    inline HydroSignalAttachment<ParameterType,Slots> &operator=(const HydroIdentity &rhs) { setObject(rhs); return *this; }
    inline HydroSignalAttachment<ParameterType,Slots> &operator=(const char *rhs) { setObject(HydroIdentity(rhs)); return *this; }
    template<class U> inline HydroSignalAttachment<ParameterType,Slots> &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroSignalAttachment<ParameterType,Slots> &operator=(const U *rhs) { setObject(rhs); return *this; }

protected:
    SignalGetterPtr _signalGetter;                          // Signal getter method ptr (weak)
    Slot<ParameterType> *_handleSlot;                       // Handle slot (owned)
};


// Actuator Attachment Point
// This attachment interfaces with actuator activation handles for actuator control, and
// registers the parent object with an actuator upon resolvement / unregisters the parent
// object from the actuator at time of destruction or reassignment.
class HydroActuatorAttachment : public HydroSignalAttachment<HydroActuator *, HYDRO_ACTUATOR_SIGNAL_SLOTS> {
public:
    HydroActuatorAttachment(HydroObjInterface *parent = nullptr);
    HydroActuatorAttachment(const HydroActuatorAttachment &attachment);
    virtual ~HydroActuatorAttachment();

    // Updates with actuator activation handle. Does not call actuator's update() (handled by system).
    virtual void updateIfNeeded(bool poll = false) override;

    // A rate multiplier is used to adjust either the intensity or duration of activations,
    // which depends on whenever they operate in binary mode (on/off) or variably (ranged).
    inline void setRateMultiplier(float rateMultiplier) { if (!isFPEqual(_rateMultiplier, rateMultiplier)) { _rateMultiplier = rateMultiplier; applySetup(); } }
    inline float getRateMultiplier() const { return _rateMultiplier; }

    // Activations are set up first by calling one of these methods. This configures the
    // direction, intensity, duration, and any run flags that the actuator will operate
    // upon once enabled, pending any rate offsetting. These methods are re-entrant.
    // The most recently used setup values are used for repeat activations.
    inline void setupActivation(const HydroActivation &activation) { _actSetup = activation; applySetup(); }
    inline void setupActivation(const HydroActivationHandle &handle) { setupActivation(handle.activation); }
    inline void setupActivation(Hydro_DirectionMode direction, float intensity = 1.0f, millis_t duration = -1, bool force = false) { setupActivation(HydroActivation(direction, intensity, duration, (force ? Hydro_ActivationFlags_Forced : Hydro_ActivationFlags_None))); }
    inline void setupActivation(millis_t duration = -1, bool force = false) { setupActivation(HydroActivation(Hydro_DirectionMode_Forward, 1.0f, duration, (force ? Hydro_ActivationFlags_Forced : Hydro_ActivationFlags_None))); }
    // These activation methods take a variable value that gets transformed by any user
    // curvature calibration data before being used, assuming units to be the same. It is
    // otherwise assumed the value is a normalized driving intensity ([0,1] or [-1,1]).
    void setupActivation(float value, millis_t duration = -1, bool force = false);
    inline void setupActivation(const HydroSingleMeasurement &measurement, millis_t duration = -1, bool force = false) { setupActivation(measurement.value, duration, force); }

    // Gets what units are expected to be used in setupActivation() methods
    inline Hydro_UnitsType getActivationUnits();

    // Enables activation handle with current setup, if not already active.
    // Repeat activations will reuse most recent setupActivation() values.
    void enableActivation();
    // Disables activation handle, if not already inactive.
    inline void disableActivation() { _actHandle.unset(); }

    // Activation status based on handle activation
    inline bool isActivated() const { return _actHandle.isActive(); }
    inline millis_t getTimeLeft() const { return _actHandle.getTimeLeft(); }
    inline millis_t getTimeActive(millis_t time = nzMillis()) const { return _actHandle.getTimeActive(time); }

    // Currently active driving intensity [-1.0,1.0] / calibrated value [calibMin,calibMax], from actuator
    inline float getActiveDriveIntensity();
    inline float getActiveCalibratedValue();

    // Currently setup driving intensity [-1.0,1.0] / calibrated value [calibMin,calibMax], from activation
    inline float getSetupDriveIntensity() const;
    inline float getSetupCalibratedValue();

    // Sets an update slot to run during execution of actuator that can further refine duration/intensity.
    // Useful for rate-based or variable activations. Slot receives actuator attachment pointer as parameter.
    // Guaranteed to be called with final finished activation.
    void setUpdateSlot(const Slot<HydroActuatorAttachment *> &updateSlot);
    inline void setUpdateFunction(void (*updateFunctionPtr)(HydroActuatorAttachment *)) { setUpdateSlot(FunctionSlot<HydroActuatorAttachment *>(updateFunctionPtr)); }
    template<class U> inline void setUpdateMethod(void (U::*updateMethodPtr)(HydroActivationHandle *), U *updateClassInst = nullptr) { setUpdateSlot(MethodSlot<U,HydroActuatorAttachment *>(updateClassInst ? updateClassInst : reinterpret_cast<U *>(_parent), updateMethodPtr)); }
    const Slot<HydroActuatorAttachment *> *getUpdateSlot() const { return _updateSlot; }

    inline const HydroActivationHandle &getActivationHandle() const { return _actHandle; }
    inline const HydroActivation &getActivationSetup() const { return _actSetup; }

    template<class U> inline void setObject(U obj, bool modify = false) { HydroAttachment::setObject(obj, modify); }
    inline SharedPtr<HydroActuator> getObject() { return HydroAttachment::getObject<HydroActuator>(); }
    inline HydroActuator *get() { return HydroAttachment::get<HydroActuator>(); }

    inline HydroActuator &operator*() { return *HydroAttachment::get<HydroActuator>(); }
    inline HydroActuator *operator->() { return HydroAttachment::get<HydroActuator>(); }

    inline HydroActuatorAttachment &operator=(const HydroIdentity &rhs) { setObject(rhs); return *this; }
    inline HydroActuatorAttachment &operator=(const char *rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroActuatorAttachment &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroActuatorAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }

protected:
    HydroActivationHandle _actHandle;                       // Actuator activation handle (double ref to object when active)
    HydroActivation _actSetup;                              // Actuator activation setup
    Slot<HydroActuatorAttachment *> *_updateSlot;           // Update slot (owned)
    float _rateMultiplier;                                  // Rate multiplier
    bool _calledLastUpdate;                                 // Last update call flag

    void applySetup();
};


// Sensor Measurement Attachment Point
// This attachment registers the parent object with a sensor's new measurement signal
// upon resolvement / unregisters the parent object from the sensor at time of
// destruction or reassignment.
// Custom handle method is responsible for calling setMeasurement() to update measurement.
class HydroSensorAttachment : public HydroSignalAttachment<const HydroMeasurement *, HYDRO_SENSOR_SIGNAL_SLOTS> {
public:
    HydroSensorAttachment(HydroObjInterface *parent = nullptr, uint8_t measurementRow = 0);
    HydroSensorAttachment(const HydroSensorAttachment &attachment);
    virtual ~HydroSensorAttachment();

    virtual void attachObject() override;
    virtual void detachObject() override;

    // Updates measurement attachment with sensor. Does not call sensor's update() (handled by system).
    virtual void updateIfNeeded(bool poll = false) override;

    // Sets the current measurement associated with this process. Required to be called by custom handlers.
    void setMeasurement(HydroSingleMeasurement measurement);
    inline void setMeasurement(float value, Hydro_UnitsType units = Hydro_UnitsType_Undefined) { setMeasurement(HydroSingleMeasurement(value, units)); }
    void setMeasurementRow(uint8_t measurementRow);
    void setMeasurementUnits(Hydro_UnitsType units, float convertParam = FLT_UNDEF);

    inline const HydroSingleMeasurement &getMeasurement(bool poll = false) { updateIfNeeded(poll); return _measurement; }
    inline uint16_t getMeasurementFrame(bool poll = false) { updateIfNeeded(poll); return _measurement.frame; }
    inline float getMeasurementValue(bool poll = false) { updateIfNeeded(poll); return _measurement.value; }
    inline Hydro_UnitsType getMeasurementUnits() const { return _measurement.units; }

    inline void setNeedsMeasurement() { _needsMeasurement = true; }
    inline bool needsMeasurement() { return _needsMeasurement; }

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
// This attachment registers the parent object with a triggers's trigger signal
// upon resolvement / unregisters the parent object from the trigger at time of
// destruction or reassignment.
class HydroTriggerAttachment  : public HydroSignalAttachment<Hydro_TriggerState, HYDRO_TRIGGER_SIGNAL_SLOTS> {
public:
    HydroTriggerAttachment(HydroObjInterface *parent = nullptr);
    HydroTriggerAttachment(const HydroTriggerAttachment &attachment);
    virtual ~HydroTriggerAttachment();

    // Updates owned trigger attachment.
    virtual void updateIfNeeded(bool poll = false) override;

    inline Hydro_TriggerState getTriggerState(bool poll = false);
    inline bool isTriggered(bool poll = false) { return getTriggerState(poll) == Hydro_TriggerState_Triggered; }

    inline SharedPtr<HydroTrigger> getObject() { return HydroAttachment::getObject<HydroTrigger>(); }
    inline HydroTrigger *get() { return HydroAttachment::get<HydroTrigger>(); }

    inline HydroTrigger &operator*() { return *HydroAttachment::getObject<HydroTrigger>().get(); }
    inline HydroTrigger *operator->() { return HydroAttachment::getObject<HydroTrigger>().get(); }

    inline HydroTriggerAttachment &operator=(const HydroIdentity &rhs) { setObject(rhs); return *this; }
    inline HydroTriggerAttachment &operator=(const char *rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroTriggerAttachment &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroTriggerAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }
};


// Balancer Attachment Point
// This attachment registers the parent object with a balancer's balancing signal
// upon resolvement / unregisters the parent object from the balancer at time of
// destruction or reassignment.
class HydroBalancerAttachment : public HydroSignalAttachment<Hydro_BalancingState, HYDRO_BALANCER_SIGNAL_SLOTS> {
public:
    HydroBalancerAttachment(HydroObjInterface *parent = nullptr);
    HydroBalancerAttachment(const HydroBalancerAttachment &attachment);
    virtual ~HydroBalancerAttachment();

    // Updates owned balancer attachment.
    virtual void updateIfNeeded(bool poll = false) override;

    inline Hydro_BalancingState getBalancingState(bool poll = false);

    template<class U> inline void setObject(U obj, bool modify = false) { HydroAttachment::setObject(obj, modify); }
    inline SharedPtr<HydroBalancer> getObject() { return HydroAttachment::getObject<HydroBalancer>(); }
    inline HydroBalancer *get() { return HydroAttachment::get<HydroBalancer>(); }

    inline HydroBalancer &operator*() { return *HydroAttachment::get<HydroBalancer>(); }
    inline HydroBalancer *operator->() { return HydroAttachment::get<HydroBalancer>(); }

    inline HydroBalancerAttachment &operator=(const HydroIdentity &rhs) { setObject(rhs); return *this; }
    inline HydroBalancerAttachment &operator=(const char *rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroBalancerAttachment &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline HydroBalancerAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }
};

#endif // /ifndef HydroAttachments_H
