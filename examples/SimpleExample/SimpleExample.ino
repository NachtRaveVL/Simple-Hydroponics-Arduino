// Hydroponics-Arduino Simple Example

#include "Hydroponics.h"

Hydroponics hydroController;            // Controller using default XXX pin DXX, and default Wire @400kHz

void setup() {
    Serial.begin(115200);               // Begin Serial and Wire interfaces
    Wire.begin();
    Wire.setClock(hydroController.getI2CSpeed());

    hydroController.addGrowLightsRelay(22);
    hydroController.addWaterPumpRelay(24);

    hydroController.addCropFromSowDate(Hydroponics_CropType_Lettuce, DateTime(2022, 5, 21).unixtime());

    // Initializes controller using default XXX
    hydroController.init();
}

void loop() {
    // Update hydroponics controller
    hydroController.update();
}
