// Simple-Hydroponics-Arduino Simple Deep Water Culture (DWC) Example
// TODO

#include <Hydroponics.h>

#define AeratorRelayPin     4
#define GrowLightsRelayPin  5
#define LettuceSowDate      DateTime(2022, 5, 21)

Hydroponics hydroController;            // Controller using default XXX TODO pin DXX, and default Wire @400kHz

void setup() {
    Serial.begin(115200);               // Begin Serial and Wire interfaces
    while(!Serial) { ; }                // Wait for USB serial to connect (remove in production)

    // Initializes controller with default environment, no logging, eeprom, SD, or anything else
    hydroController.init();

    // Adds our relay power rail as standard AC. This will manage how many active devices can be turned on at the same time.
    auto relayPower = hydroController.addSimplePowerRail(Hydroponics_RailType_AC110V);

    // Adds a 4 gallon main water reservoir, already filled with feed water.
    auto feedWater = hydroController.addFeedWaterReservoir(4, true);
    feedWater->setVolumeUnits(Hydroponics_UnitsType_LiqVolume_Gallons);

    // Add water aerator relay at AeratorRelayPin, and link it to the feed water reservoir and the relay power rail.
    auto aerator = hydroController.addWaterAeratorRelay(AeratorRelayPin);
    aerator->setRail(relayPower);
    aerator->setReservoir(feedWater);

    // Add grow lights relay at GrowLightsRelayPin, and link it to the feed water reservoir and the relay power rail.
    auto growLights = hydroController.addGrowLightsRelay(GrowLightsRelayPin);
    growLights->setRail(relayPower);
    growLights->setReservoir(feedWater);

    // Add some lettuce, set to feed on a standard 15 mins on/45 mins off timer, that we planted in clay pebbles on LettuceSowDate, and link it to the feed water reservoir.
    auto lettuce = hydroController.addTimerFedCrop(Hydroponics_CropType_Lettuce, Hydroponics_SubstrateType_ClayPebbles, LettuceSowDate);
    lettuce->setFeedReservoir(feedWater);

    // Launches controller into main operation.
    hydroController.launch();
}

void loop()
{
    // Hydruino will manage most updates for us.
    hydroController.update();
}
