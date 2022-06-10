// Simple-Hydroponics-Arduino Simple Deep Water Culture (DWC) Example
// TODO

#include <Hydroponics.h>
#include <TaskScheduler.h>

const byte AeratorRelayPin = 7;
const DateTime LettuceSowDate = DateTime(2022, 5, 21);

Hydroponics hydroController;            // Controller using default XXX TODO pin DXX, and default Wire @400kHz

void setup() {
    Serial.begin(115200);               // Begin Serial and Wire interfaces
    while(!Serial) { ; }                // Wait for USB serial to link up
    Wire.setClock(hydroController.getI2CSpeed()); // Don't worry, Wire.begin() gets called plenty enough times internally

    // Initializes controller with default environment, no logging, eeprom, SD, or anything else
    hydroController.init();

    // Add simple water aerator at AeratorRelayPin.
    hydroController.addWaterAeratorRelay(AeratorRelayPin);

    // Add some lettuce that we planted on LettuceSowDate.
    hydroController.addCropFromSowDate(Hydroponics_CropType_Lettuce, LettuceSowDate.unixtime());

    // Launches controller into main operation.
    hydroController.launch();
}

void loop()
{
    // Hydruino will manage all updates for us.
    hydroController.update();
}
