/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Minimal UI
*/

class HydruinoMinUI;

#include "Hydruino.h"

class HydruinoMinUI : HydruinoUIInterface {
public:
    virtual void begin() override;

    virtual void setNeedsLayout() override;

protected:
};
