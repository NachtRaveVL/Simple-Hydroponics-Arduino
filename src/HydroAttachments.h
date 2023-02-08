/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
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
#include "HydroActuators.h"

// forward decls
extern hkey_t stringHash(String);
extern String addressToString(uintptr_t);

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
    inline bool needsResolved() const { return isUnresolved() && _key != (hkey_t)-1; }
    inline bool resolve() { return isResolved() || (bool)getObject(); }
    void unresolve();

    template<class U> inline void setObject(U obj) { (*this) = obj; }
    template<class U = HydroObjInterface> inline SharedPtr<U> getObject() { return reinterpret_pointer_cast<U>(_getObject()); }
    template<class U = HydroObjInterface> inline U* get() { return getObject<U>().get(); }

    inline HydroIdentity getId() const { return _obj ? _obj->getId() : (_keyStr ? HydroIdentity(_keyStr) : HydroIdentity(_key)); }
    inline hkey_t getKey() const { return _key; }
    inline String getKeyString() const { return _keyStr ? String(_keyStr) : (_obj ? _obj->getKeyString() : addressToString((uintptr_t)_key)); }

    inline operator bool() const { return isResolved(); }
    inline HydroObjInterface *operator->() { return get(); }

    inline HydroDLinkObject &operator=(HydroIdentity rhs);
    inline HydroDLinkObject &operator=(const char *rhs);
    template<class U> inline HydroDLinkObject &operator=(SharedPtr<U> &rhs);
    inline HydroDLinkObject &operator=(const HydroObjInterface *rhs);
    inline HydroDLinkObject &operator=(const HydroAttachment *rhs);
    inline HydroDLinkObject &operator=(nullptr_t) { return this->operator=((HydroObjInterface *)nullptr); }

    inline bool operator==(const HydroIdentity &rhs) const { return _key == rhs.key; }
    inline bool operator==(const char *rhs) const { return _key == stringHash(rhs); }
    template<class U> inline bool operator==(const SharedPtr<U> &rhs) const { return _key == (rhs ? rhs->getKey() : (hkey_t)-1); }
    inline bool operator==(const HydroObjInterface *rhs) const { return _key == (rhs ? rhs->getKey() : (hkey_t)-1); }
    inline bool operator==(nullptr_t) const { return _key == (hkey_t)-1; }

protected:
    hkey_t _key;                                            // Object key
    SharedPtr<HydroObjInterface> _obj;                      // Shared pointer to object
    const char *_keyStr;                                    // Copy of id.keyString (if not resolved, or unresolved)

private:
    SharedPtr<HydroObjInterface> _getObject();
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

    template<class U> void setObject(U obj);
    template<class U = HydroObjInterface> SharedPtr<U> getObject();
    template<class U = HydroObjInterface> inline U* get() { return getObject<U>().get(); }

    void setParent(HydroObjInterface *parent);
    inline HydroObjInterface *getParent() const { return _parent; }

    inline HydroIdentity getId() const { return _obj.getId(); }
    inline hkey_t getKey() const { return _obj.getKey(); }
    inline String getKeyString() const { return _obj.getKeyString(); }

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
    HydroObjInterface *_parent;                             // Parent object pointer (strong due to reverse ownership)
};


// Signal Attachment Point
// This attachment registers the parent object with a Signal getter off the linked object
// upon dereference / unregisters the parent object from the Signal at time of
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
    template<class U> inline void setHandleMethod(void (U::*handleMethodPtr)(ParameterType), U *handleClassInst = nullptr) { setUpdateSlot(MethodSlot<U,ParameterType>(handleClassInst ? handleClassInst : reinterpret_cast<U *>(_parent), handleMethodPtr)); }

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
// registers the parent object with an Actuator upon dereference / unregisters the parent
// object from the Actuator at time of destruction or reassignment.
class HydroActuatorAttachment : public HydroSignalAttachment<HydroActuator *, HYDRO_ACTUATOR_SIGNAL_SLOTS> {
public:
    HydroActuatorAttachment(HydroObjInterface *parent = nullptr);
    HydroActuatorAttachment(const HydroActuatorAttachment &attachment);
    virtual ~HydroActuatorAttachment();

    // Updates with actuator activation handle. Does not call actuator's update() (handled by system).
    virtual void updateIfNeeded(bool poll = false) override;

    // A rate multiplier is used to adjust either the intensity or duration of activations,
    // which depends on whenever they operate in binary mode (on/off) or variably (ranged).
    inline void setRateMultiplier(float rateMultiplier) { _rateMultiplier = rateMultiplier; applySetup(); }
    inline float getRateMultiplier() const { return _rateMultiplier; }

    // Activations are set up first by calling one of these methods. This configures the
    // direction, intensity, duration, and any run flags that the actuator will operate
    // upon once enabled, pending any rate offsetting. These methods are re-entrant.
    // The most recently used setup values are used for repeat activations.
    inline void setupActivation(const HydroActivationHandle::Activation &activation) { _actSetup = activation; applySetup(); }
    inline void setupActivation(const HydroActivationHandle &handle) { setupActivation(handle.activation); }
    inline void setupActivation(Hydro_DirectionMode direction, float intensity = 1.0f, millis_t duration = -1, bool force = false) { setupActivation(HydroActivationHandle::Activation(direction, intensity, duration, (force ? Hydro_ActivationFlags_Forced : Hydro_ActivationFlags_None))); }
    inline void setupActivation(millis_t duration, bool force = false) { setupActivation(HydroActivationHandle::Activation(Hydro_DirectionMode_Forward, 1.0f, duration, (force ? Hydro_ActivationFlags_Forced : Hydro_ActivationFlags_None))); }
    inline void setupActivation(bool force, millis_t duration = -1) { setupActivation(HydroActivationHandle::Activation(Hydro_DirectionMode_Forward, 1.0f, duration, (force ? Hydro_ActivationFlags_Forced : Hydro_ActivationFlags_None))); }
    // These activation methods takes into account actuator settings such as user
    // calibration data and type checks in determining how to interpret passed value.
    void setupActivation(float value, millis_t duration = -1, bool force = false);
    inline void setupActivation(const HydroSingleMeasurement &measurement, millis_t duration = -1, bool force = false) { setupActivation(measurement.value, duration, force); }

    // Enables activation handle with current setup, if not already active.
    // Repeat activations will reuse most recent setupActivation() values.
    void enableActivation();
    // Disables activation handle, if not already inactive.
    inline void disableActivation() { _actHandle.unset(); }

    // Activation status based on handle activation
    inline bool isActivated() const { return _actHandle.isActive(); }
    inline millis_t getTimeLeft() const { return _actHandle.getTimeLeft(); }
    inline millis_t getTimeActive(millis_t time = nzMillis()) const { return _actHandle.getTimeActive(time); }

    // Sets an update slot to run during execution of actuator that can further refine duration/intensity.
    // Useful for rate-based or variable activations. Slot receives actuator attachment pointer as parameter.
    // Guaranteed to be called with final finished activation.
    void setUpdateSlot(const Slot<HydroActuatorAttachment *> &updateSlot);
    inline void setUpdateFunction(void (*updateFunctionPtr)(HydroActuatorAttachment *)) { setUpdateSlot(FunctionSlot<HydroActuatorAttachment *>(updateFunctionPtr)); }
    template<class U> inline void setUpdateMethod(void (U::*updateMethodPtr)(HydroActivationHandle *), U *updateClassInst = nullptr) { setUpdateSlot(MethodSlot<U,HydroActuatorAttachment *>(updateClassInst ? updateClassInst : reinterpret_cast<U *>(_parent), updateMethodPtr)); }

    inline const HydroActivationHandle &getHandle() const { return _actHandle; }
    inline const HydroActivationHandle::Activation &getSetup() const { return _actSetup; }
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
    HydroActivationHandle::Activation _actSetup;            // Actuator activation setup
    Slot<HydroActuatorAttachment *> *_updateSlot;           // Update slot (owned)
    float _rateMultiplier;                                  // Rate multiplier
    bool _calledLastUpdate;                                 // Last update call flag

    void applySetup();
};


// Sensor Measurement Attachment Point
// This attachment registers the parent object with a Sensor's new measurement Signal
// upon dereference / unregisters the parent object from the Sensor at time of
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
// upon dereference / unregisters the parent object from the Trigger at time of
// destruction or reassignment.
class HydroTriggerAttachment  : public HydroSignalAttachment<Hydro_TriggerState, HYDRO_TRIGGER_SIGNAL_SLOTS> {
public:
    HydroTriggerAttachment(HydroObjInterface *parent = nullptr);
    HydroTriggerAttachment(const HydroTriggerAttachment &attachment);
    virtual ~HydroTriggerAttachment();

    // Updates owned trigger attachment.
    virtual void updateIfNeeded(bool poll = false) override;

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


// Balancer Attachment Point
// This attachment registers the parent object with a Balancer's balancing Signal
// upon dereference / unregisters the parent object from the Balancer at time of
// destruction or reassignment.
class HydroBalancerAttachment : public HydroSignalAttachment<Hydro_BalancingState, HYDRO_BALANCER_SIGNAL_SLOTS> {
public:
    HydroBalancerAttachment(HydroObjInterface *parent = nullptr);
    HydroBalancerAttachment(const HydroBalancerAttachment &attachment);
    virtual ~HydroBalancerAttachment();

    // Updates owned balancer attachment.
    virtual void updateIfNeeded(bool poll = false) override;

    inline Hydro_BalancingState getBalancingState();

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
