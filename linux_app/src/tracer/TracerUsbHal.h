#pragma once

#include "HalBase.h"

class TracerUsbHal : public HalBase
{
public:
    TracerUsbHal(HalMgr* halMgr);
protected:
    int _OnCliOut(const Hal_Data& inData, Hal_Data& outData);
private:
};
