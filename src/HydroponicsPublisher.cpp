/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Publisher
*/

#include "Hydroponics.h"

HydroponicsPublisher::HydroponicsPublisher()
    : _dataFilename(), _needsTabulation(false), _pollingFrame(0), _dataColumns(nullptr), _columnCount(0)
{ ; }

HydroponicsPublisher::~HydroponicsPublisher()
{
    if (_dataColumns) { delete [] _dataColumns; _dataColumns = nullptr; }
    #if HYDRUINO_SYS_LEAVE_FILES_OPEN
        if (_dataFileSD) { _dataFileSD->flush(); _dataFileSD->close(); delete _dataFileSD; _dataFileSD = nullptr; }
        #ifdef HYDRUINO_USE_WIFI_STORAGE
            if (_dataFileWS) { _dataFileWS->close(); delete _dataFileWS; _dataFileWS = nullptr; }
        #endif
    #endif
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

    if (hasPublisherData() && !publisherData()->pubToSDCard) {
        auto sd = Hydroponics::_activeInstance->getSDCard();

        if (sd) {
            String dataFilename = getYYMMDDFilename(dataFilePrefix, SFP(HStr_csv));
            createDirectoryFor(sd, dataFilename);
            #if HYDRUINO_SYS_LEAVE_FILES_OPEN
                auto &dataFile = _dataFileSD ? *_dataFileSD : *(_dataFileSD = new File(sd->open(dataFilename.c_str(), FILE_WRITE)));
            #else
                auto dataFile = sd->open(dataFilename.c_str(), FILE_WRITE);
            #endif

            if (dataFile) {
                #if !HYDRUINO_SYS_LEAVE_FILES_OPEN
                    dataFile.close();
                    Hydroponics::_activeInstance->endSDCard(sd);
                #endif

                Hydroponics::_activeInstance->_systemData->_bumpRevIfNotAlreadyModded();
                strncpy(publisherData()->dataFilePrefix, dataFilePrefix.c_str(), 16);
                publisherData()->pubToSDCard = true;
                _dataFilename = dataFilename;

                setNeedsTabulation();

                return true;
            }

            #if !HYDRUINO_SYS_LEAVE_FILES_OPEN
                Hydroponics::_activeInstance->endSDCard(sd);
            #endif
        }
    }

    return false;
}

#ifdef HYDRUINO_USE_WIFI_STORAGE

bool HydroponicsPublisher::beginPublishingToWiFiStorage(String dataFilePrefix)
{
    HYDRUINO_SOFT_ASSERT(hasPublisherData(), SFP(HStr_Err_NotYetInitialized));

    if (hasPublisherData() && !publisherData()->pubToWiFiStorage) {
        String dataFilename = getYYMMDDFilename(dataFilePrefix, SFP(HStr_csv));
        #if HYDRUINO_SYS_LEAVE_FILES_OPEN
            auto &dataFile = _dataFileWS ? *_dataFileWS : *(_dataFileWS = new WiFiStorageFile(WiFiStorage.open(dataFilename.c_str())));
        #else
            auto dataFile = WiFiStorage.open(dataFilename.c_str());
        #endif

        if (dataFile) {
            #if !HYDRUINO_SYS_LEAVE_FILES_OPEN
                dataFile.close();
            #endif

            Hydroponics::_activeInstance->_systemData->_bumpRevIfNotAlreadyModded();
            strncpy(publisherData()->dataFilePrefix, dataFilePrefix.c_str(), 16);
            publisherData()->pubToWiFiStorage = true;
            _dataFilename = dataFilename;

            setNeedsTabulation();

            return true;
        }
    }

    return false;
}

#endif

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

Signal<Pair<uint8_t, const HydroponicsDataColumn *>, HYDRUINO_PUBLISH_STATE_SLOTS> &HydroponicsPublisher::getPublishSignal()
{
    return _publishSignal;
}

void HydroponicsPublisher::notifyDayChanged()
{
    if (isPublishingEnabled()) {
        _dataFilename = getYYMMDDFilename(charsToString(publisherData()->dataFilePrefix, 16), SFP(HStr_csv));
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
        auto sd = Hydroponics::_activeInstance->getSDCard(HYDRUINO_LOFS_BEGIN);

        if (sd) {
            #if HYDRUINO_SYS_LEAVE_FILES_OPEN
                auto &dataFile = _dataFileSD ? *_dataFileSD : *(_dataFileSD = new File(sd->open(_dataFilename.c_str(), FILE_WRITE)));
            #else
                createDirectoryFor(sd, _dataFilename);
                auto dataFile = sd->open(_dataFilename.c_str(), FILE_WRITE);
            #endif

            if (dataFile) {
                dataFile.print(timestamp);

                for (int columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
                    dataFile.print(',');
                    dataFile.print(_dataColumns[columnIndex].measurement.value);
                }

                dataFile.println();

                #if !HYDRUINO_SYS_LEAVE_FILES_OPEN
                    dataFile.flush();
                    dataFile.close();
                #endif
            }

            #if !HYDRUINO_SYS_LEAVE_FILES_OPEN
                Hydroponics::_activeInstance->endSDCard(sd);
            #endif
        }
    }

#ifdef HYDRUINO_USE_WIFI_STORAGE

    if (isPublishingToWiFiStorage()) {
        #if HYDRUINO_SYS_LEAVE_FILES_OPEN
            auto &dataFile = _dataFileWS ? *_dataFileWS : *(_dataFileWS = new WiFiStorageFile(WiFiStorage.open(_dataFilename.c_str())));
        #else
            auto dataFile = WiFiStorage.open(_dataFilename.c_str());
        #endif

        if (dataFile) {
            auto dataFileStream = HydroponicsWiFiStorageFileStream(dataFile, dataFile.size());
            dataFileStream.print(timestamp);

            for (int columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
                dataFileStream.print(',');
                dataFileStream.print(_dataColumns[columnIndex].measurement.value);
            }

            dataFileStream.println();
            #if !HYDRUINO_SYS_LEAVE_FILES_OPEN
                dataFile.close();
            #endif
        }
    }

#endif

    #ifndef HYDRUINO_DISABLE_MULTITASKING
        scheduleSignalFireOnce<Pair<uint8_t, const HydroponicsDataColumn *>>(_publishSignal, make_pair(_columnCount, &_dataColumns[0]));
    #else
        _publishSignal.fire(make_pair(_columnCount, &_dataColumns[0]));
    #endif
}

void HydroponicsPublisher::performTabulation()
{
    HYDRUINO_SOFT_ASSERT(hasPublisherData(), SFP(HStr_Err_NotYetInitialized));

    bool sameOrder = _dataColumns && _columnCount ? true : false;
    int columnCount = 0;

    for (auto iter = Hydroponics::_activeInstance->_objects.begin(); iter != Hydroponics::_activeInstance->_objects.end(); ++iter) {
        if (iter->second->isSensorType()) {
            auto sensor = hy_static_ptr_cast<HydroponicsSensor>(iter->second);
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
                        auto sensor = hy_static_ptr_cast<HydroponicsSensor>(iter->second);
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
        auto sd = Hydroponics::_activeInstance->getSDCard(HYDRUINO_LOFS_BEGIN);

        if (sd) {
            #if HYDRUINO_SYS_LEAVE_FILES_OPEN
                if (_dataFileSD) { _dataFileSD->flush(); _dataFileSD->close(); delete _dataFileSD; _dataFileSD = nullptr; }
            #endif
            if (sd->exists(_dataFilename.c_str())) {
                sd->remove(_dataFilename.c_str());
            }
            #if HYDRUINO_SYS_LEAVE_FILES_OPEN
                auto &dataFile = _dataFileSD ? *_dataFileSD : *(_dataFileSD = new File(sd->open(_dataFilename.c_str(), FILE_WRITE)));
            #else
                createDirectoryFor(sd, _dataFilename);
                auto dataFile = sd->open(_dataFilename.c_str(), FILE_WRITE);
            #endif

            if (dataFile) {
                HydroponicsSensor *lastSensor = nullptr;
                uint8_t measurementRow = 0;

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

                dataFile.println();

                #if !HYDRUINO_SYS_LEAVE_FILES_OPEN
                    dataFile.flush();
                    dataFile.close();
                #endif
            }

            #if !HYDRUINO_SYS_LEAVE_FILES_OPEN
                Hydroponics::_activeInstance->endSDCard(sd);
            #endif
        }
    }

#ifdef HYDRUINO_USE_WIFI_STORAGE

    if (isPublishingToWiFiStorage()) {
        #if HYDRUINO_SYS_LEAVE_FILES_OPEN
            if (_dataFileWS) { _dataFileWS->close(); delete _dataFileWS; _dataFileWS = nullptr; }
        #endif
        if (WiFiStorage.exists(_dataFilename.c_str())) {
            WiFiStorage.remove(_dataFilename.c_str());
        }
        #if HYDRUINO_SYS_LEAVE_FILES_OPEN
            auto &dataFile = _dataFileWS ? *_dataFileWS : *(_dataFileWS = new WiFiStorageFile(WiFiStorage.open(_dataFilename.c_str())));
        #else
            auto dataFile = WiFiStorage.open(_dataFilename.c_str());
        #endif

        if (dataFile) {
            auto dataFileStream = HydroponicsWiFiStorageFileStream(dataFile);
            HydroponicsSensor *lastSensor = nullptr;
            uint8_t measurementRow = 0;

            dataFileStream.print(SFP(HStr_Key_Timestamp));

            for (int columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
                dataFileStream.print(',');

                auto sensor = (HydroponicsSensor *)(Hydroponics::_activeInstance->_objects[_dataColumns[columnIndex].sensorKey].get());
                if (sensor && sensor == lastSensor) { ++measurementRow; }
                else { measurementRow = 0; lastSensor = sensor; }

                if (sensor) {
                    dataFileStream.print(sensor->getKeyString());
                    dataFileStream.print('_');
                    dataFileStream.print(unitsCategoryToString(defaultMeasureCategoryForSensorType(sensor->getSensorType(), measurementRow)));
                    dataFileStream.print('_');
                    dataFileStream.print(unitsTypeToSymbol(getMeasurementUnits(sensor->getLatestMeasurement(), measurementRow)));
                } else {
                    HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
                    dataFileStream.print(SFP(HStr_Undefined));
                }
            }

            dataFileStream.println();
        }
    }

#endif
}

void HydroponicsPublisher::cleanupOldestData(bool force)
{
    // TODO: Old data cleanup.
}


HydroponicsPublisherSubData::HydroponicsPublisherSubData()
    : HydroponicsSubData(), dataFilePrefix{0}, pubToSDCard(false), pubToWiFiStorage(false)
{
    type = 0; // no type differentiation
}

void HydroponicsPublisherSubData::toJSONObject(JsonObject &objectOut) const
{
    //HydroponicsSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    if (dataFilePrefix[0]) { objectOut[SFP(HStr_Key_DataFilePrefix)] = charsToString(dataFilePrefix, 16); }
    if (pubToSDCard != false) { objectOut[SFP(HStr_Key_PublishToSDCard)] = pubToSDCard; }
    if (pubToWiFiStorage != false) { objectOut[SFP(HStr_Key_PublishToWiFiStorage)] = pubToWiFiStorage; }
}

void HydroponicsPublisherSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    //HydroponicsSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    const char *dataFilePrefixStr = objectIn[SFP(HStr_Key_DataFilePrefix)];
    if (dataFilePrefixStr && dataFilePrefixStr[0]) { strncpy(dataFilePrefix, dataFilePrefixStr, 16); }
    pubToSDCard = objectIn[SFP(HStr_Key_PublishToSDCard)] | pubToSDCard;
    pubToWiFiStorage = objectIn[SFP(HStr_Key_PublishToWiFiStorage)] | pubToWiFiStorage;
}
