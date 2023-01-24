/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Object
*/

#ifndef HydroObject_H
#define HydroObject_H

struct HydroIdentity;
class HydroObject;
class HydroSubObject;

struct HydroObjectData;

#include "Hydruino.h"
#include "HydroData.h"

// Creates object from passed object data (return ownership transfer - user code *must* delete returned object)
extern HydroObject *newObjectFromData(const HydroData *dataIn);

// Shortcut to get shared pointer for object with static pointer cast built-in.
template<class T = HydroObjInterface> inline SharedPtr<T> getSharedPtr(const HydroObjInterface *obj) { return obj ? reinterpret_pointer_cast<T>(obj->getSharedPtr()) : nullptr; }


// Simple class for referencing an object in the Hydruino system.
// This class is mainly used to simplify object key generation, which is used when we
// want to uniquely refer to objects in the Hydruino system.
struct HydroIdentity {
    enum : signed char { Actuator, Sensor, Crop, Reservoir, Rail, Unknown = -1 } type; // Object type (custom RTTI)
    inline bool isActuatorType() const { return type == Actuator; }
    inline bool isSensorType() const { return type == Sensor; }
    inline bool isCropType() const { return type == Crop; }
    inline bool isReservoirType() const { return type == Reservoir; }
    inline bool isRailType() const { return type == Rail; }
    inline bool isUnknownType() const { return type <= Unknown; }

    union {
        Hydro_ActuatorType actuatorType;                    // As actuator type enumeration
        Hydro_SensorType sensorType;                        // As sensor type enumeration
        Hydro_CropType cropType;                            // As crop type enumeration
        Hydro_ReservoirType reservoirType;                  // As reservoir type enumeration
        Hydro_RailType railType;                            // As rail type enumeration
    } objTypeAs;                                            // Enumeration type union
    Hydro_PositionIndex posIndex;                           // Position index
    String keyString;                                       // String key
    Hydro_KeyType key;                                      // UInt Key

    // Default constructor (incomplete id)
    HydroIdentity();

    // Copy key (incomplete id)
    HydroIdentity(Hydro_KeyType key);

    // Copy into keyStr (incomplete id)
    HydroIdentity(const char *idKeyStr);
    // Copy into keyStr (incomplete id)
    HydroIdentity(String idKey);

    // Copy id with new position index
    HydroIdentity(const HydroIdentity &id,
                  Hydro_PositionIndex positionIndex);

    // Actuator id constructor
    HydroIdentity(Hydro_ActuatorType actuatorType,
                  Hydro_PositionIndex positionIndex = HYDRO_POS_SEARCH_FROMBEG);
    // Sensor id constructor
    HydroIdentity(Hydro_SensorType sensorType,
                  Hydro_PositionIndex positionIndex = HYDRO_POS_SEARCH_FROMBEG);
    // Crop id constructor
    HydroIdentity(Hydro_CropType cropType,
                  Hydro_PositionIndex positionIndex = HYDRO_POS_SEARCH_FROMBEG);
    // Reservoir id constructor
    HydroIdentity(Hydro_ReservoirType reservoirType,
                  Hydro_PositionIndex positionIndex = HYDRO_POS_SEARCH_FROMBEG);
    // Rail id constructor
    HydroIdentity(Hydro_RailType railType,
                  Hydro_PositionIndex positionIndex = HYDRO_POS_SEARCH_FROMBEG);

    // Data constructor
    HydroIdentity(const HydroData *dataIn);

    // Used to update key value after modification, returning new key by convenience
    Hydro_KeyType regenKey();

    inline operator bool() const { return key != (Hydro_KeyType)-1; }
    inline bool operator==(const HydroIdentity &otherId) const { return key == otherId.key; }
    inline bool operator!=(const HydroIdentity &otherId) const { return key != otherId.key; }
};


// Hydroponic Object Base
// A simple base class for referring to objects in the Hydruino system.
class HydroObject : public HydroObjInterface {
public:
    inline bool isActuatorType() const { return _id.isActuatorType(); }
    inline bool isSensorType() const { return _id.isSensorType(); }
    inline bool isCropType() const { return _id.isCropType(); }
    inline bool isReservoirType() const { return _id.isReservoirType(); }
    inline bool isRailType() const { return _id.isRailType(); }
    inline bool isUnknownType() const { return _id.isUnknownType(); }

    HydroObject(HydroIdentity id);                          // Standard constructor
    HydroObject(const HydroData *dataIn);                   // Data constructor
    virtual ~HydroObject();                                 // Destructor

    virtual void update();                                  // Called over intervals of time by runloop
    virtual void handleLowMemory();                         // Called upon low memory condition to try and free memory up

    HydroData *newSaveData();                               // Saves object state to proper backing data

    void allocateLinkages(size_t size = 1);                 // Allocates linkage list of specified size (reallocates)
    virtual bool addLinkage(HydroObject *obj) override;     // Adds linkage to this object, returns true upon initial add
    virtual bool removeLinkage(HydroObject *obj) override;  // Removes linkage from this object, returns true upon last remove
    bool hasLinkage(HydroObject *obj) const;                // Checks object linkage to this object

    // Returns the linkages this object contains, along with refcount for how many times it has registered itself as linked (via attachment points).
    // Objects are considered strong pointers, since existence -> SharedPtr ref to this instance exists.
    inline Pair<uint8_t, Pair<HydroObject *, int8_t> *> getLinkages() const { return make_pair(_linksSize, _links); }

    virtual HydroIdentity getId() const override;           // Returns the unique Identity of the object
    virtual Hydro_KeyType getKey() const override;          // Returns the unique key of the object
    virtual String getKeyString() const override;           // Returns the key string of the object
    virtual SharedPtr<HydroObjInterface> getSharedPtr() const override; // Returns the SharedPtr instance of the object

protected:
    HydroIdentity _id;                                      // Object id
    uint8_t _linksSize;                                     // Size of object linkages
    Pair<HydroObject *, int8_t> *_links;                    // Object linkages (owned, lazily allocated)

    virtual HydroData *allocateData() const;                // Only up to base type classes (sensor, crop, etc.) does this need overriden
    virtual void saveToData(HydroData *dataOut);            // *ALL* derived classes must override and implement

private:
    HydroObject() = default;                                // Private constructor to disable derived/public access
};


// Sub Object Base
// A base class for sub objects that are typically found embedded in bigger main objects,
// but want to replicate some of the same functionality. Not required to be inherited from.
class HydroSubObject : public HydroObjInterface {
public:
    virtual HydroIdentity getId() const override;
    virtual Hydro_KeyType getKey() const override;
    virtual String getKeyString() const override;
    virtual SharedPtr<HydroObjInterface> getSharedPtr() const override;

    virtual bool addLinkage(HydroObject *obj) override;
    virtual bool removeLinkage(HydroObject *obj) override;
};


// Object Data Intermediate
// Intermediate data class for object data.
struct HydroObjectData : public HydroData {
    char name[HYDRO_NAME_MAXSIZE];

    HydroObjectData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroObject_H
