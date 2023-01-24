/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Full UI
*/

class HydruinoFullUI;

#include "Hydruino.h"

class HydruinoFullUI : HydruinoUIInterface {
public:
    virtual void begin() override;

    virtual void setNeedsLayout() override;

protected:
};
