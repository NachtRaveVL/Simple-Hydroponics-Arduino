/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Object
*/

#ifndef HydroponicsObject_H
#define HydroponicsObject_H

struct HydroponicsIdentity;
class HydroponicsObject;

#include "Hydroponics.h"

// Simple class for referencing an object in the Hydroponics system.
// This class is mainly used to simplify object key generation, which is used when we
// want to uniquely refer to objects in the Hydroponics system.
struct HydroponicsIdentity {
    enum { Actuator, Sensor, Crop, Reservoir, Rail, Unknown = -1 } type; // Object type (custom RTTI)
    inline bool isActuatorType() const { return type == Actuator; }
    inline bool isSensorType() const { return type == Sensor; }
    inline bool isCropType() const { return type == Crop; }
    inline bool isReservoirType() const { return type == Reservoir; }
    inline bool isRailType() const { return type == Rail; }
    inline bool isUnknownType() const { return type >= Unknown; }

    // Default constructor (no id)
    HydroponicsIdentity();

    // Copy id with new position index
    HydroponicsIdentity(const HydroponicsIdentity &id,
                        Hydroponics_PositionIndex positionIndex);

    // Actuator id
    HydroponicsIdentity(Hydroponics_ActuatorType actuatorType,
                        Hydroponics_PositionIndex positionIndex = HYDRUINO_POS_SEARCH_FROMBEG);
    // Sensor id
    HydroponicsIdentity(Hydroponics_SensorType sensorType,
                        Hydroponics_PositionIndex positionIndex = HYDRUINO_POS_SEARCH_FROMBEG);
    // Crop id
    HydroponicsIdentity(Hydroponics_CropType cropType,
                        Hydroponics_PositionIndex positionIndex = HYDRUINO_POS_SEARCH_FROMBEG);
    // Reservoir id
    HydroponicsIdentity(Hydroponics_ReservoirType reservoirType,
                        Hydroponics_PositionIndex positionIndex = HYDRUINO_POS_SEARCH_FROMBEG);
    // Rail id
    HydroponicsIdentity(Hydroponics_RailType railType,
                        Hydroponics_PositionIndex positionIndex = HYDRUINO_POS_SEARCH_FROMBEG);

    union {
        Hydroponics_ActuatorType actuatorType;              // As actuator type enumeration
        Hydroponics_SensorType sensorType;                  // As sensor type enumeration
        Hydroponics_CropType cropType;                      // As crop type enumeration
        Hydroponics_ReservoirType reservoirType;            // As reservoir type enumeration
        Hydroponics_RailType railType;                      // As rail type enumeration
    } as;                                                   // Type enumeration union
    Hydroponics_PositionIndex posIndex;                     // Position index
    String keyStr;                                          // String key
    Hydroponics_KeyType key;                                // UInt Key

    Hydroponics_KeyType regenKey();                         // Used to update Key value after modification, returns new key by convenience

    inline operator bool() const { return key != (Hydroponics_KeyType)-1; }
    inline bool operator==(const HydroponicsIdentity &otherId) const { return key == otherId.key; }
    inline bool operator!=(const HydroponicsIdentity &otherId) const { return key != otherId.key; }
};

// Hydroponic Object Base
// A simple base class for referring to objects in the Hydroponics system.
class HydroponicsObject {
public:
    inline bool isActuatorType() const { return _id.isActuatorType(); }
    inline bool isSensorType() const { return _id.isSensorType(); }
    inline bool isCropType() const { return _id.isCropType(); }
    inline bool isReservoirType() const { return _id.isReservoirType(); }
    inline bool isRailType() const { return _id.isRailType(); }
    inline bool isUnknownType() const { return _id.isUnknownType(); }

    HydroponicsObject(HydroponicsIdentity id);              // Standard constructor
    virtual ~HydroponicsObject();                           // Destructor

    virtual void update();                                  // Called over intervals of time by runloop
    virtual void resolveLinks();                            // Called after unpack/during launch, to link delayed load objects
    virtual void handleLowMemory();                         // Called upon low memory condition to try and free memory up

    bool hasLinkage(HydroponicsObject *obj) const;          // Checks object linkage to this object.

    const HydroponicsIdentity getId() const;                // Returns the unique Identity of the object.
    const Hydroponics_KeyType getKey() const;               // Returns the unique key of the object.
protected:
    HydroponicsIdentity _id;                                // Object id
    arx::map<Hydroponics_KeyType, HydroponicsObject *, HYDRUINO_LINKAGE_MAXSIZE> _links; // Linked objects (weak)

    bool addLinkage(HydroponicsObject *obj);
    bool removeLinkage(HydroponicsObject *obj);
};

#endif // /ifndef HydroponicsObject_H
