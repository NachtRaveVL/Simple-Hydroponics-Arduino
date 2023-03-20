/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Menu Screens
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroMenus_H
#define HydroMenus_H

class HydroMenu;
class HydroHomeMenu;
class HydroAlertsMenu;
class HydroActuatorsMenu;
class HydroSensorsMenu;
class HydroCropsMenu;
class HydroReservoirsMenu;
class HydroPowerRailsMenu;
class HydroSchedulingMenu;
class HydroCropsLibMenu;
class HydroAdditivesMenu;
class HydroCalibrationsMenu;

#include "HydruinoUI.h"
#include "RemoteMenuItem.h"
#include "EditableLargeNumberMenuItem.h"

// Menu Screen Base
class HydroMenu
{
public:
    HydroMenu();
    virtual ~HydroMenu();

    virtual void loadMenu(MenuItem *addFrom = nullptr) = 0; // should call menuMgr.addMenuAfter()
    virtual MenuItem *getRootItem() = 0;

    inline bool isLoaded() const { return _loaded; }

protected:
    bool _loaded;
};

#define CALLBACK_FUNCTION
#define NO_ADDRESS                      0xffff              // No EEPROM address

// Initializes an AnyMenuInfo structure
#define InitAnyMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback)\
    strncpy(varName.name, SFP(strNum).c_str(), NAME_SIZE_T);\
    varName.id = itemId;\
    varName.eepromAddr = eepromPosition;\
    varName.maxValue = valMaximum;\
    varName.callback = fnCallback

// Initializes a BooleanMenuInfo structure
#define InitBooleanMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback,boolNaming)\
    strncpy(varName.name, SFP(strNum).c_str(), NAME_SIZE_T);\
    varName.id = itemId;\
    varName.eepromAddr = eepromPosition;\
    varName.maxValue = valMaximum;\
    varName.callback = fnCallback;\
    varName.naming = boolNaming

// Initializes a SubMenuInfo structure
#define InitSubMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback)\
    InitAnyMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback)

// Initializes an EnumMenuInfo structure
#define InitEnumMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback,enumItems)\
    strncpy(varName.name, SFP(strNum).c_str(), NAME_SIZE_T);\
    varName.id = itemId;\
    varName.eepromAddr = eepromPosition;\
    varName.maxValue = valMaximum;\
    varName.callback = fnCallback;\
    varName.menuItems = enumItems

// Initializes an AnalogMenuInfo structure, with units
#define InitAnalogMenuInfoUnits(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback,valOffset,valDivisor,unitsStrNum)\
    strncpy(varName.name, SFP(strNum).c_str(), NAME_SIZE_T);\
    varName.id = itemId;\
    varName.eepromAddr = eepromPosition;\
    varName.maxValue = valMaximum;\
    varName.callback = fnCallback;\
    varName.offset = valOffset;\
    varName.divisor = valDivisor;\
    strncpy(varName.unitName, SFP(unitsStrNum).c_str(), UNIT_SIZE_T)

// Initializes an AnalogMenuInfo structure, blank units
#define InitAnalogMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback,valOffset,valDivisor)\
    strncpy(varName.name, SFP(strNum).c_str(), NAME_SIZE_T);\
    varName.id = itemId;\
    varName.eepromAddr = eepromPosition;\
    varName.maxValue = valMaximum;\
    varName.callback = fnCallback;\
    varName.offset = valOffset;\
    varName.divisor = valDivisor;\
    strncpy(varName.unitName, HUIStr_Blank, UNIT_SIZE_T)

// Altered rendering callback
#define H_RENDERING_CALLBACK_NAME_INVOKE(fnName, parent, strNum, eepromPosition, invoke) \
int fnName(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int buffSize) { \
	switch(mode) { \
	case RENDERFN_NAME: \
        strncpy(buffer, SFP(strNum).c_str(), buffSize); \
		return true; \
    case RENDERFN_INVOKE: \
		invokeIfSafe(invoke, item); \
		return true; \
	case RENDERFN_EEPROM_POS: \
		return eepromPosition; \
	default: \
		return parent(item, row, mode, buffer, buffSize); \
	} \
}

#include "screens/HydroMenuHome.h"
#include "screens/HydroMenuAlerts.h"
#include "screens/HydroMenuActuators.h"
#include "screens/HydroMenuSensors.h"
#include "screens/HydroMenuCrops.h"
#include "screens/HydroMenuReservoirs.h"
#include "screens/HydroMenuPowerRails.h"
#include "screens/HydroMenuScheduling.h"
#include "screens/HydroMenuCropsLib.h"
#include "screens/HydroMenuAdditives.h"
#include "screens/HydroMenuCalibrations.h"

#endif // /ifndef HydroMenus_H
#endif
