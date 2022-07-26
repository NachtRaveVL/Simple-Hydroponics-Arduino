/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Object
*/

#ifndef HydroponicsObject_H
#define HydroponicsObject_H

struct HydroponicsIdentity;
class HydroponicsObject;
class HydroponicsSubObject;

struct HydroponicsObjectData;

#include "Hydroponics.h"
#include "HydroponicsData.h"

// Creates object from passed object data (return ownership transfer - user code *must* delete returned object)
extern HydroponicsObject *newObjectFromData(const HydroponicsData *dataIn);

// Simple class for referencing an object in the Hydroponics system.
// This class is mainly used to simplify object key generation, which is used when we
// want to uniquely refer to objects in the Hydroponics system.
struct HydroponicsIdentity {
    enum { Actuator, Sensor, Crop, Reservoir, Rail, SubObject, Unknown = -1 } type; // Object type (custom RTTI)
    inline bool isActuatorType() const { return type == Actuator; }
    inline bool isSensorType() const { return type == Sensor; }
    inline bool isCropType() const { return type == Crop; }
    inline bool isReservoirType() const { return type == Reservoir; }
    inline bool isRailType() const { return type == Rail; }
    inline bool isSubObject() const { return type == SubObject; }
    inline bool isUnknownType() const { return type <= Unknown; }

    union {
        Hydroponics_ActuatorType actuatorType;              // As actuator type enumeration
        Hydroponics_SensorType sensorType;                  // As sensor type enumeration
        Hydroponics_CropType cropType;                      // As crop type enumeration
        Hydroponics_ReservoirType reservoirType;            // As reservoir type enumeration
        Hydroponics_RailType railType;                      // As rail type enumeration
    } objTypeAs;                                            // Enumeration type union
    Hydroponics_PositionIndex posIndex;                     // Position index
    String keyString;                                       // String key
    Hydroponics_KeyType key;                                // UInt Key

    // Default constructor (no id)
    HydroponicsIdentity();

    // Copy id with new position index
    HydroponicsIdentity(const HydroponicsIdentity &id,
                        Hydroponics_PositionIndex positionIndex);

    // Actuator id constructor
    HydroponicsIdentity(Hydroponics_ActuatorType actuatorType,
                        Hydroponics_PositionIndex positionIndex = HYDRUINO_POS_SEARCH_FROMBEG);
    // Sensor id constructor
    HydroponicsIdentity(Hydroponics_SensorType sensorType,
                        Hydroponics_PositionIndex positionIndex = HYDRUINO_POS_SEARCH_FROMBEG);
    // Crop id constructor
    HydroponicsIdentity(Hydroponics_CropType cropType,
                        Hydroponics_PositionIndex positionIndex = HYDRUINO_POS_SEARCH_FROMBEG);
    // Reservoir id constructor
    HydroponicsIdentity(Hydroponics_ReservoirType reservoirType,
                        Hydroponics_PositionIndex positionIndex = HYDRUINO_POS_SEARCH_FROMBEG);
    // Rail id constructor
    HydroponicsIdentity(Hydroponics_RailType railType,
                        Hydroponics_PositionIndex positionIndex = HYDRUINO_POS_SEARCH_FROMBEG);

    // Sub-object null-id constructor
    HydroponicsIdentity(const HydroponicsSubObject *obj);

    // Data constructor
    HydroponicsIdentity(const HydroponicsData *dataIn);

    // String constructor
    HydroponicsIdentity(const char *idKeyStr);
    // String constructor
    HydroponicsIdentity(const String &idKeyString);

    // Used to update key value after modification, returning new key by convenience
    Hydroponics_KeyType regenKey();

    inline operator bool() const { return key != (Hydroponics_KeyType)-1 && keyString.length(); }
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
    inline bool isSubObject() const { return _id.isSubObject(); }
    inline bool isUnknownType() const { return _id.isUnknownType(); }

    HydroponicsObject(HydroponicsIdentity id);              // Standard constructor
    HydroponicsObject(const HydroponicsData *dataIn);       // Data constructor
    virtual ~HydroponicsObject();                           // Destructor

    virtual void update();                                  // Called over intervals of time by runloop
    virtual void handleLowMemory();                         // Called upon low memory condition to try and free memory up

    HydroponicsData *newSaveData();                         // Saves object state to proper backing data

    virtual bool addLinkage(HydroponicsObject *obj);        // Adds linkage to this object, returns true upon initial add
    virtual bool removeLinkage(HydroponicsObject *obj);     // Removes linkage from this object, returns true upon last remove
    bool hasLinkage(HydroponicsObject *obj) const;          // Checks object linkage to this object

    // Returns the linkages this object contains, along with refcount for how many times it has registered itself as linked (via attachment points).
    // Objects are considered strong pointers, since existence -> shared_ptr ref to this instance exists.
    inline const Map<Hydroponics_KeyType, Pair<HydroponicsObject *, int8_t>::type, HYDRUINO_OBJ_LINKS_MAXSIZE>::type getLinkages() const { return _links; }

    inline const HydroponicsIdentity &getId() const { return _id; } // Returns the unique Identity of the object
    inline Hydroponics_KeyType getKey() const { return _id.key; } // Returns the unique key of the object
    inline const String &getKeyString() const { return _id.keyString; } // Returns the key string of the object
    shared_ptr<HydroponicsObject> getSharedPtr() const;     // Returns the shared_ptr instance of the object

protected:
    HydroponicsIdentity _id;                                // Object id
    Map<Hydroponics_KeyType, Pair<HydroponicsObject *, int8_t>::type, HYDRUINO_OBJ_LINKS_MAXSIZE>::type _links; // Linked objects (strong)

    virtual HydroponicsData *allocateData() const;          // Only up to base type classes (sensor, crop, etc.) does this need overriden
    virtual void saveToData(HydroponicsData *dataOut);      // *ALL* derived classes must override and implement

private:
    HydroponicsObject() = default;                          // Private constructor to disable derived/public access
};

// Shortcut to get shared pointer from object with static pointer cast built-in.
template<class T> inline shared_ptr<T> getSharedPtr(const HydroponicsObject *object) { return static_pointer_cast<T>(object->getSharedPtr()); }


// Hydroponics Sub Object Base
// A base class for sub objects that are typically found embedded in bigger main objects,
// but want to replicate some of the same functionality. Not required to be inherited from.
class HydroponicsSubObject {
public:
    inline const HydroponicsIdentity &getId() const { return HydroponicsIdentity(this); }
    inline Hydroponics_KeyType getKey() const { return (Hydroponics_KeyType)(uintptr_t)this; }
    template<class T> inline shared_ptr<T> getSharedPtr() const { return shared_ptr<T>(this); }; // Only meant to be used once

    virtual void update() = 0;
    virtual void handleLowMemory() = 0;

    inline bool addLinkage(HydroponicsObject *obj) { return false; }
    inline bool removeLinkage(HydroponicsObject *obj) { return false; }
};

// Shortcut to get shared pointer from object with static pointer cast built-in.
// Only meant to be used once during initial object assignment directly from new operator (e.g. <attachment> = new HydroSubObject()).
template<class T> inline shared_ptr<T> getSharedPtr(const HydroponicsSubObject *subObj) { return subObj->getSharedPtr<T>(); }


// Hydroponics Object Data Intermediate
// Intermediate data class for object data.
struct HydroponicsObjectData : public HydroponicsData {
    char name[HYDRUINO_NAME_MAXSIZE];

    HydroponicsObjectData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroponicsObject_H
