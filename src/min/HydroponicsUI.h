/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydroponics Minimal UI
*/

class HydroponicsMinUI;

#include "Hydroponics.h"

class HydroponicsMinUI : HydroponicsUIInterface {
public:
    virtual void begin() override;

    virtual void setNeedsLayout() override;

protected:
};
