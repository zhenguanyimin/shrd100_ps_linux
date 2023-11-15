#include "TracerMainHalMgr.h"
#include "TracerAdcHal.h"
#include "AlinkCommonHal.h"
#include "TracerDeviceHal.h"
#include "TracerDataPath.h"

TracerMainHalMgr::TracerMainHalMgr()
{
    _commHal = nullptr;
    _adcHal = nullptr;
    _dataHal = nullptr;
}

TracerMainHalMgr::~TracerMainHalMgr()
{

}

void TracerMainHalMgr::_InstallHals()
{
    _commHal = new TracerCommonHal(this);
    AddHal(_commHal);
    _adcHal = new TracerAdcHal(this);
    AddHal(_adcHal);
    AddHal(new AlinkCommonHal(this));
    AddHal(new TracerDeviceHal(this));
    _dataHal = new TracerDataPath(this);
    AddHal(_dataHal);
}