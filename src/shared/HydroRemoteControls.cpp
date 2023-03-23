/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Remote Controls
*/

#include "HydruinoUI.h"
#ifdef HYDRO_USE_GUI

HydroRemoteSerialControl::HydroRemoteSerialControl(UARTDeviceSetup serialSetup)
    : _serialTransport(serialSetup.serial), _serialInitializer(), _serialConnection(_serialTransport, _serialInitializer)
{ ; }

BaseRemoteServerConnection *HydroRemoteSerialControl::getConnection()
{
    return &_serialConnection;
}


HydroRemoteSimhubControl::HydroRemoteSimhubControl(UARTDeviceSetup serialSetup, menuid_t statusMenuId)
    : _simhubConnection(serialSetup.serial, statusMenuId)
{ ; }

BaseRemoteServerConnection *HydroRemoteSimhubControl::getConnection()
{
    return &_simhubConnection;
}


#ifdef HYDRO_USE_WIFI

HydroRemoteWiFiControl::HydroRemoteWiFiControl(uint16_t listeningPort)
    : _rcServer(listeningPort), _netInitialisation(&_rcServer), _netTransport(), _netConnection(_netTransport, _netInitialisation)
{ ; }

BaseRemoteServerConnection *HydroRemoteWiFiControl::getConnection()
{
    return &_netConnection;
}

#endif // /ifdef HYDRO_USE_WIFI


#ifdef HYDRO_USE_ETHERNET

HydroRemoteEthernetControl::HydroRemoteEthernetControl(uint16_t listeningPort)
    : _rcServer(listeningPort), _netInitialisation(&_rcServer), _netTransport(), _netConnection(_netTransport, _netInitialisation)
{ ; }

BaseRemoteServerConnection *HydroRemoteEthernetControl::getConnection()
{
    return &_netConnection;
}

#endif // /ifdef HYDRO_USE_ETHERNET

#endif
