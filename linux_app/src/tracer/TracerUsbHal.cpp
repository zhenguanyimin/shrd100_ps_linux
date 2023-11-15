#include "TracerUsbHal.h"
#include "eap_sys_drvcodes.h"

TracerUsbHal::TracerUsbHal(HalMgr* halMgr) : HalBase(halMgr)
{
    _Register(EAP_DRVCODE_CLI_OUT, (Hal_Func)&TracerUsbHal::_OnCliOut);
}

int TracerUsbHal::_OnCliOut(const Hal_Data& inData, Hal_Data& outData)
{
    return EAP_SUCCESS;
}
