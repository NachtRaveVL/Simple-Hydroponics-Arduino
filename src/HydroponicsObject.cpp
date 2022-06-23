/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Object
*/

#include "Hydroponics.h"

HydroponicsIdentity::HydroponicsIdentity()
    : type(Unknown), as{.actuatorType=(Hydroponics_ActuatorType)-1}, posIndex(-1), keyStr(""), key(-1)
{ ; }

HydroponicsIdentity::HydroponicsIdentity(const HydroponicsIdentity &id, Hydroponics_PositionIndex positionIndex)
    : type(id.type), as{.actuatorType=id.as.actuatorType}, posIndex(positionIndex)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(Hydroponics_ActuatorType actuatorTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Actuator), as{.actuatorType=actuatorTypeIn}, posIndex(positionIndex)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(Hydroponics_SensorType sensorTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Sensor), as{.sensorType=sensorTypeIn}, posIndex(positionIndex)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(Hydroponics_CropType cropTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Crop), as{.cropType=cropTypeIn}, posIndex(positionIndex)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(Hydroponics_ReservoirType reservoirTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Reservoir), as{.reservoirType=reservoirTypeIn}, posIndex(positionIndex)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(Hydroponics_RailType railTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Rail), as{.railType=railTypeIn}, posIndex(positionIndex)
{
    regenKey();
}

Hydroponics_KeyType HydroponicsIdentity::regenKey()
{
    switch(type) {
        case Actuator:
            keyStr = actuatorTypeToString(as.actuatorType, true) + positionIndexToString(posIndex, true);
            break;
        case Sensor:
            keyStr = sensorTypeToString(as.sensorType, true) + positionIndexToString(posIndex, true);
            break;
        case Crop:
            keyStr = cropTypeToString(as.cropType, true) + positionIndexToString(posIndex, true);
            break;
        case Reservoir:
            keyStr = reservoirTypeToString(as.reservoirType, true) + positionIndexToString(posIndex, true);
            break;
        case Rail:
            keyStr = railTypeToString(as.railType, true) + positionIndexToString(posIndex, true);
            break;
        case Unknown:
            keyStr = String();
            break;
    }
    return (key = stringHash(keyStr));
}


HydroponicsObject::HydroponicsObject(HydroponicsIdentity id)
    : _id(id), _links()
{ ; }

HydroponicsObject::~HydroponicsObject()
{ ; }

void HydroponicsObject::update()
{ ; }

void HydroponicsObject::resolveLinks()
{ ; }

void HydroponicsObject::handleLowMem()
{
    //_links.shrink_to_fit(); // not yet implemented library method
}

bool HydroponicsObject::hasLinkage(HydroponicsObject *obj) const
{
    return _links.find(obj->_id) != _links.end();
}

const HydroponicsIdentity HydroponicsObject::getId() const
{
    return _id;
}

const Hydroponics_KeyType HydroponicsObject::getKey() const
{
    return _id.key;
}

bool HydroponicsObject::addLinkage(HydroponicsObject *obj)
{
    return _links.insert(obj->getKey(), obj).second;
}

bool HydroponicsObject::removeLinkage(HydroponicsObject *obj)
{
    auto iter = _links.find(obj->_id);
    if (iter != _links.end()) {
        _links.erase(iter);
        return true;
    }
    return false;
}
