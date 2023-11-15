#include "AlinkCommonHal.h"
#include "HalMgr.h"
#include "SysBase.h"
int AlinkCommonHal::_SendAlinkRspMsg(const Hal_Data& inData, Hal_Data& outData)
{
	HalMgr *halMgr = EapGetSys().GetHalMgr();
	if (nullptr != halMgr)
	{
		halMgr->OnSet(EAP_DRVCODE_SEND_ALINK, inData);
	}else{
		return EAP_FAIL;
	}

	return EAP_SUCCESS;
}


void AlinkCommonHal::Init()
{	
}


AlinkCommonHal::AlinkCommonHal(HalMgr* halMgr) : HalBase(halMgr)
{
    _Register(EAP_DRVCODE_ALINK_RESPONSE, (Hal_Func)&AlinkCommonHal::_SendAlinkRspMsg);
}
