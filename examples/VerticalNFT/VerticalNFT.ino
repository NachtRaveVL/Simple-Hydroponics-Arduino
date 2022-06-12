// Simple-Hydroponics-Arduino Vertical Nutrient Film Technique (NFT) Example
// TODO

#include <Hydroponics.h>
#include <TaskScheduler.h>

// TODO

Hydroponics hydroController;            // Controller using default XXX TODO pin DXX, and default Wire @400kHz

void setup() {
    Serial.begin(115200);               // Begin Serial and Wire interfaces
    while(!Serial) { ; }                // Wait for USB serial to connect (remove in production)
    Wire.setClock(hydroController.getI2CSpeed()); // Don't worry, Wire.begin() gets called plenty enough times internally

    // Initializes controller with first initialization method that successfully returns using default XXX TODO
    if (!(hydroController.initFromSDCard() || hydroController.initFromEEPROM())) {
        // First time running controller, set up default initial environment.
        hydroController.init();

        // TODO show off features of controller
    }

    // Launches controller into main operation.
    hydroController.launch();
}

void loop()
{
    // Hydruino will manage most updates for us.
    hydroController.update();
}
