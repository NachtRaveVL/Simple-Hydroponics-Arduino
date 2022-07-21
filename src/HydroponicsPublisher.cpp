/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Publisher
*/

#include "Hydroponics.h"

HydroponicsPublisher::HydroponicsPublisher()
    : _publisherData(nullptr), _dataFileName(), _needsTabulation(false), _pollingFrame(0), _dataColumns(nullptr), _columnsCount(0)
{ ; }

HydroponicsPublisher::~HydroponicsPublisher()
{
    if (_dataColumns) { delete [] _dataColumns; _dataColumns = nullptr; }
}

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

        checkCanPublish();
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
    auto hydroponics = getHydroponicsInstance();
    HYDRUINO_SOFT_ASSERT(_publisherData, SFP(HS_Err_NotYetInitialized));

    if (_publisherData) {
        auto sd = hydroponics->getSDCard();

        if (sd && sd->exists("/")) {
            String dataFileName = getYYMMDDFilename(csvFilePrefix, SFP(HS_csv));
            auto dataFile = sd->open(dataFileName, FILE_WRITE);
            if (dataFile && dataFile.availableForWrite()) {
                dataFile.close();
                hydroponics->endSDCard(sd);

                hydroponics->_systemData->_bumpRevIfNotAlreadyModded();
                strncpy(_publisherData->csvFilePrefix, csvFilePrefix.c_str(), 16);
                _publisherData->publishToSDCard = true;
                _dataFileName = dataFileName;

                setNeedsTabulation();

                return true;
            }
            if (dataFile) { dataFile.close(); }
        }

        if (sd) { hydroponics->endSDCard(sd); }
    }
    return false;
}

bool HydroponicsPublisher::getIsPublishingToSDCard()
{
    HYDRUINO_SOFT_ASSERT(_publisherData, SFP(HS_Err_NotYetInitialized));
    return _publisherData && _publisherData->publishToSDCard;
}

void HydroponicsPublisher::publishData(Hydroponics_PositionIndex columnIndex, HydroponicsSingleMeasurement measurement)
{
    if (_dataColumns && _columnsCount && columnIndex >= 0 && columnIndex < _columnsCount) {
        _dataColumns[columnIndex].measurement = measurement;
        checkCanPublish();
    }
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

void HydroponicsPublisher::notifyDayChanged()
{
    if (getIsPublishingEnabled()) {
        _dataFileName = getYYMMDDFilename(stringFromChars(_publisherData->csvFilePrefix, 16), SFP(HS_csv));
        cleanupOldestData();
    }
}

void HydroponicsPublisher::advancePollingFrame()
{
    auto hydroponics = getHydroponicsInstance();
    auto pollingFrame = hydroponics->getPollingFrame();

    if (pollingFrame && _pollingFrame != pollingFrame) {
        time_t timestamp = now();
        _pollingFrame = pollingFrame;

        if (hydroponics->getInOperationalMode()) {
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleObjectMethodCallOnce<HydroponicsPublisher>(this, &HydroponicsPublisher::publish, timestamp);
            #else
                publish(timestamp);
            #endif
        }
    }

    if (++pollingFrame == 0) { pollingFrame = 1; } // use only valid frame #

    hydroponics->_pollingFrame = pollingFrame;
}

void HydroponicsPublisher::checkCanPublish()
{
    auto hydroponics = getHydroponicsInstance();

    if (_dataColumns && _columnsCount && hydroponics->getIsPollingFrameOld(_pollingFrame)) {
        bool allCurrent = true;
        for (int columnIndex = 0; columnIndex < _columnsCount; ++columnIndex) {
            if (hydroponics->getIsPollingFrameOld(_dataColumns[columnIndex].measurement.frame)) {
                allCurrent = false;
                break;
            }
        }
        if (allCurrent) {
            time_t timestamp = now();
            _pollingFrame = hydroponics->getPollingFrame();

            if (hydroponics->getInOperationalMode()) {
                #ifndef HYDRUINO_DISABLE_MULTITASKING
                    scheduleObjectMethodCallOnce<HydroponicsPublisher>(this, &HydroponicsPublisher::publish, timestamp);
                #else
                    publish(timestamp);
                #endif
            }
        }
    }
}

void HydroponicsPublisher::publish(time_t timestamp)
{
    if (getIsPublishingToSDCard()) {
        auto sd = getHydroponicsInstance()->getSDCard();

        if (sd) {
            auto dataFile = sd->open(_dataFileName, FILE_WRITE);
            if (dataFile && dataFile.availableForWrite()) {
                dataFile.print(timestamp);
                for (int columnIndex = 0; columnIndex < _columnsCount; ++columnIndex) {
                    dataFile.print(',');
                    dataFile.print(_dataColumns[columnIndex].measurement.value);
                }
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

void HydroponicsPublisher::resetDataFile()
{
    auto hydroponics = getHydroponicsInstance();

    if (getIsPublishingToSDCard()) {
        auto sd = hydroponics->getSDCard();

        if (sd) {
            if (sd->exists(_dataFileName)) {
                sd->remove(_dataFileName);
            }
            auto dataFile = sd->open(_dataFileName, FILE_WRITE);
            if (dataFile && dataFile.availableForWrite()) {
                HydroponicsSensor *lastSensor = nullptr;
                Hydroponics_PositionIndex measurementRow = 0;
                dataFile.print(SFP(HS_Key_Timestamp));
                for (int columnIndex = 0; columnIndex < _columnsCount; ++columnIndex) {
                    dataFile.print(',');

                    auto sensor = (HydroponicsSensor *)(hydroponics->_objects[_dataColumns[columnIndex].sensorKey].get());
                    if (sensor == lastSensor) { ++measurementRow; }
                    else { measurementRow = 0; lastSensor = sensor; }

                    dataFile.print(sensor->getId().keyStr);
                    dataFile.print('_');
                    dataFile.print(unitsCategoryToString(defaultMeasureCategoryForSensorType(sensor->getSensorType(), measurementRow)));
                    dataFile.print('_');
                    dataFile.print(unitsTypeToSymbol(getMeasurementUnits(sensor->getLatestMeasurement(), measurementRow)));
                }
                dataFile.println();
                dataFile.close();
            } else if (dataFile) { dataFile.close(); }
            hydroponics->endSDCard(sd);
        }
    }
}

void HydroponicsPublisher::cleanupOldestData(bool force)
{
    // TODO: Old data cleanup.
}


HydroponicsPublisherSubData::HydroponicsPublisherSubData()
    : HydroponicsSubData() // TODO
{
    type = 0; // no type differentiation
}

void HydroponicsPublisherSubData::toJSONObject(JsonObject &objectOut) const
{
    //HydroponicsSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)
    // TODO
}

void HydroponicsPublisherSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    //HydroponicsSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)
    // TODO
}
