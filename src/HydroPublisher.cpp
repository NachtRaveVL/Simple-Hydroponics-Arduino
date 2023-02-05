/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Publisher
*/

#include "Hydruino.h"

HydroPublisher::HydroPublisher()
    : _dataFilename(), _needsTabulation(false), _pollingFrame(0), _dataColumns(nullptr), _columnCount(0)
#if HYDRO_SYS_LEAVE_FILES_OPEN
      , _dataFileSD(nullptr)
#ifdef HYDRO_USE_WIFI_STORAGE
      , _dataFileWS(nullptr)
#endif
#endif
#ifdef HYDRO_USE_MQTT
    , _mqttClient(nullptr)
#endif
{ ; }

HydroPublisher::~HydroPublisher()
{
    if (_dataColumns) { delete [] _dataColumns; _dataColumns = nullptr; }
    #if HYDRO_SYS_LEAVE_FILES_OPEN
        if (_dataFileSD) { _dataFileSD->flush(); _dataFileSD->close(); delete _dataFileSD; _dataFileSD = nullptr; }
        #ifdef HYDRO_USE_WIFI_STORAGE
            if (_dataFileWS) { _dataFileWS->close(); delete _dataFileWS; _dataFileWS = nullptr; }
        #endif
    #endif
    #ifdef HYDRO_USE_MQTT
        if (_mqttClient) {
            if (_mqttClient->connected()) { _mqttClient->disconnect(); }
            delete _mqttClient; _mqttClient = nullptr;
        }
    #endif
}

void HydroPublisher::update()
{
    if (hasPublisherData()) {
        if (_needsTabulation) { performTabulation(); }

        checkCanPublish();
    }
}

bool HydroPublisher::beginPublishingToSDCard(String dataFilePrefix)
{
    HYDRO_SOFT_ASSERT(hasPublisherData(), SFP(HStr_Err_NotYetInitialized));

    if (hasPublisherData() && !publisherData()->pubToSDCard) {
        auto sd = Hydruino::_activeInstance->getSDCard();

        if (sd) {
            String dataFilename = getYYMMDDFilename(dataFilePrefix, SFP(HStr_csv));
            createDirectoryFor(sd, dataFilename);
            #if HYDRO_SYS_LEAVE_FILES_OPEN
                auto &dataFile = _dataFileSD ? *_dataFileSD : *(_dataFileSD = new File(sd->open(dataFilename.c_str(), FILE_WRITE)));
            #else
                auto dataFile = sd->open(dataFilename.c_str(), FILE_WRITE);
            #endif

            if (dataFile) {
                #if !HYDRO_SYS_LEAVE_FILES_OPEN
                    dataFile.close();
                    Hydruino::_activeInstance->endSDCard(sd);
                #endif

                Hydruino::_activeInstance->_systemData->_bumpRevIfNotAlreadyModded();
                strncpy(publisherData()->dataFilePrefix, dataFilePrefix.c_str(), 16);
                publisherData()->pubToSDCard = true;
                _dataFilename = dataFilename;

                setNeedsTabulation();

                return true;
            }

            #if !HYDRO_SYS_LEAVE_FILES_OPEN
                Hydruino::_activeInstance->endSDCard(sd);
            #endif
        }
    }

    return false;
}

#ifdef HYDRO_USE_WIFI_STORAGE

bool HydroPublisher::beginPublishingToWiFiStorage(String dataFilePrefix)
{
    HYDRO_SOFT_ASSERT(hasPublisherData(), SFP(HStr_Err_NotYetInitialized));

    if (hasPublisherData() && !publisherData()->pubToWiFiStorage) {
        String dataFilename = getYYMMDDFilename(dataFilePrefix, SFP(HStr_csv));
        #if HYDRO_SYS_LEAVE_FILES_OPEN
            auto &dataFile = _dataFileWS ? *_dataFileWS : *(_dataFileWS = new WiFiStorageFile(WiFiStorage.open(dataFilename.c_str())));
        #else
            auto dataFile = WiFiStorage.open(dataFilename.c_str());
        #endif

        if (dataFile) {
            #if !HYDRO_SYS_LEAVE_FILES_OPEN
                dataFile.close();
            #endif

            Hydruino::_activeInstance->_systemData->_bumpRevIfNotAlreadyModded();
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
#ifdef HYDRO_USE_MQTT

static uint32_t mqttNow()
{
    return unixNow();
}

bool HydroPublisher::beginPublishingToMQTTClient(MQTTClient &client)
{
    HYDRO_SOFT_ASSERT(hasPublisherData(), SFP(HStr_Err_NotYetInitialized));

    if (hasPublisherData() && !_mqttClient) {
        _mqttClient = &client;
        _mqttClient->setClockSource(&mqttNow);
        if (!_mqttClient->connected()) {
            String unPw = String(F("public"));
            _mqttClient->connect(Hydruino::_activeInstance->getSystemName().c_str(),
                                 unPw.c_str(), unPw.c_str());
        }

        setNeedsTabulation();

        return true;
    }

    return false;
}

#endif

void HydroPublisher::publishData(Hydro_PositionIndex columnIndex, HydroSingleMeasurement measurement)
{
    HYDRO_SOFT_ASSERT(hasPublisherData() && _dataColumns && _columnCount, SFP(HStr_Err_NotYetInitialized));
    if (_dataColumns && _columnCount && columnIndex >= 0 && columnIndex < _columnCount) {
        _dataColumns[columnIndex].measurement = measurement;
        checkCanPublish();
    }
}

Hydro_PositionIndex HydroPublisher::getColumnIndexStart(Hydro_KeyType sensorKey)
{
    HYDRO_SOFT_ASSERT(hasPublisherData() && _dataColumns && _columnCount, SFP(HStr_Err_NotYetInitialized));
    if (_dataColumns && _columnCount) {
        for (int columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
            if (_dataColumns[columnIndex].sensorKey == sensorKey) {
                return (Hydro_PositionIndex)columnIndex;
            }
        }
    }
    return (Hydro_PositionIndex)-1;
}

Signal<Pair<uint8_t, const HydroDataColumn *>, HYDRO_PUBLISH_SIGNAL_SLOTS> &HydroPublisher::getPublishSignal()
{
    return _publishSignal;
}

void HydroPublisher::notifyDayChanged()
{
    if (isPublishingEnabled()) {
        _dataFilename = getYYMMDDFilename(charsToString(publisherData()->dataFilePrefix, 16), SFP(HStr_csv));
        cleanupOldestData();
    }
}

void HydroPublisher::advancePollingFrame()
{
    HYDRO_HARD_ASSERT(hasPublisherData(), SFP(HStr_Err_NotYetInitialized));

    auto pollingFrame = Hydruino::_activeInstance->getPollingFrame();

    if (pollingFrame && _pollingFrame != pollingFrame) {
        time_t timestamp = unixNow();
        _pollingFrame = pollingFrame;

        if (Hydruino::_activeInstance->inOperationalMode()) {
            #ifdef HYDRO_USE_MULTITASKING
                scheduleObjectMethodCallOnce<HydroPublisher>(this, &HydroPublisher::publish, timestamp);
            #else
                publish(timestamp);
            #endif
        }
    }

    if (++pollingFrame == 0) { pollingFrame = 1; } // use only valid frame #

    Hydruino::_activeInstance->_pollingFrame = pollingFrame;
}

void HydroPublisher::checkCanPublish()
{
    if (_dataColumns && _columnCount && Hydruino::_activeInstance->isPollingFrameOld(_pollingFrame)) {
        bool allCurrent = true;

        for (int columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
            if (Hydruino::_activeInstance->isPollingFrameOld(_dataColumns[columnIndex].measurement.frame)) {
                allCurrent = false;
                break;
            }
        }

        if (allCurrent) {
            time_t timestamp = unixNow();
            _pollingFrame = Hydruino::_activeInstance->getPollingFrame();

            if (Hydruino::_activeInstance->inOperationalMode()) {
                #ifdef HYDRO_USE_MULTITASKING
                    scheduleObjectMethodCallOnce<HydroPublisher>(this, &HydroPublisher::publish, timestamp);
                #else
                    publish(timestamp);
                #endif
            }
        }
    }
}

void HydroPublisher::publish(time_t timestamp)
{
    if (isPublishingToSDCard()) {
        auto sd = Hydruino::_activeInstance->getSDCard(HYDRO_LOFS_BEGIN);

        if (sd) {
            #if HYDRO_SYS_LEAVE_FILES_OPEN
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

                #if !HYDRO_SYS_LEAVE_FILES_OPEN
                    dataFile.flush();
                    dataFile.close();
                #endif
            }

            #if !HYDRO_SYS_LEAVE_FILES_OPEN
                Hydruino::_activeInstance->endSDCard(sd);
            #endif
        }
    }

#ifdef HYDRO_USE_WIFI_STORAGE

    if (isPublishingToWiFiStorage()) {
        #if HYDRO_SYS_LEAVE_FILES_OPEN
            auto &dataFile = _dataFileWS ? *_dataFileWS : *(_dataFileWS = new WiFiStorageFile(WiFiStorage.open(_dataFilename.c_str())));
        #else
            auto dataFile = WiFiStorage.open(_dataFilename.c_str());
        #endif

        if (dataFile) {
            auto dataFileStream = HydroWiFiStorageFileStream(dataFile, dataFile.size());
            dataFileStream.print(timestamp);

            for (int columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
                dataFileStream.print(',');
                dataFileStream.print(_dataColumns[columnIndex].measurement.value);
            }

            dataFileStream.println();
            #if !HYDRO_SYS_LEAVE_FILES_OPEN
                dataFile.close();
            #endif
        }
    }

#endif
#ifdef HYDRO_USE_MQTT

    if (isPublishingToMQTTClient()) {
        String systemName = Hydruino::_activeInstance->getSystemName();
        for (int columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
            auto sensor = (HydroSensor *)(Hydruino::_activeInstance->_objects[_dataColumns[columnIndex].sensorKey].get());
            if (sensor) {
                String topic; topic.reserve(systemName.length() + 1 + sensor->getKeyString().length() + 1);
                topic.concat(systemName);
                topic.concat('/');
                topic.concat(sensor->getKeyString());
                String payload = String(_dataColumns[columnIndex].measurement.value, 6); // skipping units/rounding/etc to allow MQTT broker full value data
                _mqttClient->publish(topic.c_str(), payload.c_str());
            }
        }
    }

#endif

    #ifdef HYDRO_USE_MULTITASKING
        scheduleSignalFireOnce<Pair<uint8_t, const HydroDataColumn *>>(_publishSignal, make_pair(_columnCount, &_dataColumns[0]));
    #else
        _publishSignal.fire(make_pair(_columnCount, &_dataColumns[0]));
    #endif
}

void HydroPublisher::performTabulation()
{
    HYDRO_SOFT_ASSERT(hasPublisherData(), SFP(HStr_Err_NotYetInitialized));

    bool sameOrder = _dataColumns && _columnCount ? true : false;
    int columnCount = 0;

    for (auto iter = Hydruino::_activeInstance->_objects.begin(); iter != Hydruino::_activeInstance->_objects.end(); ++iter) {
        if (iter->second->isSensorType()) {
            auto sensor = static_pointer_cast<HydroSensor>(iter->second);
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
                _dataColumns = new HydroDataColumn[_columnCount];
                HYDRO_SOFT_ASSERT(_dataColumns, SFP(HStr_Err_AllocationFailure));
            }
            if (_dataColumns) {
                int columnIndex = 0;

                for (auto iter = Hydruino::_activeInstance->_objects.begin(); iter != Hydruino::_activeInstance->_objects.end(); ++iter) {
                    if (iter->second->isSensorType()) {
                        auto sensor = static_pointer_cast<HydroSensor>(iter->second);
                        auto measurement = sensor->getLatestMeasurement();
                        auto rowCount = getMeasurementRowCount(measurement);

                        for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
                            HYDRO_HARD_ASSERT(columnIndex < _columnCount, SFP(HStr_Err_OperationFailure));
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

void HydroPublisher::resetDataFile()
{
    if (isPublishingToSDCard()) {
        auto sd = Hydruino::_activeInstance->getSDCard(HYDRO_LOFS_BEGIN);

        if (sd) {
            #if HYDRO_SYS_LEAVE_FILES_OPEN
                if (_dataFileSD) { _dataFileSD->flush(); _dataFileSD->close(); delete _dataFileSD; _dataFileSD = nullptr; }
            #endif
            if (sd->exists(_dataFilename.c_str())) {
                sd->remove(_dataFilename.c_str());
            }
            #if HYDRO_SYS_LEAVE_FILES_OPEN
                auto &dataFile = _dataFileSD ? *_dataFileSD : *(_dataFileSD = new File(sd->open(_dataFilename.c_str(), FILE_WRITE)));
            #else
                createDirectoryFor(sd, _dataFilename);
                auto dataFile = sd->open(_dataFilename.c_str(), FILE_WRITE);
            #endif

            if (dataFile) {
                HydroSensor *lastSensor = nullptr;
                uint8_t measurementRow = 0;

                dataFile.print(SFP(HStr_Key_Timestamp));

                for (int columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
                    dataFile.print(',');

                    auto sensor = (HydroSensor *)(Hydruino::_activeInstance->_objects[_dataColumns[columnIndex].sensorKey].get());
                    if (sensor && sensor == lastSensor) { ++measurementRow; }
                    else { measurementRow = 0; lastSensor = sensor; }

                    if (sensor) {
                        dataFile.print(sensor->getKeyString());
                        dataFile.print('_');
                        dataFile.print(unitsCategoryToString(defaultMeasureCategoryForSensorType(sensor->getSensorType(), measurementRow)));
                        dataFile.print('_');
                        dataFile.print(unitsTypeToSymbol(getMeasurementUnits(sensor->getLatestMeasurement(), measurementRow)));
                    } else {
                        HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
                        dataFile.print(SFP(HStr_Undefined));
                    }
                }

                dataFile.println();

                #if !HYDRO_SYS_LEAVE_FILES_OPEN
                    dataFile.flush();
                    dataFile.close();
                #endif
            }

            #if !HYDRO_SYS_LEAVE_FILES_OPEN
                Hydruino::_activeInstance->endSDCard(sd);
            #endif
        }
    }

#ifdef HYDRO_USE_WIFI_STORAGE

    if (isPublishingToWiFiStorage()) {
        #if HYDRO_SYS_LEAVE_FILES_OPEN
            if (_dataFileWS) { _dataFileWS->close(); delete _dataFileWS; _dataFileWS = nullptr; }
        #endif
        if (WiFiStorage.exists(_dataFilename.c_str())) {
            WiFiStorage.remove(_dataFilename.c_str());
        }
        #if HYDRO_SYS_LEAVE_FILES_OPEN
            auto &dataFile = _dataFileWS ? *_dataFileWS : *(_dataFileWS = new WiFiStorageFile(WiFiStorage.open(_dataFilename.c_str())));
        #else
            auto dataFile = WiFiStorage.open(_dataFilename.c_str());
        #endif

        if (dataFile) {
            auto dataFileStream = HydroWiFiStorageFileStream(dataFile);
            HydroSensor *lastSensor = nullptr;
            uint8_t measurementRow = 0;

            dataFileStream.print(SFP(HStr_Key_Timestamp));

            for (int columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
                dataFileStream.print(',');

                auto sensor = (HydroSensor *)(Hydruino::_activeInstance->_objects[_dataColumns[columnIndex].sensorKey].get());
                if (sensor && sensor == lastSensor) { ++measurementRow; }
                else { measurementRow = 0; lastSensor = sensor; }

                if (sensor) {
                    dataFileStream.print(sensor->getKeyString());
                    dataFileStream.print('_');
                    dataFileStream.print(unitsCategoryToString(defaultMeasureCategoryForSensorType(sensor->getSensorType(), measurementRow)));
                    dataFileStream.print('_');
                    dataFileStream.print(unitsTypeToSymbol(getMeasurementUnits(sensor->getLatestMeasurement(), measurementRow)));
                } else {
                    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
                    dataFileStream.print(SFP(HStr_Undefined));
                }
            }

            dataFileStream.println();
        }
    }

#endif
}

void HydroPublisher::cleanupOldestData(bool force)
{
    // TODO: Old data cleanup.
}


HydroPublisherSubData::HydroPublisherSubData()
    : HydroSubData(), dataFilePrefix{0}, pubToSDCard(false), pubToWiFiStorage(false)
{
    type = 0; // no type differentiation
}

void HydroPublisherSubData::toJSONObject(JsonObject &objectOut) const
{
    //HydroSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    if (dataFilePrefix[0]) { objectOut[SFP(HStr_Key_DataFilePrefix)] = charsToString(dataFilePrefix, 16); }
    if (pubToSDCard != false) { objectOut[SFP(HStr_Key_PublishToSDCard)] = pubToSDCard; }
    if (pubToWiFiStorage != false) { objectOut[SFP(HStr_Key_PublishToWiFiStorage)] = pubToWiFiStorage; }
}

void HydroPublisherSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    //HydroSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    const char *dataFilePrefixStr = objectIn[SFP(HStr_Key_DataFilePrefix)];
    if (dataFilePrefixStr && dataFilePrefixStr[0]) { strncpy(dataFilePrefix, dataFilePrefixStr, 16); }
    pubToSDCard = objectIn[SFP(HStr_Key_PublishToSDCard)] | pubToSDCard;
    pubToWiFiStorage = objectIn[SFP(HStr_Key_PublishToWiFiStorage)] | pubToWiFiStorage;
}
