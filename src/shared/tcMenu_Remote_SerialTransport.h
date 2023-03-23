/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    tcMenu Serial Remote Control
*/
#include <Hydruino.h>
#ifdef HYDRO_USE_GUI

/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/* Changelist:
 * - Enclosed inside of #ifdef & reorg'ed for general inclusion
 */

/**
 * @file tcMenu_Remote_SerialTransport.h
 * 
 * Serial remote capability plugin. This file is a plugin file and should not be directly edited,
 * it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 */

#ifndef TCMENU_SERIALTRANSPORT_H_
#define TCMENU_SERIALTRANSPORT_H_

#include <RemoteConnector.h>
#include <RemoteAuthentication.h>
#include <MessageProcessors.h>
#include <tcUtil.h>
#include <remote/BaseRemoteComponents.h>

namespace tcremote {

    /**
     * Serial transport is an implementation of TagValueTransport that works over a serial port
     */
    class SerialTagValueTransport : public TagValueTransport {
    private:
        Stream* serialPort;
    public:
        explicit SerialTagValueTransport(Stream* thePort);
        ~SerialTagValueTransport() override = default;

        void flush() override {serialPort->flush();}
        int writeChar(char data) override;
        int writeStr(const char* data) override;

        uint8_t readByte() override { return serialPort->read(); }
        bool readAvailable() override { return serialPort->available(); }
        bool available() override { return serialPort->availableForWrite() != 0;}
        bool connected() override { return true;}

        void close() override;
    };
}

#ifndef TC_MANUAL_NAMESPACING
using namespace tcremote;
#endif // TC_MANUAL_NAMESPACING

#endif /* TCMENU_SERIALTRANSPORT_H_ */
#endif
