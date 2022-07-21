/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Publisher
*/

#include "Hydroponics.h"

HydroponicsPublisher::HydroponicsPublisher()
    : _publisherData(nullptr), _dataFileName(), _needsTabulation(false), _pollingFrame(0), _dataColumns(nullptr), _columnCount(0)
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

bool HydroponicsPublisher::beginPublishingToSDCard(String dataFilePrefix)
{
    auto hydroponics = getHydroponicsInstance();
    HYDRUINO_SOFT_ASSERT(_publisherData, SFP(HS_Err_NotYetInitialized));

    if (_publisherData) {
        auto sd = hydroponics->getSDCard();

        if (sd && sd->exists("/")) {
            String dataFileName = getYYMMDDFilename(dataFilePrefix, SFP(HS_csv));
            auto dataFile = sd->open(dataFileName, FILE_WRITE);

            if (dataFile && dataFile.availableForWrite()) {
                dataFile.close();
                hydroponics->endSDCard(sd);

                hydroponics->_systemData->_bumpRevIfNotAlreadyModded();
                strncpy(_publisherData->dataFilePrefix, dataFilePrefix.c_str(), 16);
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
    if (_dataColumns && _columnCount && columnIndex >= 0 && columnIndex < _columnCount) {
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
        _dataFileName = getYYMMDDFilename(stringFromChars(_publisherData->dataFilePrefix, 16), SFP(HS_csv));
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

    if (_dataColumns && _columnCount && hydroponics->getIsPollingFrameOld(_pollingFrame)) {
        bool allCurrent = true;

        for (int columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
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

                for (int columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
                    dataFile.print(',');
                    dataFile.print(_dataColumns[columnIndex].measurement.value);
                }

                dataFile.println();
            }

            if (dataFile) { dataFile.close(); }

            getHydroponicsInstance()->endSDCard(sd);
        }
    }
}

void HydroponicsPublisher::performTabulation()
{
    if (getIsPublishingEnabled()) {
        auto hydroponics = getHydroponicsInstance();
        bool sameOrder = _dataColumns && _columnCount ? true : false;
        int columnCount = 0;

        for (auto iter = hydroponics->_objects.begin(); iter != hydroponics->_objects.end(); ++iter) {
            if (iter->second && iter->second->isSensorType()) {
                auto sensor = static_pointer_cast<HydroponicsSensor>(iter->second);

                if (sensor) {
                    auto rowCount = getMeasurementRowCount(sensor->getLatestMeasurement());

                    for (int rowIndex = 0; sameOrder && rowIndex < rowCount; ++rowIndex) {
                        sameOrder = sameOrder && (columnCount + rowIndex + 1 <= _columnCount) &&
                                    (_dataColumns[columnCount + rowIndex].sensorKey == sensor->getKey());
                    }

                    columnCount += rowCount;
                }
            }
        }
        sameOrder = sameOrder && (columnCount == _columnCount);

        if (!sameOrder) {
            if (_dataColumns && _columnCount != columnCount) { delete [] _dataColumns; _dataColumns = nullptr; }
            _columnCount = columnCount;

            if (_columnCount) {
                if (!_dataColumns) {
                    _dataColumns = new HydroponicsDataColumn[_columnCount];
                    HYDRUINO_SOFT_ASSERT(_dataColumns, SFP(HS_Err_AllocationFailure));
                }

                if (_dataColumns) {
                    int columnIndex = 0;

                    for (auto iter = hydroponics->_objects.begin(); iter != hydroponics->_objects.end(); ++iter) {
                        if (iter->second && iter->second->isSensorType()) {
                            auto sensor = static_pointer_cast<HydroponicsSensor>(iter->second);

                            if (sensor) {
                                auto measurement = sensor->getLatestMeasurement();
                                auto rowCount = getMeasurementRowCount(measurement);

                                for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
                                    HYDRUINO_HARD_ASSERT(columnIndex < _columnCount, SFP(HS_Err_OperationFailure));
                                    _dataColumns[columnIndex].measurement = getAsSingleMeasurement(measurement, rowIndex);
                                    _dataColumns[columnIndex].sensorKey = sensor->getKey();
                                    columnIndex++;
                                }
                            }
                        }
                    }
                }
            }

            resetDataFile();
        }
    }

    _needsTabulation = false;
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

                for (int columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
                    dataFile.print(',');

                    auto sensor = (HydroponicsSensor *)(hydroponics->_objects[_dataColumns[columnIndex].sensorKey].get());
                    if (sensor && sensor == lastSensor) { ++measurementRow; }
                    else { measurementRow = 0; lastSensor = sensor; }

                    if (sensor) {
                        dataFile.print(sensor->getId().keyStr);
                        dataFile.print('_');
                        dataFile.print(unitsCategoryToString(defaultMeasureCategoryForSensorType(sensor->getSensorType(), measurementRow)));
                        dataFile.print('_');
                        dataFile.print(unitsTypeToSymbol(getMeasurementUnits(sensor->getLatestMeasurement(), measurementRow)));
                    } else {
                        HYDRUINO_SOFT_ASSERT(false, SFP(HS_Err_OperationFailure));
                        dataFile.print(SFP(HS_Undefined));
                    }
                }

                dataFile.println();
            }

            if (dataFile) { dataFile.close(); }

            hydroponics->endSDCard(sd);
        }
    }
}

void HydroponicsPublisher::cleanupOldestData(bool force)
{
    // TODO: Old data cleanup.
}


HydroponicsPublisherSubData::HydroponicsPublisherSubData()
    : HydroponicsSubData(), dataFilePrefix{0}, publishToSDCard(false)
{
    type = 0; // no type differentiation
}

void HydroponicsPublisherSubData::toJSONObject(JsonObject &objectOut) const
{
    //HydroponicsSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    if (dataFilePrefix[0]) { objectOut[SFP(HS_Key_DataFilePrefix)] = stringFromChars(dataFilePrefix, 16); }
    if (publishToSDCard != false) { objectOut[SFP(HS_Key_PublishToSDCard)] = publishToSDCard; }
}

void HydroponicsPublisherSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    //HydroponicsSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    const char *dataFilePrefixStr = objectIn[SFP(HS_Key_DataFilePrefix)];
    if (dataFilePrefixStr && dataFilePrefixStr[0]) { strncpy(dataFilePrefix, dataFilePrefixStr, 16); }
    publishToSDCard = objectIn[SFP(HS_Key_PublishToSDCard)] | publishToSDCard;
}
