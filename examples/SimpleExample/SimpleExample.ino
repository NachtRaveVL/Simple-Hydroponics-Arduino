// Hydroponics-Arduino Simple Example

#include "Hydroponics.h"

Hydroponics hydroController;            // Controller using default chip select pin D10, and default Wire @400kHz

void setup() {
    Serial.begin(115200);               // Begin Serial, SPI, and Wire interfaces
#ifdef __SAM3X8E__
    // Arduino Due has SPI library that manages the CS pin for us
    SPI.begin(flirController.getChipSelectPin());
#else
    SPI.begin();
#endif
    Wire.begin();
    Wire.setClock(hydroController.getI2CSpeed());

    // Initializes module using Lepton v1 camera, and default celsius temperature mode
    // NOTE: Make sure to change this to what hardware camera version you're using! (see manufacturer website)
    hydroController.init(Hydroponics_CameraType_Lepton1);
}

void loop() {
    // Establishes sync, then reads next frame into raw data buffer
    if (hydroController.tryReadNextFrame())
        Serial.println("Frame read success");
    else
        Serial.println("Frame read failure");
}
