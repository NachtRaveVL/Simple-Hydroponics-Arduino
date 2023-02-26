/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Full UI
*/

class HydruinoFullUI;

#include "Hydruino.h"

class HydruinoFullUI : HydruinoUIInterface {
public:
    virtual bool begin() override;

    virtual void setNeedsLayout() override;

protected:
};
