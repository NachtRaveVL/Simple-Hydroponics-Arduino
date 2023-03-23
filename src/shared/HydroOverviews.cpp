/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Overview Screens
*/

#include "HydruinoUI.h"
#ifdef HYDRO_USE_GUI

HydroOverview::HydroOverview(HydroDisplayDriver *display)
    : _display(display), _needsFullRedraw(true)
{ ; }

HydroOverview::~HydroOverview()
{ ; }

#endif
