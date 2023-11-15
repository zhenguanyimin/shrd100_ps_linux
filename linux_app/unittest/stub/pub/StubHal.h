#pragma once

#include "HalBase.h"

class StubHal : public HalBase
{
public:
    StubHal(HalMgr* halMgr) : HalBase(halMgr){}
    void Init() override;
protected:
    int _Dispatch(uint16_t drvCode, const Hal_Data& inData, Hal_Data& outData) override;
    int _OnRptInfoByAlink(const Hal_Data& inData, Hal_Data& outData);
    int _OnGetDroneFreq(const Hal_Data& inData, Hal_Data& outData);
    int _OnGetDroneAdc(const Hal_Data& inData, Hal_Data& outData);
private:
};
