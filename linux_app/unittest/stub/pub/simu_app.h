#pragma once

#include "TracerMainSys.h"
#include "TracerMainHalMgr.h"

class TracerMainSysStub : public TracerMainSys
{
public:
    TracerMainSysStub(uint8_t deviceId, int16_t processId);
};

class TracerMainHalMgrStub : public TracerMainHalMgr
{
protected:
    void _InstallHals() override;
};
