/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
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
        hid_t idType;                                       // As standard id type enumeration
    } objTypeAs;                                            // Object type union
    hposi_t posIndex;                                       // Position index
    String keyString;                                       // String key
    hkey_t key;                                             // UInt Key

    // Default/copy key (incomplete id)
    inline HydroIdentity(hkey_t key = -1) : type(Unknown), objTypeAs{.idType=Unknown}, posIndex(-1), keyString(), key(key) { ; }
    // Copy into keyStr (incomplete id)
    inline HydroIdentity(const char *idKeyStr) : type(Unknown), objTypeAs{.idType=Unknown}, posIndex(-1), keyString(idKeyStr), key(stringHash(idKeyStr)) { ; }
    // Copy into keyStr (incomplete id)
    inline HydroIdentity(String idKey) : type(Unknown), objTypeAs{.idType=Unknown}, posIndex(-1), keyString(idKey), key(stringHash(idKey.c_str())) { ; }

    // Copy id with new position index
    inline HydroIdentity(const HydroIdentity &id, hposi_t positionIndex) : type(id.type), objTypeAs{.idType=id.objTypeAs.idType}, posIndex(positionIndex), keyString(), key(hkey_none) { regenKey(); }

    // Actuator id constructor
    inline HydroIdentity(Hydro_ActuatorType actuatorTypeIn,
                         hposi_t positionIndex = HYDRO_POS_SEARCH_FROMBEG) : type(Actuator), objTypeAs{.actuatorType=actuatorTypeIn}, posIndex(positionIndex), keyString(), key(hkey_none) { regenKey(); }
    // Sensor id constructor
    inline HydroIdentity(Hydro_SensorType sensorTypeIn,
                         hposi_t positionIndex = HYDRO_POS_SEARCH_FROMBEG) : type(Sensor), objTypeAs{.sensorType=sensorTypeIn}, posIndex(positionIndex), keyString(), key(hkey_none) { regenKey(); }
    // Crop id constructor
    inline HydroIdentity(Hydro_CropType cropTypeIn,
                         hposi_t positionIndex = HYDRO_POS_SEARCH_FROMBEG) : type(Crop), objTypeAs{.cropType=cropTypeIn}, posIndex(positionIndex), keyString(), key(hkey_none) { regenKey(); }
    // Reservoir id constructor
    inline HydroIdentity(Hydro_ReservoirType reservoirTypeIn,
                         hposi_t positionIndex = HYDRO_POS_SEARCH_FROMBEG) : type(Reservoir), objTypeAs{.reservoirType=reservoirTypeIn}, posIndex(positionIndex), keyString(), key(hkey_none) { regenKey(); }
    // Rail id constructor
    inline HydroIdentity(Hydro_RailType railTypeIn,
                         hposi_t positionIndex = HYDRO_POS_SEARCH_FROMBEG) : type(Rail), objTypeAs{.railType=railTypeIn}, posIndex(positionIndex), keyString(), key(hkey_none) { regenKey(); }

    // Data constructor
    inline HydroIdentity(const HydroData *dataIn) : type((typeof(type))(dataIn->id.object.idType)), objTypeAs{.idType=dataIn->id.object.objType}, posIndex(dataIn->id.object.posIndex), keyString(), key(hkey_none) { regenKey(); }

    // Used to update key value after modification, returning new key by convenience
    hkey_t regenKey();

    inline operator bool() const { return key != hkey_none; }
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

    inline HydroObject(HydroIdentity id) : _id(id), _revision(-1), _linksSize(0), _links(nullptr) { ; }
    inline HydroObject(const HydroData *data) : _id(data), _revision(data->_revision), _linksSize(0), _links(nullptr) { ; }
    virtual ~HydroObject();

    // Called over intervals of time by runloop
    virtual void update();
    // Called upon low memory condition to try and free memory up
    virtual void handleLowMemory();

    // Saves object state to proper backing data
    HydroData *newSaveData();

    // (Re)allocates linkage list of specified size
    void allocateLinkages(size_t size = 1);
    // Adds linkage to this object, returns true upon initial add
    virtual bool addLinkage(HydroObject *obj);
    // Removes linkage from this object, returns true upon last remove
    virtual bool removeLinkage(HydroObject *obj);
    // Checks object linkage to this object
    bool hasLinkage(HydroObject *obj) const;

    // Returns the linkages this object contains, along with refcount for how many times it has registered itself as linked (via attachment points).
    // Objects are considered strong pointers, since existence -> SharedPtr ref to this instance exists.
    inline Pair<uint8_t, Pair<HydroObject *, int8_t> *> getLinkages() const { return make_pair(_linksSize, _links); }

    // Unresolves any dlinks to obj prior to caching
    virtual void unresolveAny(HydroObject *obj) override;
    // Unresolves this instance from any dlinks
    inline void unresolve() { unresolveAny(this); }

    // Returns the unique Identity of the object
    virtual HydroIdentity getId() const override;
    // Returns the unique key of the object
    virtual hkey_t getKey() const override;
    // Returns the key string of the object
    virtual String getKeyString() const override;
    // Returns the SharedPtr instance for this object
    virtual SharedPtr<HydroObjInterface> getSharedPtr() const override;
    // Returns the SharedPtr instance for passed object
    virtual SharedPtr<HydroObjInterface> getSharedPtrFor(const HydroObjInterface *obj) const override;
    // Returns true for object
    virtual bool isObject() const override;

    // Returns revision #
    inline uint8_t getRevision() const { return abs(_revision); }
    // If revision has been modified since last saved
    inline bool isModified() const { return _revision < 0; }
    // Bumps revision # if not already modified, and sets modified flag (called after modifying data)
    inline void bumpRevisionIfNeeded() { if (!isModified()) { _revision = -(abs(_revision) + 1); } }
    // Unsets modified flag from revision (called after save-out)
    inline void unsetModified() { _revision = abs(_revision); }

protected:
    HydroIdentity _id;                                      // Object id
    int8_t _revision;                                       // Revision # of stored data (uses -vals for modified flag)
    uint8_t _linksSize;                                     // Number of object linkages
    Pair<HydroObject *, int8_t> *_links;                    // Object linkages array (owned, lazily allocated/grown/shrunk)

    virtual HydroData *allocateData() const;                // Only up to base type classes (sensor, crop, etc.) does this need overriden
    virtual void saveToData(HydroData *dataOut);            // *ALL* derived classes must override and implement

private:
    // Private constructor to disable derived/public access
    inline HydroObject() : _id(), _revision(-1), _linksSize(0), _links(nullptr) { ; }
};


// Sub Object Base
// A base class for sub objects that are typically found embedded in bigger main objects,
// but want to replicate some of the same functionality. Not required to be inherited from.
class HydroSubObject : public HydroObjInterface {
public:
    inline HydroSubObject(HydroObjInterface *parent = nullptr) : _parent(parent) { ; }

    virtual void setParent(HydroObjInterface *parent);
    inline HydroObjInterface *getParent() const { return _parent; }

    virtual void unresolveAny(HydroObject *obj) override;

    virtual HydroIdentity getId() const override;
    virtual hkey_t getKey() const override;
    virtual String getKeyString() const override;
    virtual SharedPtr<HydroObjInterface> getSharedPtr() const override;
    virtual SharedPtr<HydroObjInterface> getSharedPtrFor(const HydroObjInterface *obj) const override;

    virtual bool isObject() const override;

    inline uint8_t getRevision() const { return _parent && _parent->isObject() ? ((HydroObject *)_parent)->getRevision() : 0; }
    inline bool isModified() const { return _parent && _parent->isObject() ? ((HydroObject *)_parent)->isModified() : false; }
    inline void bumpRevisionIfNeeded() { if (_parent && _parent->isObject()) { ((HydroObject *)_parent)->bumpRevisionIfNeeded(); } }
    inline void unsetModified() { ; }

protected:
    HydroObjInterface *_parent;                             // Parent object pointer (reverse ownership)
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
