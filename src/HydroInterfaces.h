/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Interfaces
*/

#ifndef HydroInterfaces_H
#define HydroInterfaces_H

struct HydroJSONSerializableInterface;

class HydroObjInterface;
class HydroUIInterface;
class HydroRTCInterface;

struct HydroDigitalInputPinInterface;
struct HydroDigitalOutputPinInterface;
struct HydroAnalogInputPinInterface;
struct HydroAnalogOutputPinInterface;

class HydroAirConcentrateUnitsInterfaceStorage;
class HydroDilutionUnitsInterfaceStorage;
class HydroFlowRateUnitsInterfaceStorage;
class HydroMeasurementUnitsInterface;
template <size_t N = 1> class HydroMeasurementUnitsStorage;
class HydroMeasurementUnitsInterfaceStorageSingle;
class HydroMeasurementUnitsInterfaceStorageDouble;
class HydroMeasurementUnitsInterfaceStorageTriple;
class HydroPowerUnitsInterfaceStorage;
class HydroTemperatureUnitsInterfaceStorage;
class HydroVolumeUnitsInterfaceStorage;
class HydroWaterConcentrateUnitsInterfaceStorage;

class HydroActuatorObjectInterface;
class HydroSensorObjectInterface;
class HydroCropObjectInterface;
class HydroReservoirObjectInterface;
class HydroRailObjectInterface;
class HydroBalancerObjectInterface;
class HydroTriggerObjectInterface;

class HydroPumpObjectInterface;

class HydroParentActuatorAttachmentInterface;
class HydroParentSensorAttachmentInterface;
class HydroParentCropAttachmentInterface;
class HydroParentReservoirAttachmentInterface;
class HydroParentRailAttachmentInterface;

class HydroFeedReservoirAttachmentInterface;

class HydroSensorAttachmentInterface;
class HydroAirCO2SensorAttachmentInterface;
class HydroAirTemperatureSensorAttachmentInterface;
class HydroPowerProductionSensorAttachmentInterface;
class HydroPowerUsageSensorAttachmentInterface;
class HydroSoilMoistureSensorAttachmentInterface;
class HydroWaterFlowRateSensorAttachmentInterface;
class HydroWaterPHSensorAttachmentInterface;
class HydroWaterTDSSensorAttachmentInterface;
class HydroWaterTemperatureSensorAttachmentInterface;
class HydroWaterVolumeSensorAttachmentInterface;

class HydroTriggerAttachmentInterface;
class HydroFilledTriggerAttachmentInterface;
class HydroEmptyTriggerAttachmentInterface;
class HydroFeedingTriggerAttachmentInterface;
class HydroLimitTriggerAttachmentInterface;

#include "Hydruino.h"

// JSON Serializable Interface
struct HydroJSONSerializableInterface {
    // Given a JSON element to fill in, writes self to JSON format.
    virtual void toJSONObject(JsonObject &objectOut) const = 0;

    // Given a JSON element to read from, reads overtop self from JSON format.
    virtual void fromJSONObject(JsonObjectConst &objectIn) = 0;
};


// Object Interface
class HydroObjInterface {
public:
    virtual void unresolveAny(HydroObject *obj) = 0;

    virtual HydroIdentity getId() const = 0;
    virtual hkey_t getKey() const = 0;
    virtual String getKeyString() const = 0;
    virtual SharedPtr<HydroObjInterface> getSharedPtr() const = 0;
    virtual SharedPtr<HydroObjInterface> getSharedPtrFor(const HydroObjInterface *obj) const = 0;

    virtual bool isObject() const = 0;
    inline bool isSubObject() const { return !isObject(); }
};

// UI Interface
class HydroUIInterface {
public:
    virtual bool begin() = 0;

    virtual void setNeedsLayout() = 0;
};

// RTC Module Interface
class HydroRTCInterface {
public:
    virtual bool begin(TwoWire *wireInstance) = 0;
    virtual void adjust(const DateTime &dt) = 0;
    virtual bool lostPower(void) = 0;
    virtual DateTime now() = 0;
};


// Digital Input Pin Interface
struct HydroDigitalInputPinInterface {
    virtual ard_pinstatus_t digitalRead() = 0;
    inline int get() { return digitalRead(); }
};

// Digital Output Pin Interface
struct HydroDigitalOutputPinInterface {
    virtual void digitalWrite(ard_pinstatus_t status) = 0;
    inline void set(ard_pinstatus_t status) { digitalWrite(status); }
};

// Analog Input Pin Interface
struct HydroAnalogInputPinInterface {
    virtual float analogRead() = 0;
    virtual int analogRead_raw() = 0;
    inline float get() { return analogRead(); }
    inline int get_raw() { return analogRead_raw(); }
};

// Analog Output Pin Interface
struct HydroAnalogOutputPinInterface {
    virtual void analogWrite(float amount) = 0;
    virtual void analogWrite_raw(int amount) = 0;
    inline void set(float amount) { analogWrite(amount); }
    inline void set_raw(int amount) { analogWrite_raw(amount); }
};


// Air Concentrate Units Interface + Storage
class HydroAirConcentrateUnitsInterfaceStorage {
public:
    virtual void setAirConcentrateUnits(Hydro_UnitsType airConcentrateUnits) = 0;
    inline Hydro_UnitsType getAirConcentrateUnits() const { return _airConcUnits; }

protected:
    Hydro_UnitsType _airConcUnits;
    inline HydroAirConcentrateUnitsInterfaceStorage(Hydro_UnitsType airConcentrateUnits = Hydro_UnitsType_Undefined) : _airConcUnits(airConcentrateUnits) { ; }
};

// Dilution Units Interface + Storage
class HydroDilutionUnitsInterfaceStorage {
public:
    virtual void setDilutionUnits(Hydro_UnitsType dilutionUnits) = 0;
    inline Hydro_UnitsType getDilutionUnits() const { return _dilutionUnits; }
    inline Hydro_UnitsType getVolumeUnits() const;

protected:
    Hydro_UnitsType _dilutionUnits;
    inline HydroDilutionUnitsInterfaceStorage(Hydro_UnitsType dilutionUnits = Hydro_UnitsType_Undefined) : _dilutionUnits(dilutionUnits) { ; }
};

// Flow Rate Units Interface + Storage
class HydroFlowRateUnitsInterfaceStorage {
public:
    virtual void setFlowRateUnits(Hydro_UnitsType flowRateUnits) = 0;
    inline Hydro_UnitsType getFlowRateUnits() const { return _flowRateUnits; }
    inline Hydro_UnitsType getVolumeUnits() const;

protected:
    Hydro_UnitsType _flowRateUnits;
    inline HydroFlowRateUnitsInterfaceStorage(Hydro_UnitsType flowRateUnits = Hydro_UnitsType_Undefined) : _flowRateUnits(flowRateUnits) { ; }
};

// Measure Units Interface
class HydroMeasurementUnitsInterface {
public:
    virtual void setMeasurementUnits(Hydro_UnitsType measurementUnits, uint8_t measurementRow = 0) = 0;
    virtual Hydro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const = 0;

    inline Hydro_UnitsType getRateUnits(uint8_t measurementRow = 0) const;
    inline Hydro_UnitsType getBaseUnits(uint8_t measurementRow = 0) const;
};

// Measure Units Storage
template <size_t N> class HydroMeasurementUnitsStorage {
protected:
    Hydro_UnitsType _measurementUnits[N];
    inline HydroMeasurementUnitsStorage(Hydro_UnitsType measurementUnits = Hydro_UnitsType_Undefined) { for (hposi_t i = 0; i < N; ++i) { _measurementUnits[i] = measurementUnits; } }
};

// Single Measure Units Interface + Storage
class HydroMeasurementUnitsInterfaceStorageSingle : public HydroMeasurementUnitsInterface, public HydroMeasurementUnitsStorage<1> {
protected:
    inline HydroMeasurementUnitsInterfaceStorageSingle(Hydro_UnitsType measurementUnits = Hydro_UnitsType_Undefined) : HydroMeasurementUnitsStorage<1>(measurementUnits) { ; }
};

// Double Measure Units Interface + Storage
class HydroMeasurementUnitsInterfaceStorageDouble : public HydroMeasurementUnitsInterface, public HydroMeasurementUnitsStorage<2> {
protected:
    inline HydroMeasurementUnitsInterfaceStorageDouble(Hydro_UnitsType measurementUnits = Hydro_UnitsType_Undefined) : HydroMeasurementUnitsStorage<2>(measurementUnits) { ; }
};

// Triple Measure Units Interface + Storage
class HydroMeasurementUnitsInterfaceStorageTriple : public HydroMeasurementUnitsInterface, public HydroMeasurementUnitsStorage<3> {
protected:
    inline HydroMeasurementUnitsInterfaceStorageTriple(Hydro_UnitsType measurementUnits = Hydro_UnitsType_Undefined) : HydroMeasurementUnitsStorage<3>(measurementUnits) { ; }
};

// Power Units Interface + Storage
class HydroPowerUnitsInterfaceStorage {
public:
    virtual void setPowerUnits(Hydro_UnitsType powerUnits) = 0;
    inline Hydro_UnitsType getPowerUnits() const { return _powerUnits; }

protected:
    Hydro_UnitsType _powerUnits;
    inline HydroPowerUnitsInterfaceStorage(Hydro_UnitsType powerUnits = Hydro_UnitsType_Undefined) : _powerUnits(powerUnits) { ; }
};

// Temperature Units Interface + Storage
class HydroTemperatureUnitsInterfaceStorage {
public:
    virtual void setTemperatureUnits(Hydro_UnitsType temperatureUnits) = 0;
    inline Hydro_UnitsType getTemperatureUnits() const { return _tempUnits; }

protected:
    Hydro_UnitsType _tempUnits;
    inline HydroTemperatureUnitsInterfaceStorage(Hydro_UnitsType temperatureUnits = Hydro_UnitsType_Undefined) : _tempUnits(temperatureUnits) { ; }
};

// Volume Units Interface + Storage
class HydroVolumeUnitsInterfaceStorage {
public:
    virtual void setVolumeUnits(Hydro_UnitsType volumeUnits) = 0;
    inline Hydro_UnitsType getVolumeUnits() const { return _volumeUnits; }
    inline Hydro_UnitsType getFlowRateUnits() const;
    inline Hydro_UnitsType getDilutionUnits() const;

protected:
    Hydro_UnitsType _volumeUnits;
    inline HydroVolumeUnitsInterfaceStorage(Hydro_UnitsType volumeUnits = Hydro_UnitsType_Undefined) : _volumeUnits(volumeUnits) { ; }
};

// Water Concentrate Units Interface + Storage
class HydroWaterConcentrateUnitsInterfaceStorage {
public:
    virtual void setWaterConcentrateUnits(Hydro_UnitsType waterConcentrateUnits) = 0;
    inline Hydro_UnitsType getWaterConcentrateUnits() const { return _waterConcUnits; }

protected:
    Hydro_UnitsType _waterConcUnits;
    inline HydroWaterConcentrateUnitsInterfaceStorage(Hydro_UnitsType waterConcentrateUnits = Hydro_UnitsType_Undefined) : _waterConcUnits(waterConcentrateUnits) { ; }
};


// Actuator Object Interface
class HydroActuatorObjectInterface {
public:
    virtual bool getCanEnable() = 0;
    virtual float getDriveIntensity() const = 0;
    virtual bool isEnabled(float tolerance = 0.0f) const = 0;

    virtual void setContinuousPowerUsage(HydroSingleMeasurement contPowerUsage) = 0;
    virtual const HydroSingleMeasurement &getContinuousPowerUsage() = 0;
    inline void setContinuousPowerUsage(float contPowerUsage, Hydro_UnitsType contPowerUsageUnits = Hydro_UnitsType_Undefined);

protected:
    virtual void _enableActuator(float intensity = 1.0) = 0;
    virtual void _disableActuator() = 0;
};

// Sensor Object Interface
class HydroSensorObjectInterface {
public:
    virtual bool takeMeasurement(bool force = false) = 0;
    virtual const HydroMeasurement *getMeasurement(bool poll = false) = 0;
    virtual bool isTakingMeasurement() const = 0;
    virtual bool needsPolling(hframe_t allowance = 0) const = 0;
};

// Crop Object Interface
class HydroCropObjectInterface {
public:
    virtual bool needsFeeding(bool poll = false) = 0;
    virtual void notifyFeedingBegan() = 0;
    virtual void notifyFeedingEnded() = 0;
};

// Reservoir Object Interface
class HydroReservoirObjectInterface {
public:
    virtual bool canActivate(HydroActuator *actuator) = 0;
    virtual bool isFilled(bool poll = false) = 0;
    virtual bool isEmpty(bool poll = false) = 0;
};

// Rail Object Interface
class HydroRailObjectInterface {
public:
    virtual bool canActivate(HydroActuator *actuator) = 0;
    virtual float getCapacity(bool poll = false) = 0;
};

// Balancer Object Interface
class HydroBalancerObjectInterface {
public:
    virtual void setTargetSetpoint(float targetSetpoint) = 0;
    virtual Hydro_BalancingState getBalancingState(bool poll = false) = 0;
    inline bool isBalanced(bool poll = false) { return getBalancingState(poll) == Hydro_BalancingState_Balanced; }
};

// Trigger Object Interface
class HydroTriggerObjectInterface {
public:
    virtual Hydro_TriggerState getTriggerState(bool poll = false) = 0;
    inline bool isTriggered(bool poll = false) { return getTriggerState(poll) == Hydro_TriggerState_Triggered; }
};


// Pump Object Interface
class HydroPumpObjectInterface {
public:
    virtual bool canPump(float volume, Hydro_UnitsType volumeUnits = Hydro_UnitsType_Undefined) = 0;
    virtual HydroActivationHandle pump(float volume, Hydro_UnitsType volumeUnits = Hydro_UnitsType_Undefined) = 0;
    virtual bool canPump(millis_t time) = 0;
    virtual HydroActivationHandle pump(millis_t time) = 0;

    virtual HydroAttachment &getSourceReservoirAttachment() = 0;
    template<class U> inline void setSourceReservoir(U reservoir);
    template<class U = HydroReservoir> inline SharedPtr<U> getSourceReservoir();

    virtual HydroAttachment &getDestinationReservoirAttachment() = 0;
    template<class U> inline void setDestinationReservoir(U reservoir);
    template<class U = HydroReservoir> inline SharedPtr<U> getDestinationReservoir();

    virtual void setContinuousFlowRate(HydroSingleMeasurement contFlowRate) = 0;
    virtual const HydroSingleMeasurement &getContinuousFlowRate() = 0;
    inline void setContinuousFlowRate(float contFlowRate, Hydro_UnitsType contFlowRateUnits = Hydro_UnitsType_Undefined);

    inline bool isSourceReservoirEmpty(bool poll = false);
    inline bool isDestinationReservoirFilled(bool poll = false);

protected:
    virtual void handlePumpTime(millis_t time) = 0;
};


// Parent Actuator Attachment Interface
class HydroParentActuatorAttachmentInterface {
public:
    virtual HydroAttachment &getParentActuatorAttachment() = 0;

    template<class U> inline void setParentActuator(U actuator);
    template<class U = HydroActuator> inline SharedPtr<U> getParentActuator();
};

// Parent Sensor Attachment Interface
class HydroParentSensorAttachmentInterface {
public:
    virtual HydroAttachment &getParentSensorAttachment() = 0;

    template<class U> inline void setParentSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getParentSensor();
};

// Parent Crop Attachment Interface
class HydroParentCropAttachmentInterface {
public:
    virtual HydroAttachment &getParentCropAttachment() = 0;

    template<class U> inline void setParentCrop(U crop);
    template<class U = HydroCrop> inline SharedPtr<U> getParentCrop();
};

// Parent Reservoir Attachment Interface
class HydroParentReservoirAttachmentInterface {
public:
    virtual HydroAttachment &getParentReservoirAttachment() = 0;

    template<class U> inline void setParentReservoir(U reservoir);
    template<class U = HydroReservoir> inline SharedPtr<U> getParentReservoir();
};

// Parent Rail Attachment Interface
class HydroParentRailAttachmentInterface {
public:
    virtual HydroAttachment &getParentRailAttachment() = 0;

    template<class U> inline void setParentRail(U rail);
    template<class U = HydroRail> inline SharedPtr<U> getParentRail();
};


// Feed Reservoir Attachment Interface
class HydroFeedReservoirAttachmentInterface {
public:
    virtual HydroAttachment &getFeedReservoirAttachment() = 0;

    template<class U> inline void setFeedReservoir(U reservoir);
    template<class U = HydroFeedReservoir> inline SharedPtr<U> getFeedReservoir(bool poll = false);
};


// Abstract Sensor Attachment Interface
class HydroSensorAttachmentInterface {
    virtual HydroSensorAttachment &getSensorAttachment() = 0;

    template<class U> inline void setSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getSensor(bool poll = false);
};

// Air CO2 Sensor Attachment Interface
class HydroAirCO2SensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getAirCO2SensorAttachment() = 0;

    template<class U> inline void setAirCO2Sensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getAirCO2Sensor(bool poll = false);
};

// Air Temperature Sensor Attachment Interface
class HydroAirTemperatureSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getAirTemperatureSensorAttachment() = 0;

    template<class U> inline void setAirTemperatureSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getAirTemperatureSensor(bool poll = false);
};

// Power Production Sensor Attachment Interface
class HydroPowerProductionSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getPowerProductionSensorAttachment() = 0;

    template<class U> inline void setPowerProductionSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getPowerProductionSensor(bool poll = false);
};

// Power Usage Sensor Attachment Interface
class HydroPowerUsageSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getPowerUsageSensorAttachment() = 0;

    template<class U> inline void setPowerUsageSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getPowerUsageSensor(bool poll = false);
};

// Soil Moisture Sensor Attachment Interface
class HydroSoilMoistureSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getSoilMoistureSensorAttachment() = 0;

    template<class U> inline void setSoilMoistureSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getSoilMoistureSensor(bool poll = false);
};

// Liquid Flow Rate Sensor Attachment Interface
class HydroWaterFlowRateSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getFlowRateSensorAttachment() = 0;

    template<class U> inline void setFlowRateSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getFlowRateSensor(bool poll = false);
};

// Water pH/Alkalinity Sensor Attachment Interface
class HydroWaterPHSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getWaterPHSensorAttachment() = 0;

    template<class U> inline void setWaterPHSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getWaterPHSensor(bool poll = false);
};

// Water TDS/Concentration Sensor Attachment Interface
class HydroWaterTDSSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getWaterTDSSensorAttachment() = 0;

    template<class U> inline void setWaterTDSSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getWaterTDSSensor(bool poll = false);
};

// Water Temperature Sensor Attachment Interface
class HydroWaterTemperatureSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getWaterTemperatureSensorAttachment() = 0;

    template<class U> inline void setWaterTemperatureSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getWaterTemperatureSensor(bool poll = false);
};

// Liquid Volume Sensor Attachment Interface
class HydroWaterVolumeSensorAttachmentInterface {
public:
    virtual HydroSensorAttachment &getWaterVolumeSensorAttachment() = 0;

    template<class U> inline void setWaterVolumeSensor(U sensor);
    template<class U = HydroSensor> inline SharedPtr<U> getWaterVolumeSensor(bool poll = false);
};


// Abstract Trigger Attachment Interface
class HydroTriggerAttachmentInterface {
    virtual HydroTriggerAttachment &getTriggerAttachment() = 0;

    template<class U> inline void setTrigger(U trigger);
    template<class U = HydroTrigger> inline SharedPtr<U> getTrigger(bool poll = false);
};

// Filled Trigger Attachment Interface
class HydroFilledTriggerAttachmentInterface {
public:
    virtual HydroTriggerAttachment &getFilledTriggerAttachment() = 0;

    template<class U> inline void setFilledTrigger(U trigger);
    template<class U = HydroTrigger> inline SharedPtr<U>getFilledTrigger(bool poll = false);
};

// Empty Trigger Attachment Interface
class HydroEmptyTriggerAttachmentInterface {
public:
    virtual HydroTriggerAttachment &getEmptyTriggerAttachment() = 0;

    template<class U> inline void setEmptyTrigger(U trigger);
    template<class U = HydroTrigger> inline SharedPtr<U>getEmptyTrigger(bool poll = false);
};

// Feeding Trigger Attachment Interface
class HydroFeedingTriggerAttachmentInterface {
public:
    virtual HydroTriggerAttachment &getFeedingTriggerAttachment() = 0;

    template<class U> inline void setFeedingTrigger(U trigger);
    template<class U = HydroTrigger> inline SharedPtr<U>getFeedingTrigger(bool poll = false);
};

// Limit Trigger Attachment Interface
class HydroLimitTriggerAttachmentInterface {
public:
    virtual HydroTriggerAttachment &getLimitTriggerAttachment() = 0;

    template<class U> inline void setLimitTrigger(U trigger);
    template<class U = HydroTrigger> inline SharedPtr<U>getLimitTrigger(bool poll = false);
};

#endif // /ifndef HydroInterfaces_H
