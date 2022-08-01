/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Publisher
*/

#include "Hydroponics.h"

HydroponicsPublisher::HydroponicsPublisher()
    : _dataFileName(), _needsTabulation(false), _pollingFrame(0), _dataColumns(nullptr), _columnCount(0)
{ ; }

HydroponicsPublisher::~HydroponicsPublisher()
{
    if (_dataColumns) { delete [] _dataColumns; _dataColumns = nullptr; }
}

void HydroponicsPublisher::update()
{
    if (hasPublisherData()) {
        if (_needsTabulation) { performTabulation(); }

        checkCanPublish();
    }
}

bool HydroponicsPublisher::beginPublishingToSDCard(String dataFilePrefix)
{
    HYDRUINO_SOFT_ASSERT(hasPublisherData(), SFP(HStr_Err_NotYetInitialized));

    if (hasPublisherData()) {
        auto sd = Hydroponics::_activeInstance->getSDCard();

        if (sd) {
            String dataFileName = getYYMMDDFilename(dataFilePrefix, SFP(HStr_csv));
            createDirectoryFor(sd, dataFileName);
            auto dataFile = sd->open(dataFileName, FILE_WRITE);

            if (dataFile) {
                dataFile.close();
                Hydroponics::_activeInstance->endSDCard(sd);

                Hydroponics::_activeInstance->_systemData->_bumpRevIfNotAlreadyModded();
                strncpy(publisherData()->dataFilePrefix, dataFilePrefix.c_str(), 16);
                publisherData()->publishToSDCard = true;
                _dataFileName = dataFileName;

                setNeedsTabulation();

                return true;
            }
        }

        if (sd) { Hydroponics::_activeInstance->endSDCard(sd); }
    }
    return false;
}

void HydroponicsPublisher::publishData(Hydroponics_PositionIndex columnIndex, HydroponicsSingleMeasurement measurement)
{
    HYDRUINO_SOFT_ASSERT(hasPublisherData() && _dataColumns && _columnCount, SFP(HStr_Err_NotYetInitialized));
    if (_dataColumns && _columnCount && columnIndex >= 0 && columnIndex < _columnCount) {
        _dataColumns[columnIndex].measurement = measurement;
        checkCanPublish();
    }
}

Hydroponics_PositionIndex HydroponicsPublisher::getColumnIndexStart(Hydroponics_KeyType sensorKey)
{
    HYDRUINO_SOFT_ASSERT(hasPublisherData() && _dataColumns && _columnCount, SFP(HStr_Err_NotYetInitialized));
    if (_dataColumns && _columnCount) {
        for (int columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
            if (_dataColumns[columnIndex].sensorKey == sensorKey) {
                return (Hydroponics_PositionIndex)columnIndex;
            }
        }
    }
    return (Hydroponics_PositionIndex)-1;
}

void HydroponicsPublisher::notifyDayChanged()
{
    if (isPublishingEnabled()) {
        _dataFileName = getYYMMDDFilename(charsToString(publisherData()->dataFilePrefix, 16), SFP(HStr_csv));
        cleanupOldestData();
    }
}

void HydroponicsPublisher::advancePollingFrame()
{
    HYDRUINO_HARD_ASSERT(hasPublisherData(), SFP(HStr_Err_NotYetInitialized));

    auto pollingFrame = Hydroponics::_activeInstance->getPollingFrame();

    if (pollingFrame && _pollingFrame != pollingFrame) {
        time_t timestamp = unixNow();
        _pollingFrame = pollingFrame;

        if (Hydroponics::_activeInstance->inOperationalMode()) {
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleObjectMethodCallOnce<HydroponicsPublisher>(this, &HydroponicsPublisher::publish, timestamp);
            #else
                publish(timestamp);
            #endif
        }
    }

    if (++pollingFrame == 0) { pollingFrame = 1; } // use only valid frame #

    Hydroponics::_activeInstance->_pollingFrame = pollingFrame;
}

void HydroponicsPublisher::checkCanPublish()
{
    if (_dataColumns && _columnCount && Hydroponics::_activeInstance->isPollingFrameOld(_pollingFrame)) {
        bool allCurrent = true;

        for (int columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
            if (Hydroponics::_activeInstance->isPollingFrameOld(_dataColumns[columnIndex].measurement.frame)) {
                allCurrent = false;
                break;
            }
        }

        if (allCurrent) {
            time_t timestamp = unixNow();
            _pollingFrame = Hydroponics::_activeInstance->getPollingFrame();

            if (Hydroponics::_activeInstance->inOperationalMode()) {
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
    if (isPublishingToSDCard()) {
        auto sd = Hydroponics::_activeInstance->getSDCard();

        if (sd) {
            createDirectoryFor(sd, _dataFileName);
            auto dataFile = sd->open(_dataFileName, FILE_WRITE);

            if (dataFile) {
                dataFile.print(timestamp);

                for (int columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
                    dataFile.print(',');
                    dataFile.print(_dataColumns[columnIndex].measurement.value);
                }

                dataFile.println();
                dataFile.close();
            }

            Hydroponics::_activeInstance->endSDCard(sd);
        }
    }
}

void HydroponicsPublisher::performTabulation()
{
    HYDRUINO_SOFT_ASSERT(hasPublisherData(), SFP(HStr_Err_NotYetInitialized));

    bool sameOrder = _dataColumns && _columnCount ? true : false;
    int columnCount = 0;

    for (auto iter = Hydroponics::_activeInstance->_objects.begin(); iter != Hydroponics::_activeInstance->_objects.end(); ++iter) {
        if (iter->second->isSensorType()) {
            auto sensor = static_pointer_cast<HydroponicsSensor>(iter->second);
            auto rowCount = getMeasurementRowCount(sensor->getLatestMeasurement());

            for (int rowIndex = 0; sameOrder && rowIndex < rowCount; ++rowIndex) {
                sameOrder = sameOrder && (columnCount + rowIndex + 1 <= _columnCount) &&
                            (_dataColumns[columnCount + rowIndex].sensorKey == sensor->getKey());
            }

            columnCount += rowCount;
        }
    }
    sameOrder = sameOrder && (columnCount == _columnCount);

    if (!sameOrder) {
        if (_dataColumns && _columnCount != columnCount) { delete [] _dataColumns; _dataColumns = nullptr; }
        _columnCount = columnCount;

        if (_columnCount) {
            if (!_dataColumns) {
                _dataColumns = new HydroponicsDataColumn[_columnCount];
                HYDRUINO_SOFT_ASSERT(_dataColumns, SFP(HStr_Err_AllocationFailure));
            }
            if (_dataColumns) {
                int columnIndex = 0;

                for (auto iter = Hydroponics::_activeInstance->_objects.begin(); iter != Hydroponics::_activeInstance->_objects.end(); ++iter) {
                    if (iter->second->isSensorType()) {
                        auto sensor = static_pointer_cast<HydroponicsSensor>(iter->second);
                        auto measurement = sensor->getLatestMeasurement();
                        auto rowCount = getMeasurementRowCount(measurement);

                        for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
                            HYDRUINO_HARD_ASSERT(columnIndex < _columnCount, SFP(HStr_Err_OperationFailure));
                            _dataColumns[columnIndex].measurement = getAsSingleMeasurement(measurement, rowIndex);
                            _dataColumns[columnIndex].sensorKey = sensor->getKey();
                            columnIndex++;
                        }
                    }
                }
            }
        }

        resetDataFile();
    }

    _needsTabulation = false;
}

void HydroponicsPublisher::resetDataFile()
{
    if (isPublishingToSDCard()) {
        auto sd = Hydroponics::_activeInstance->getSDCard();

        if (sd) {
            if (sd->exists(_dataFileName)) {
                sd->remove(_dataFileName);
            }
            createDirectoryFor(sd, _dataFileName);
            auto dataFile = sd->open(_dataFileName, FILE_WRITE);

            if (dataFile) {
                HydroponicsSensor *lastSensor = nullptr;
                byte measurementRow = 0;

                dataFile.print(SFP(HStr_Key_Timestamp));

                for (int columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
                    dataFile.print(',');

                    auto sensor = (HydroponicsSensor *)(Hydroponics::_activeInstance->_objects[_dataColumns[columnIndex].sensorKey].get());
                    if (sensor && sensor == lastSensor) { ++measurementRow; }
                    else { measurementRow = 0; lastSensor = sensor; }

                    if (sensor) {
                        dataFile.print(sensor->getKeyString());
                        dataFile.print('_');
                        dataFile.print(unitsCategoryToString(defaultMeasureCategoryForSensorType(sensor->getSensorType(), measurementRow)));
                        dataFile.print('_');
                        dataFile.print(unitsTypeToSymbol(getMeasurementUnits(sensor->getLatestMeasurement(), measurementRow)));
                    } else {
                        HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
                        dataFile.print(SFP(HStr_Undefined));
                    }
                }

                dataFile.close();
            }

            Hydroponics::_activeInstance->endSDCard(sd);
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

    if (dataFilePrefix[0]) { objectOut[SFP(HStr_Key_DataFilePrefix)] = charsToString(dataFilePrefix, 16); }
    if (publishToSDCard != false) { objectOut[SFP(HStr_Key_PublishToSDCard)] = publishToSDCard; }
}

void HydroponicsPublisherSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    //HydroponicsSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    const char *dataFilePrefixStr = objectIn[SFP(HStr_Key_DataFilePrefix)];
    if (dataFilePrefixStr && dataFilePrefixStr[0]) { strncpy(dataFilePrefix, dataFilePrefixStr, 16); }
    publishToSDCard = objectIn[SFP(HStr_Key_PublishToSDCard)] | publishToSDCard;
}
