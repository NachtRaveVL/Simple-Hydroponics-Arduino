/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Remote Controls
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroRemoteControls_H
#define HydroRemoteControls_H

class HydroRemoteControl;
class HydroRemoteSerialControl;
class HydroRemoteSimhubControl;
#ifdef HYDRO_USE_WIFI
class HydroRemoteWiFiControl;
#endif
#ifdef HYDRO_USE_ETHERNET
class HydroRemoteEthernetControl;
#endif

#include "HydruinoUI.h"

// Remote Control Base
// Base remote control class.
class HydroRemoteControl {
public:
    virtual ~HydroRemoteControl() = default;

    virtual BaseRemoteServerConnection *getConnection() = 0;
};


// Serial UART Remote Control
// Manages remote control over serial UART.
class HydroRemoteSerialControl : public HydroRemoteControl {
public:
    HydroRemoteSerialControl(UARTDeviceSetup serialSetup);
    virtual ~HydroRemoteSerialControl() = default;

    virtual BaseRemoteServerConnection *getConnection() override;

protected:
    SerialTagValueTransport _serialTransport;
    NoInitialisationNeeded _serialInitializer;
    TagValueRemoteServerConnection _serialConnection;
};


// Simhub Connector Remote Control
// Manages remote control over simhub connector.
class HydroRemoteSimhubControl : public HydroRemoteControl {
public:
    HydroRemoteSimhubControl(UARTDeviceSetup serialSetup, menuid_t statusMenuId);
    virtual ~HydroRemoteSimhubControl() = default;

    virtual BaseRemoteServerConnection *getConnection() override;

protected:
    SimHubRemoteConnection _simhubConnection;
};


#ifdef HYDRO_USE_WIFI
// WiFi Remote Control
// Manages remote control over a WiFi connection.
class HydroRemoteWiFiControl : public HydroRemoteControl {
public:
    HydroRemoteWiFiControl(uint16_t listeningPort = HYDRO_UI_REMOTESERVER_PORT);
    virtual ~HydroRemoteWiFiControl() = default;

    virtual BaseRemoteServerConnection *getConnection() override;

    inline WiFiServer &getRCServer() { return _rcServer; }

protected:
    WiFiServer _rcServer;
    WiFiInitialisation _netInitialisation;
    WiFiTagValTransport _netTransport;
    TagValueRemoteServerConnection _netConnection;
};
#endif


#ifdef HYDRO_USE_ETHERNET
// Ethernet Remote Control
// Manages remote control over an Ethernet connection.
class HydroRemoteEthernetControl : public HydroRemoteControl {
public:
    HydroRemoteEthernetControl(uint16_t listeningPort = HYDRO_UI_REMOTESERVER_PORT);
    virtual ~HydroRemoteEthernetControl() = default;

    virtual BaseRemoteServerConnection *getConnection() override;

    inline EthernetServer &getRCServer() { return _rcServer; }

protected:
    EthernetServer _rcServer;
    EthernetInitialisation _netInitialisation;
    EthernetTagValTransport _netTransport;
    TagValueRemoteServerConnection _netConnection;
};
#endif

#endif // /ifndef HydroRemoteControls_H
#endif
