/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    tcMenu WiFi Remote Control
*/
#include <Hydruino.h>
#if defined(HYDRO_USE_WIFI) && defined(HYDRO_USE_GUI)

/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * WiFi remote capability plugin. This file is a plugin file and should not be directly edited,
 * it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 */

#include "tcMenu_Remote_WiFiTransport.h"

using namespace tcremote;

#if WIFI_BUFFER_SIZE > 0 // we need buffering when dealing with WiFi2

bool WiFiTagValTransport::available() {
	return client && client.connected();
}

bool WiFiTagValTransport::connected() {
	return client && client.connected();
}

void WiFiTagValTransport::flush() {
    if(!client || writeBufferPos == 0) return;

    if((int)client.write(writeBuffer, writeBufferPos) == writeBufferPos) {
        serdebugF2("Buffer written ", writeBufferPos);
        writeBufferPos = 0;
        client.flush();
    }
    else {
        writeBufferPos = 0;
        close();
    }
}

int WiFiTagValTransport::fillReadBuffer(uint8_t* dataBuffer, int maxData) {
    if(client && client.connected() && client.available()) {
        auto amt = client.read(dataBuffer, maxData);
        if(amt <= 0) {
            close();
            return 0;
        }
        serdebugF2("read to buffer ", amt);
        return amt;
    }
    return 0;
}

void WiFiTagValTransport::close() {
    serdebugF("socket close");
    BaseBufferedRemoteTransport::close();
    client.stop();
}

#else // unbuffed client - requires library to support Nagle algorythm.

bool WiFiTagValTransport::available() {
	return client && client.connected();
}

bool WiFiTagValTransport::connected() {
	return client && client.connected();
}

int WiFiTagValTransport::writeChar(char data) {
    // only uncomment below for worst case debugging..
//	serdebug2("writing ", data);
	return client.write(data);
}

int WiFiTagValTransport::writeStr(const char* data) {
    // only uncomment below for worst case debugging..
//	serdebug2("writing ", data);
	return client.write(data);
}

void WiFiTagValTransport::flush() {
	if(client) client.flush();
}

uint8_t WiFiTagValTransport::readByte() {
	return client.read();
}

bool WiFiTagValTransport::readAvailable() {
	return client && client.connected() && client.available();
}

void WiFiTagValTransport::close() {
    serdebugF("socket close");
    client.stop();
    currentField.msgType = UNKNOWN_MSG_TYPE;
    currentField.fieldType = FVAL_PROCESSING_AWAITINGMSG;
}

#endif

bool WiFiInitialisation::attemptInitialisation() {
#ifdef ARDUINO_ARCH_STM32
    // we'll keep checking if the link is up before trying to initialise further
    if(WiFi.status() != WL_CONNECTED) return false;
#endif
    serdebugF("Initialising server ");
    this->server->begin();
    initialised = true;
    return initialised;
}

bool WiFiInitialisation::attemptNewConnection(BaseRemoteServerConnection *remoteServerConnection) {
    auto client = server->available();
    if(client) {
        serdebugF("Client found");
        auto* tvCon = reinterpret_cast<TagValueRemoteServerConnection*>(remoteServerConnection);
        reinterpret_cast<WiFiTagValTransport*>(tvCon->transport())->setClient(client);
        return true;
    }
    return false;
}

#endif
