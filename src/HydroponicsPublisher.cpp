/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Publisher
*/

#include "Hydroponics.h"

HydroponicsPublisher::HydroponicsPublisher()
    : _publisherData(nullptr), _dataFileName(), _needsTabulation(false)
{ ; }

HydroponicsPublisher::~HydroponicsPublisher()
{ ; }

void HydroponicsPublisher::initFromData(HydroponicsPublisherSubData *dataIn)
{
    _publisherData = dataIn;
    setNeedsTabulation();
}

void HydroponicsPublisher::update()
{
    if (_publisherData) {
        if (_needsTabulation) {
            performTabulation();
        }
    }
}

void HydroponicsPublisher::resolveLinks()
{
    setNeedsTabulation();
}

void HydroponicsPublisher::handleLowMemory()
{ ; }

bool HydroponicsPublisher::beginPublishingToSDCard(String csvFilePrefix)
{
    HYDRUINO_SOFT_ASSERT(_publisherData, SFP(HS_Err_NotYetInitialized));

    if (_publisherData) {
        auto sd = getHydroponicsInstance()->getSDCard();

        if (sd && sd->exists("/")) {
            String dataFileName = getYYMMDDFilename(csvFilePrefix, SFP(HS_txt));
            auto dataFile = sd->open(dataFileName, FILE_WRITE);
            if (dataFile && dataFile.availableForWrite()) {
                dataFile.close();
                getHydroponicsInstance()->endSDCard(sd);

                getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
                strncpy(_publisherData->csvFilePrefix, csvFilePrefix.c_str(), 16);
                _publisherData->publishToSDCard = true;
                _dataFileName = dataFileName;

                return true;
            }
            if (dataFile) { dataFile.close(); }
        }

        if (sd) { getHydroponicsInstance()->endSDCard(sd); }
    }
    return false;
}

bool HydroponicsPublisher::getIsPublishingToSDCard()
{
    HYDRUINO_SOFT_ASSERT(_publisherData, SFP(HS_Err_NotYetInitialized));
    return _publisherData && _publisherData->publishToSDCard;
}

void HydroponicsPublisher::setNeedsTabulation()
{
    _needsTabulation = (bool)_publisherData;
}

bool HydroponicsPublisher::getIsPublishingEnabled()
{
    HYDRUINO_SOFT_ASSERT(_publisherData, SFP(HS_Err_NotYetInitialized));
    return _publisherData && (_publisherData->publishToSDCard);
}

void HydroponicsPublisher::notifyNewFrame()
{ ; }

void HydroponicsPublisher::notifyDayChanged()
{
    if (getIsPublishingEnabled()) {
        regenDataFileName();
        cleanupOldestData();
    }
}

void HydroponicsPublisher::publish()
{
    if (getIsPublishingToSDCard()) {
        auto sd = getHydroponicsInstance()->getSDCard();

        if (sd) {
            auto dataFile = sd->open(_dataFileName, FILE_WRITE);
            if (dataFile && dataFile.availableForWrite()) {
                // for (int dataIndex = 0; dataIndex < _entriesCount; ++dataIndex) {
                //     if (dataIndex) { dataFile.print(','); }
                //     dataFile.print(_dataEntries[dataIndex]);
                // }
                dataFile.println();
                dataFile.close();
            } else if (dataFile) { dataFile.close(); }
            getHydroponicsInstance()->endSDCard(sd);
        }
    }
}

void HydroponicsPublisher::performTabulation()
{
    // TODO: Data table reallocation.
}

String HydroponicsPublisher::regenDataFileName()
{
    return _publisherData ? (_dataFileName = getYYMMDDFilename(stringFromChars(_publisherData->csvFilePrefix, 16), SFP(HS_csv))) : String();
}

void HydroponicsPublisher::cleanupOldestData(bool force)
{
    // TODO: Old data cleanup.
}


HydroponicsPublisherSubData::HydroponicsPublisherSubData()
    : HydroponicsSubData() // TODO
{ ; }

void HydroponicsPublisherSubData::toJSONObject(JsonObject &objectOut) const
{
    // purposeful no call to base method (ignores type)
    // TODO
}

void HydroponicsPublisherSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    // purposeful no call to base method (ignores type)
    // TODO
}
