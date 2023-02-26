/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Actuators
*/

#ifndef HydroActuators_H
#define HydroActuators_H

class HydroActuator;
class HydroRelayActuator;
class HydroRelayPumpActuator;
class HydroVariableActuator;
//class HydroVariablePumpActuator;

struct HydroActuatorData;
struct HydroPumpActuatorData;

#include "Hydruino.h"
#include "HydroDatas.h"
#include "HydroActivation.h"

// Creates actuator object from passed actuator data (return ownership transfer - user code *must* delete returned object)
extern HydroActuator *newActuatorObjectFromData(const HydroActuatorData *dataIn);


// Actuator Base
// This is the base class for all actuators, which defines how the actuator is identified,
// where it lives, and what it's attached to.
class HydroActuator : public HydroObject, public HydroActuatorObjectInterface, public HydroParentRailAttachmentInterface, public HydroParentReservoirAttachmentInterface {
public:
    const enum : signed char { Relay, RelayPump, Variable, VariablePump, Unknown = -1 } classType; // Actuator class type (custom RTTI)
    inline bool isRelayClass() const { return classType == Relay; }
    inline bool isRelayPumpClass() const { return classType == RelayPump; }
    inline bool isVariableClass() const { return classType == Variable; }
    inline bool isVariablePumpClass() const { return classType == VariablePump; }
    inline bool isAnyBinaryClass() const { return isRelayClass() || isRelayPumpClass(); }
    inline bool isAnyVariableClass() const { return isVariableClass() || isVariablePumpClass(); }
    inline bool isAnyPumpClass() const { return isRelayPumpClass() || isVariablePumpClass(); }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    HydroActuator(Hydro_ActuatorType actuatorType,
                  hposi_t actuatorIndex,
                  int classType = Unknown);
    HydroActuator(const HydroActuatorData *dataIn);

    virtual void update() override;

    virtual bool getCanEnable() override;

    // Activating actuators is done through activation handles, which must stay memory
    // resident in order for the actuator to pick up and process it. Enablement mode
    // affects how handles are processed - in parallel, or in serial - and what the
    // applied output is. See HydroActuatorAttachment for an abstraction of this process.
    inline HydroActivationHandle enableActuator(Hydro_DirectionMode direction, float intensity = 1.0f, millis_t duration = -1, bool force = false) { return HydroActivationHandle(::getSharedPtr<HydroActuator>(this), direction, intensity, duration, force); }
    inline HydroActivationHandle enableActuator(float value, millis_t duration = -1, bool force = false) { return enableActuator(Hydro_DirectionMode_Forward, calibrationInvTransform(value), duration, force); }
    inline HydroActivationHandle enableActuator(millis_t duration = -1, bool force = false) { return enableActuator(Hydro_DirectionMode_Forward, 1.0f, duration, force); }

    inline void setEnableMode(Hydro_EnableMode enableMode) { _enableMode = enableMode; setNeedsUpdate(); }
    inline Hydro_EnableMode getEnableMode() { return _enableMode; }

    inline bool isSerialMode() { return getActuatorIsSerialFromMode(getEnableMode()); }
    inline bool isPumpType() { return getActuatorIsPumpFromType(getActuatorType()); }
    inline bool isBidirectionalType() { return false; }

    virtual void setContinuousPowerUsage(HydroSingleMeasurement contPowerUsage) override;
    virtual const HydroSingleMeasurement &getContinuousPowerUsage() override;

    virtual HydroAttachment &getParentRailAttachment() override;
    virtual HydroAttachment &getParentReservoirAttachment() override;

    void setUserCalibrationData(HydroCalibrationData *userCalibrationData);
    inline const HydroCalibrationData *getUserCalibrationData() const { return _calibrationData; }

    // Transformation methods that convert from normalized driving intensity/driver value to calibration units
    inline float calibrationTransform(float value) const { return _calibrationData ? _calibrationData->transform(value) : value; }
    inline void calibrationTransform(float *valueInOut, Hydro_UnitsType *unitsOut = nullptr) const { if (valueInOut && _calibrationData) { _calibrationData->transform(valueInOut, unitsOut); } }
    inline HydroSingleMeasurement calibrationTransform(HydroSingleMeasurement measurement) { return _calibrationData ? HydroSingleMeasurement(_calibrationData->transform(measurement.value), _calibrationData->calibrationUnits, measurement.timestamp, measurement.frame) : measurement; }
    inline void calibrationTransform(HydroSingleMeasurement *measurementInOut) const { if (measurementInOut && _calibrationData) { _calibrationData->transform(&measurementInOut->value, &measurementInOut->units); } }

    // Transformation methods that convert from calibration units to normalized driving intensity/driver value
    inline float calibrationInvTransform(float value) const { return _calibrationData ? _calibrationData->inverseTransform(value) : value; }
    inline void calibrationInvTransform(float *valueInOut, Hydro_UnitsType *unitsOut = nullptr) const { if (valueInOut && _calibrationData) { _calibrationData->inverseTransform(valueInOut, unitsOut); } }
    inline HydroSingleMeasurement calibrationInvTransform(HydroSingleMeasurement measurement) { return _calibrationData ? HydroSingleMeasurement(_calibrationData->inverseTransform(measurement.value), _calibrationData->calibrationUnits, measurement.timestamp, measurement.frame) : measurement; }
    inline void calibrationInvTransform(HydroSingleMeasurement *measurementInOut) const { if (measurementInOut && _calibrationData) { _calibrationData->inverseTransform(&measurementInOut->value, &measurementInOut->units); } }

    inline float getCalibratedValue() const { return calibrationTransform(getDriveIntensity()); }

    inline Hydro_ActuatorType getActuatorType() const { return _id.objTypeAs.actuatorType; }
    inline hposi_t getActuatorIndex() const { return _id.posIndex; }

    inline void setNeedsUpdate() { _needsUpdate = true; }
    inline bool needsUpdate() { return _needsUpdate; }

    Signal<HydroActuator *, HYDRO_ACTUATOR_SIGNAL_SLOTS> &getActivationSignal();

protected:
    bool _enabled;                                          // Enabled state flag
    bool _needsUpdate;                                      // Stale flag for handle updates
    Hydro_EnableMode _enableMode;                           // Handle activation mode
    Vector<HydroActivationHandle *> _handles;               // Activation handles array
    HydroSingleMeasurement _contPowerUsage;                 // Continuous power draw
    HydroAttachment _parentRail;                            // Parent power rail attachment
    HydroAttachment _parentReservoir;                       // Parent reservoir attachment
    const HydroCalibrationData *_calibrationData;           // Calibration data
    Signal<HydroActuator *, HYDRO_ACTUATOR_SIGNAL_SLOTS> _activateSignal; // Activation update signal

    virtual HydroData *allocateData() const override;
    virtual void saveToData(HydroData *dataOut) override;

    virtual void handleActivation();

    friend struct HydroActivationHandle;
};


// Binary Relay Actuator
// This actuator acts as a standard on/off switch, typically paired with a variety of
// different equipment from pumps to grow lights and heaters.
class HydroRelayActuator : public HydroActuator {
public:
    HydroRelayActuator(Hydro_ActuatorType actuatorType,
                       hposi_t actuatorIndex,
                       HydroDigitalPin outputPin,
                       int classType = Relay);
    HydroRelayActuator(const HydroActuatorData *dataIn);
    virtual ~HydroRelayActuator();

    virtual bool getCanEnable() override;
    virtual float getDriveIntensity() const override;
    virtual bool isEnabled(float tolerance = 0.0f) const override;

    inline const HydroDigitalPin &getOutputPin() const { return _outputPin; }

protected:
    HydroDigitalPin _outputPin;                             // Digital output pin

    virtual void saveToData(HydroData *dataOut) override;

    virtual void _enableActuator(float intensity = 1.0) override;
    virtual void _disableActuator() override;
};


// Relay Pump Actuator
// This actuator acts as a water pump and attaches to both an input and output reservoir.
// Pumps using this class are either on/off and do not contain any variable flow control,
// but can be paired with a flow sensor for more precise pumping calculations.
class HydroRelayPumpActuator : public HydroRelayActuator, public HydroPumpObjectInterface, public HydroFlowRateUnitsInterfaceStorage, public HydroWaterFlowRateSensorAttachmentInterface {
public:
    HydroRelayPumpActuator(Hydro_ActuatorType actuatorType,
                           hposi_t actuatorIndex,
                           HydroDigitalPin outputPin,
                           int classType = RelayPump);
    HydroRelayPumpActuator(const HydroPumpActuatorData *dataIn);

    virtual void update() override;

    virtual bool getCanEnable() override;

    virtual bool canPump(float volume, Hydro_UnitsType volumeUnits = Hydro_UnitsType_Undefined) override;
    virtual HydroActivationHandle pump(float volume, Hydro_UnitsType volumeUnits = Hydro_UnitsType_Undefined) override;
    virtual bool canPump(millis_t time) override;
    virtual HydroActivationHandle pump(millis_t time) override;

    virtual void setFlowRateUnits(Hydro_UnitsType flowRateUnits) override;

    virtual HydroAttachment &getSourceReservoirAttachment() override;
    virtual HydroAttachment &getDestinationReservoirAttachment() override;

    virtual void setContinuousFlowRate(HydroSingleMeasurement contFlowRate) override;
    virtual const HydroSingleMeasurement &getContinuousFlowRate() override;

    virtual HydroSensorAttachment &getFlowRateSensorAttachment() override;

protected:
    HydroSingleMeasurement _contFlowRate;                   // Continuous flow rate
    HydroSensorAttachment _flowRate;                        // Flow rate sensor attachment
    HydroAttachment _destReservoir;                         // Destination output reservoir

    float _pumpVolumeAccum;                                 // Accumulator for total volume of fluid pumped
    millis_t _pumpTimeStart;                                // Time millis pump was activated at
    millis_t _pumpTimeAccum;                                // Time millis pump has been accumulated up to

    virtual void saveToData(HydroData *dataOut) override;

    virtual void handleActivation() override;

    virtual void handlePumpTime(millis_t time) override;
};


// Variable Actuator
// This actuator acts as a simple variable ranged dial, typically paired with a variety of
// different equipment that allows analog throttle or position control.
class HydroVariableActuator : public HydroActuator {
public:
    HydroVariableActuator(Hydro_ActuatorType actuatorType,
                          hposi_t actuatorIndex,
                          HydroAnalogPin outputPin,
                          int classType = Variable);
    HydroVariableActuator(const HydroActuatorData *dataIn);
    virtual ~HydroVariableActuator();

    virtual bool getCanEnable() override;
    virtual float getDriveIntensity() const override;
    virtual bool isEnabled(float tolerance = 0.0f) const override;

    inline int getDriveIntensity_raw() const { return _outputPin.bitRes.inverseTransform(_intensity); }

    inline const HydroAnalogPin &getOutputPin() const { return _outputPin; }

protected:
    HydroAnalogPin _outputPin;                              // Analog output pin
    float _intensity;                                       // Current set intensity

    virtual void saveToData(HydroData *dataOut) override;

    virtual void _enableActuator(float intensity = 1.0) override;
    virtual void _disableActuator() override;
};


// Variable/Throttled Pump Actuator
// This actuator acts as a throttleable water pump and attaches to both an input and output
// reservoir. Pumps using this class have variable flow control but also can be paired with
// a flow sensor for more precise pumping calculations.
//class HydroVariablePumpActuator : public HydroVariableActuator, public HydroPumpObjectInterface, public HydroWaterFlowRateSensorAttachmentInterface {
// TODO
//};


// Actuator Serialization Data
struct HydroActuatorData : public HydroObjectData
{
    HydroPinData outputPin;                                 // Output pin
    Hydro_EnableMode enableMode;                            // Activation enablement mode
    HydroMeasurementData contPowerUsage;                    // Continuous power usage
    char railName[HYDRO_NAME_MAXSIZE];                      // Parent rail
    char reservoirName[HYDRO_NAME_MAXSIZE];                 // Parent reservoir

    HydroActuatorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Pump Actuator Serialization Data
struct HydroPumpActuatorData : public HydroActuatorData
{
    Hydro_UnitsType flowRateUnits;                          // Flow rate units
    HydroMeasurementData contFlowRate;                      // Continuous flow rate
    char destReservoir[HYDRO_NAME_MAXSIZE];                 // Destination reservoir
    char flowRateSensor[HYDRO_NAME_MAXSIZE];                // Flow rate sensor

    HydroPumpActuatorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroActuators_H
