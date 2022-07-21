
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

// Hydroponics Publisher
class HydroponicsPublisher : public HydroponicsSubObject {
public:
    HydroponicsPublisher();
    virtual ~HydroponicsPublisher();
    void initFromData(HydroponicsPublisherSubData *dataIn);

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;
 
    bool beginPublishingToSDCard(String dataFilePrefix);
    bool getIsPublishingToSDCard();

    void publishData(Hydroponics_PositionIndex columnIndex, HydroponicsSingleMeasurement measurement);

    void setNeedsTabulation();

    Hydroponics_PositionIndex getColumnIndexStart(Hydroponics_KeyType sensorKey);
    bool getIsPublishingEnabled();

    void notifyDayChanged();

protected:
    HydroponicsPublisherSubData *_publisherData;            // Publisher data (strong, saved to storage via system data)

    String _dataFileName;                                   // Resolved data file name (based on day)
    bool _needsTabulation;                                  // Needs tabulation tracking flag
    uint16_t _pollingFrame;                                 // Polling frame that publishing is caught up to
    HydroponicsDataColumn *_dataColumns;                    // Data columns (owned)
    byte _columnsCount;                                     // Data columns count

    friend class Hydroponics;

    void advancePollingFrame();

    void checkCanPublish();
    void publish(time_t timestamp);

    void performTabulation();

    void resetDataFile();
    void cleanupOldestData(bool force = false);
};


struct HydroponicsDataColumn {
    Hydroponics_KeyType sensorKey;
    HydroponicsSingleMeasurement measurement;
};


// Publisher Serialization Sub Data
// A part of HSYS system data.
struct HydroponicsPublisherSubData : public HydroponicsSubData {
    char dataFilePrefix[HYDRUINO_NAME_MAXSIZE];
    bool publishToSDCard;

    HydroponicsPublisherSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef HydroponicsPublisher_H
