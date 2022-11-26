
/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Publisher
*/

#ifndef HydroponicsPublisher_H
#define HydroponicsPublisher_H

class HydroponicsPublisher;
struct HydroponicsPublisherSubData;
struct HydroponicsDataColumn;

#include "Hydroponics.h"

// Hydroponics Data Publisher
// The Publisher allows for data collection and publishing capabilities. The data output
// is based on a simple table of time and measured value. Each time segment, called a
// polling frame (and controlled by the polling rate interval), collects data from all
// sensors into a data row, with the appropriate total number of columns. At time of
// either all sensors having reported in for their frame #, or the frame # proceeding
// to advance (in which case the existing value is recycled), the table's row is
// submitted to the publishing service initially set up.
// Logging to SD card .csv data files (via SPI card reader) is supported as is logging to
// WiFiStorage .csv data files (via OS/OTA filesystem / WiFiNINA_Generic only).
class HydroponicsPublisher {
public:
    HydroponicsPublisher();
    ~HydroponicsPublisher();

    void update();
 
    bool beginPublishingToSDCard(String dataFilePrefix);
    inline bool isPublishingToSDCard() const;

#ifdef HYDRUINO_USE_WIFI_STORAGE
    bool beginPublishingToWiFiStorage(String dataFilePrefix);
    inline bool isPublishingToWiFiStorage() const;
#endif

#ifdef HYDRUINO_ENABLE_MQTT
    bool beginPublishingToMQTTClient(MQTTClient &client);
    inline bool isPublishingToMQTTClient() const;
#endif

    void publishData(Hydroponics_PositionIndex columnIndex, HydroponicsSingleMeasurement measurement);

    inline void setNeedsTabulation();
    inline bool needsTabulation() { return _needsTabulation; }

    inline bool isPublishingEnabled() const;
    Hydroponics_PositionIndex getColumnIndexStart(Hydroponics_KeyType sensorKey);

    Signal<Pair<uint8_t, const HydroponicsDataColumn *>, HYDRUINO_PUBLISH_STATE_SLOTS> &getPublishSignal();

    void notifyDayChanged();

protected:
#if HYDRUINO_SYS_LEAVE_FILES_OPEN
    File *_dataFileSD;                                      // SD Card log file instance (owned)
#ifdef HYDRUINO_USE_WIFI_STORAGE
    WiFiStorageFile *_dataFileWS;                           // WiFiStorageFile log file instance (owned)
#endif
#endif
#ifdef HYDRUINO_ENABLE_MQTT
    MQTTClient *_mqttClient;                                // MQTT client object (strong)
#endif
    String _dataFilename;                                   // Resolved data file name (based on day)
    uint16_t _pollingFrame;                                 // Polling frame that publishing is caught up to
    bool _needsTabulation;                                  // Needs tabulation tracking flag
    uint8_t _columnCount;                                   // Data columns count
    HydroponicsDataColumn *_dataColumns;                    // Data columns (owned)

    Signal<Pair<uint8_t, const HydroponicsDataColumn *>, HYDRUINO_PUBLISH_STATE_SLOTS> _publishSignal; // Data publishing signal

    friend class Hydroponics;

    inline HydroponicsPublisherSubData *publisherData() const;
    inline bool hasPublisherData() const;

    void advancePollingFrame();
    friend void dataLoop();

    void checkCanPublish();
    void publish(time_t timestamp);

    void performTabulation();

    void resetDataFile();
    void cleanupOldestData(bool force = false);
};

// Publisher Data Column
// Data column worth of storage. Intended to be array allocated.
struct HydroponicsDataColumn {
    Hydroponics_KeyType sensorKey;                          // Key to sensor object
    HydroponicsSingleMeasurement measurement;               // Storage polling frame measurement
};


// Publisher Serialization Sub Data
// A part of HSYS system data.
struct HydroponicsPublisherSubData : public HydroponicsSubData {
    char dataFilePrefix[16];                                // Base data file name prefix / folder (default: "data/hy")
    bool pubToSDCard;                                       // If publishing sensor data to SD card is enabled (default: false)
    bool pubToWiFiStorage;                                  // If publishing sensor data to WiFiStorage is enabled (default: false)

    HydroponicsPublisherSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef HydroponicsPublisher_H
