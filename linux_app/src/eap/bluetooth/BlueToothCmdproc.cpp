#include "AlinkCmdproc.h"
#include "HalMgr.h"
#include "SysBase.h"
#include "BlueToothCmdproc.h"

#if 0
#define DEBUG_BLUETOOTH_PRINTF(arg...) printf(arg)
#else
#define DEBUG_BLUETOOTH_PRINTF(arg...) 
#endif


BlueToothCmdproc::BlueToothCmdproc()
{
}

BlueToothCmdproc::~BlueToothCmdproc()
{

}

void BlueToothCmdproc::_RegisterCmd()
{
    Register(EAP_CMD_SET_WIFI_NAME_AND_SECRET, (Cmdproc_Func)&BlueToothCmdproc::_BlueToothSetNameAndSecret);
}


void BlueToothCmdproc::_BlueToothSetNameAndSecret(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
	alink_msg_t *pmsg = (alink_msg_t *)inMsg.data;
	HalMgr* halMgr = EapGetSys().GetHalMgr();
	
	if (nullptr != halMgr)
	{
			Hal_Data inData = {0, 0};
	
			inData.length = (pmsg->msg_head.len_hi << 8) + pmsg->msg_head.len_lo;
			inData.buf = pmsg->buffer;
			DEBUG_BLUETOOTH_PRINTF("[%s, %d]pmsg->msg_head.msgid = 0x%x\n", __func__, __LINE__, pmsg->msg_head.msgid);
	
			DEBUG_BLUETOOTH_PRINTF("[%s, %d]inData.length = %d\n", __func__, __LINE__, inData.length);
	
			halMgr->OnSet(EAP_DRVCODE_BLUTTOOTH_SET_NETWORK, inData);
	}
	
}



