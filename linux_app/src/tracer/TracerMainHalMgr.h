#pragma once

#include "HalMgr.h"

class TracerCommonHal;
class TracerAdcHal;
class TracerDataPath;
class TracerMainHalMgr : public HalMgr
{
public:
    TracerMainHalMgr();
    ~TracerMainHalMgr();
    TracerCommonHal * GetCommHal() { return _commHal; }
    TracerAdcHal * GetAdcHal() { return _adcHal; }
    TracerDataPath * GetDataHal() { return _dataHal; }
protected:
    void _InstallHals() override;
private:
    TracerCommonHal *_commHal;
    TracerAdcHal *_adcHal;
    TracerDataPath *_dataHal;
};