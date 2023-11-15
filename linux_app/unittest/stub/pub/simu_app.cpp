#include "simu_app.h"
#include "StubHal.h"

TracerMainSys tracerSys(DEV_TRACER, 0);

TracerMainSysStub::TracerMainSysStub(uint8_t deviceId, int16_t processId) : TracerMainSys(deviceId, processId)
{
    _InstallHalMgr(new TracerMainHalMgrStub());
}

void TracerMainHalMgrStub::_InstallHals()
{
    AddHal(new StubHal(this));
}

TracerMainSysStub tracerSysStub(DEV_TRACER, 0);
