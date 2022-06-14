/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Interfaces
*/

#ifndef HydroponicsInterfaces_H
#define HydroponicsInterfaces_H

struct HydroponicsSerializableInterface;
struct HydroponicsJSONSerializableInterface;

#include "Hydroponics.h"

// Binary Serializable Interface
struct HydroponicsBinarySerializableInterface {
    // Given a byte stream to write to, writes self to binary format.
    virtual void toBinaryStream(Print *streamOut) const = 0;

    // Given a byte stream to read from, reads overtop self from binary format.
    virtual void fromBinaryStream(Stream *streamIn) = 0;
};

// JSON Serializable Interface
struct HydroponicsJSONSerializableInterface {
    // Given a JSON document to fill in, writes self to JSON format.
    virtual void toJSONDocument(JsonDocument *docOut) const = 0;

    // Given a JSON document to read from, reads overtop self from JSON format.
    virtual void fromJSONDocument(JsonDocument *docIn) = 0;
};

#endif // /ifndef HydroponicsInterfaces_H
