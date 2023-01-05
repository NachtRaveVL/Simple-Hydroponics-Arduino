/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydroponics Full UI
*/

class HydroponicsFullUI;

#include "Hydroponics.h"

class HydroponicsFullUI : HydroponicsUIInterface {
public:
    virtual void begin() override;

    virtual void setNeedsLayout() override;

protected:
};
