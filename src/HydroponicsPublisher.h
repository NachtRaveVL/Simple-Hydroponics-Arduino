
/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Publisher
*/

#ifndef HydroponicsPublisher_H
#define HydroponicsPublisher_H

class HydroponicsPublisher;
struct HydroponicsPublisherSubData;

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
 
    bool beginPublishingToSDCard(String csvFilePrefix);
    bool getIsPublishingToSDCard();

    void setNeedsTabulation();

    bool getIsPublishingEnabled();

    void notifyNewFrame();
    void notifyDayChanged();

protected:
    HydroponicsPublisherSubData *_publisherData;            // Publisher data (strong, saved to storage via system data)

    String _dataFileName;                                   // Resolved data file name (based on day)
    bool _needsTabulation;                                  // Needs tabulation tracking flag

    friend class Hydroponics;

    void publish();

    void performTabulation();
    String regenDataFileName();
    void cleanupOldestData(bool force = false);
};

// Publisher Serialization Sub Data
// A part of HSYS system data.
struct HydroponicsPublisherSubData : public HydroponicsSubData {
    char csvFilePrefix[HYDRUINO_NAME_MAXSIZE];
    bool publishToSDCard;

    HydroponicsPublisherSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef HydroponicsPublisher_H
