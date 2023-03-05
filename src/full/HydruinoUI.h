/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Full/RW UI
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroUI_H
#define HydroUI_H

class HydruinoFullUI;
typedef HydruinoFullUI HydruinoUI;

#include "..\shared\HydruinoUI.h"

class HydruinoFullUI : public HydruinoBaseUI {
public:
    virtual bool isFullUI() override;

protected:
};

#endif // /ifndef HydroUI_H
#endif
