// Simple-Hydroponics-Arduino Simple Deep Water Culture (DWC) Example
// TODO

#include <Hydroponics.h>

const byte AeratorRelayPin = 7;
const DateTime LettuceSowDate = DateTime(2022, 5, 21);

Hydroponics hydroController;            // Controller using default XXX TODO pin DXX, and default Wire @400kHz

void setup() {
    Serial.begin(115200);               // Begin Serial and Wire interfaces
    while(!Serial) { ; }                // Wait for USB serial to connect (remove in production)
    Wire.setClock(hydroController.getI2CSpeed()); // Don't worry, Wire.begin() gets called plenty enough times internally

    // Initializes controller with default environment, no logging, eeprom, SD, or anything else
    hydroController.init();

    // Adds our relay power rail as standard AC. This will manage how many active devices can be turned on at the same time.
    auto relayPower = hydroController.addSimplePowerRail(Hydroponics_RailType_AC110V);

    // Adds the 4 gallon main water reservoir. This will contain sensors and feed crops.
    auto feedWater = hydroController.addFeedWaterReservoir(4);
    feedWater->setVolumeUnits(Hydroponics_UnitsType_LiqVolume_Gallons);

    // Add simple water aerator relay at AeratorRelayPin, and link it to the feed water reservoir and the relay power rail.
    auto aerator = hydroController.addWaterAeratorRelay(AeratorRelayPin);
    aerator->setRail(relayPower);
    aerator->setReservoir(feedWater);

    // Add some lettuce, set to feed on a timer, that we planted in clay pebbles on LettuceSowDate, and link it to the feed water reservoir.
    auto lettuce = hydroController.addTimerFedCrop((Hydroponics_CropType)0, Hydroponics_SubstrateType_ClayPebbles, LettuceSowDate.unixtime());
    lettuce->setFeedReservoir(feedWater);

    // Launches controller into main operation.
    hydroController.launch();
}

void loop()
{
    // Hydruino will manage most updates for us.
    hydroController.update();
}
