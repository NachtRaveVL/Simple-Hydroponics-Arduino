// Simple-Hydroponics-Arduino Simple Deep Water Culture (DWC) Example
// TODO

#include <Hydroponics.h>

#define SETUP_GROW_LIGHTS_PIN       8               // Grow lights relay pin (digital)
#define SETUP_WATER_AERATOR_PIN     7               // Aerator relay pin (digital)
#define SETUP_FEED_RESERVOIR_SIZE   5               // Reservoir size, in default measurement units
#define SETUP_AC_POWER_RAIL_TYPE    AC110V          // Rail power type used for AC rail (AC110V, AC220V)

#define SETUP_CROP1_TYPE            Lettuce         // Type of crop planted at position 1, else Undefined
#define SETUP_CROP1_SUBSTRATE       ClayPebbles     // Type of crop substrate at position 1
#define SETUP_CROP1_SOW_DATE        DateTime(2022, 5, 21) // Date that crop was planted at position 1

Hydroponics hydroController;            // Controller using default settings

void setup() {
    Serial.begin(115200);               // Begin USB Serial interface
    while(!Serial) { ; }                // Wait for USB Serial to connect (remove in production)

    // Initializes controller with default environment, no logging, eeprom, SD, or anything else.
    hydroController.init();

    // Adds a simple relay power rail using standard AC. This will manage how many active devices can be turned on at the same time.
    auto relayPower = hydroController.addSimplePowerRail(JOIN(Hydroponics_RailType,SETUP_AC_POWER_RAIL_TYPE));

    // Adds a 5 unit main water reservoir, already filled with feed water.
    auto feedReservoir = hydroController.addFeedWaterReservoir(SETUP_FEED_RESERVOIR_SIZE, true);

    // Adds a water aerator at SETUP_WATER_AERATOR_PIN, and links it to the feed water reservoir and the relay power rail.
    auto aerator = hydroController.addWaterAeratorRelay(SETUP_WATER_AERATOR_PIN);
    aerator->setRail(relayPower);
    aerator->setReservoir(feedReservoir);

    // Add grow lights relay at SETUP_GROW_LIGHTS_PIN, and links it to the feed water reservoir and the relay power rail.
    auto lights = hydroController.addGrowLightsRelay(SETUP_GROW_LIGHTS_PIN);
    lights->setRail(relayPower);
    lights->setReservoir(feedReservoir);

    // Add timer fed crop set to feed on a standard 15 mins on/45 mins off timer, and links it to the feed water reservoir.
    auto crop = hydroController.addTimerFedCrop(JOIN(Hydroponics_CropType,SETUP_CROP1_TYPE),
                                                JOIN(Hydroponics_SubstrateType,SETUP_CROP1_SUBSTRATE),
                                                SETUP_CROP1_SOW_DATE);
    crop->setFeedReservoir(feedReservoir);

    // Launches controller into main operation.
    hydroController.launch();
}

void loop()
{
    // Hydruino will manage most updates for us.
    hydroController.update();
}
