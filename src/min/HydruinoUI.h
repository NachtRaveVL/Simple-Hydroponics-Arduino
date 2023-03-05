/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Minimal/RO UI
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroUI_H
#define HydroUI_H

class HydruinoMinUI;
typedef HydruinoMinUI HydruinoUI;

#include "..\shared\HydruinoUI.h"

class HydruinoMinUI : public HydruinoBaseUI {
public:
    virtual bool isFullUI() override;

protected:
};

#endif // /ifndef HydroUI_H
#endif
