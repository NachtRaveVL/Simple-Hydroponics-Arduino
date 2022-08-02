/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Data
*/

#ifndef HydroponicsData_H
#define HydroponicsData_H

struct HydroponicsData;
struct HydroponicsSubData;

#include "Hydroponics.h"

// Serializes Hydroponics data structure to a binary output stream (essentially a memcpy), with optional skipBytes
extern size_t serializeDataToBinaryStream(const HydroponicsData *data, Stream *streamOut, size_t skipBytes = sizeof(void*));
// Deserializes Hydroponics data structure from a binary input stream (essentially a memcpy), with optional skipBytes
extern size_t deserializeDataFromBinaryStream(HydroponicsData *data, Stream *streamIn, size_t skipBytes = sizeof(void*));

// Creates a new hydroponics data object corresponding to a binary input stream (return ownership transfer - user code *must* delete returned data)
extern HydroponicsData *newDataFromBinaryStream(Stream *streamIn);
// Creates a new hydroponics data object corresponding to an input JSON element (return ownership transfer - user code *must* delete returned data)
extern HydroponicsData *newDataFromJSONObject(JsonObjectConst &objectIn);


// Hydroponics Data Base
// Base class for serializable (JSON+Binary) storage data, used to define the base
// header of all data stored internally.
// NOTE: NON-CONST VALUE TYPES ONLY. All data *MUST* be able to use default operator=.
struct HydroponicsData : public HydroponicsJSONSerializableInterface {
    union {
        char chars[4];                                          // Standalone data structure 4-char identifier
        struct {
          int8_t idType;                                        // Object ID type enum value (e.g. actuator, sensor, etc.)
          int8_t objType;                                       // Object type enum value (e.g. actuatorType, sensorType, etc.)
          int8_t posIndex;                                      // Object position index # (zero-ordinal)
          int8_t classType;                                     // Object class type enum value (e.g. pump, dht1w, etc.)
        } object;
    } id;                                                       // Identifier union
    uint16_t _size;                                             // The size (in bytes) of the data
    uint8_t _version;                                           // Version # of data container
    uint8_t _revision;                                          // Revision # of stored data
    bool _modified;                                             // Flag tracking modified status

    inline bool isStandardData() const { return id.chars[0] == 'H'; }
    inline bool isSystemData() const { return strncasecmp(id.chars, SFP(HStr_DataName_HSYS).c_str(), 4) == 0; }
    inline bool isCalibrationData() const { return strncasecmp(id.chars, SFP(HStr_DataName_HCAL).c_str(), 4) == 0; }
    inline bool isCropsLibData() const { return strncasecmp(id.chars, SFP(HStr_DataName_HCLD).c_str(), 4) == 0; }
    inline bool isAdditiveData() const { return strncasecmp(id.chars, SFP(HStr_DataName_HADD).c_str(), 4) == 0; }
    inline bool isObjectectData() const { return !isStandardData() && id.object.idType >= 0; }

    HydroponicsData();                                          // Default constructor
    HydroponicsData(const char *id,                             // 4-char identifier
                    uint8_t version = 1,                        // Data structure version #
                    uint8_t revision = 1);                      // Stored data revision #
    HydroponicsData(int8_t idType,                              // ID type enum value
                    int8_t objType,                             // Object type enum value
                    int8_t posIndex,                            // Object position index #
                    int8_t classType,                           // Class type enum value
                    uint8_t version = 1,                        // Data structure version #
                    uint8_t revision = 1);                      // Stored data revision #
    HydroponicsData(const HydroponicsIdentity &id);             // Identity constructor

    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;

    inline void _bumpRev() { _revision += 1; _setModded(); }
    inline void _bumpRevIfNotAlreadyModded() { if (!_modified) { _bumpRev(); } } // Should be called before modifying data
    inline void _setModded() { _modified = true; }              // Should be called after modifying any data
    inline void _unsetModded() { _modified = false; }           // Should be called after save-out
};


// Hydroponics Sub Data Base
// Sub-data exists inside of regular data for smaller objects that don't require the
// entire data object hierarchy, useful for triggers, measurements, etc.
// NOTE: NON-CONST VALUE TYPES ONLY, NO VIRTUALS. All data *MUST* be able to use default operator=.
struct HydroponicsSubData {
    int8_t type;

    HydroponicsSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};


// Internal use, but must contain all ways for all data types to be new'ed
extern HydroponicsData *_allocateDataFromBaseDecode(const HydroponicsData &baseDecode);
extern HydroponicsData *_allocateDataForObjType(int8_t idType, int8_t classType);

#endif // /ifndef HydroponicsData_H
