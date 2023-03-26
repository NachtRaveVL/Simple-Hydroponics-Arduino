/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Menu Screens
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroMenus_H
#define HydroMenus_H

class HydroMenu;

#include "HydruinoUI.h"
#include "RemoteMenuItem.h"
#include "EditableLargeNumberMenuItem.h"

// Menu Screen Base
class HydroMenu
{
public:
    inline HydroMenu() : _loaded(false) { ; }
    virtual ~HydroMenu() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) = 0; // should call menuMgr.addMenuAfter()
    virtual MenuItem *getRootItem() = 0;

    inline bool isLoaded() const { return _loaded; }

protected:
    bool _loaded;
};

// Initializes an AnyMenuInfo structure
#define InitAnyMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback)\
    safeProgCpy(varName.name, CFP(strNum), NAME_SIZE_T);\
    varName.id = itemId;\
    varName.eepromAddr = eepromPosition;\
    varName.maxValue = valMaximum;\
    varName.callback = fnCallback

// Initializes a BooleanMenuInfo structure
#define InitBooleanMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback,boolNaming)\
    InitAnyMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback);\
    varName.naming = boolNaming

// Initializes a SubMenuInfo structure
#define InitSubMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback)\
    InitAnyMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback)

// Initializes an EnumMenuInfo structure
#define InitEnumMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback,enumItems)\
    InitAnyMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback);\
    varName.menuItems = enumItems

// Initializes an AnalogMenuInfo structure, with units from CFP()
#define InitAnalogMenuInfoUnits(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback,valOffset,valDivisor,unitsStrNum)\
    InitAnyMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback);\
    varName.offset = valOffset;\
    varName.divisor = valDivisor;\
    safeProgCpy(varName.unitName, CFP(unitsStrNum), UNIT_SIZE_T)

// Initializes an AnalogMenuInfo structure, with blank units
#define InitAnalogMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback,valOffset,valDivisor)\
    InitAnyMenuInfo(varName,strNum,itemId,eepromPosition,valMaximum,fnCallback);\
    varName.offset = valOffset;\
    varName.divisor = valDivisor;\
    varName.unitName[0] = '\000'

// Altered rendering callback that uses CFP()
#define H_RENDERING_CALLBACK_NAME_INVOKE(fnName, parent, strNum, eepromPosition, invoke) \
int fnName(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int buffSize) { \
	switch(mode) { \
        case RENDERFN_NAME: \
            safeProgCpy(buffer, CFP(strNum), buffSize); \
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

#ifdef HYDRO_DISABLE_BUILTIN_DATA
#define InfoPtrForItem(itemName,castType) (&init.minfo##itemName)
#define InfoLocation INFO_LOCATION_RAM
#else
#define InfoPtrForItem(itemName,castType) ((const castType *)CFP(HUIStr_Item_##itemName))
#define InfoLocation INFO_LOCATION_PGM
#endif

#include "screens/HydroMenuHome.h"
#include "screens/HydroMenuAlerts.h"
#include "screens/HydroMenuActuators.h"
#include "screens/HydroMenuSensors.h"
#include "screens/HydroMenuCrops.h"
#include "screens/HydroMenuReservoirs.h"
#include "screens/HydroMenuPowerRails.h"
#include "screens/HydroMenuScheduling.h"
#include "screens/HydroMenuSettings.h"
#include "screens/HydroMenuCropsLib.h"
#include "screens/HydroMenuAdditives.h"
#include "screens/HydroMenuCalibrations.h"
#include "screens/HydroMenuInformation.h"

#endif // /ifndef HydroMenus_H
#endif
