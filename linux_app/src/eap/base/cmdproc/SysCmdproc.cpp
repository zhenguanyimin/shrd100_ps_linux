#include "SysCmdproc.h"
#include "HalMgr.h"
#include "CLI_Template.h"
#include "HalBase.h"

void cli_set_board_sn_func(int32_t len, const char *param, char *pcWriteBuffer, size_t xWriteBufferLen)
{
	if (len <= EAP_SN_LENGTH)
	{
        SendAsynData(EAP_CMD_SET_SN_CODE, (uint8_t*)param, len);
		lLOG_DEBUG("sn code : %s\r\n", param );
	}
	else
	{
		lLOG_DEBUG("para len invalide!\r\n");
	}
}

void cli_get_board_sn_func(char *pcWriteBuffer, size_t xWriteBufferLen)
{
    uint8_t boardSN[EAP_SN_LENGTH] = {0};
    uint32_t respDataLen = EAP_SN_LENGTH;
    SendSynData(EAP_THREAD_ID_CLI, EAP_CMD_GET_SN_CODE, nullptr, 0, boardSN, &respDataLen);
    if (0 == respDataLen || respDataLen > EAP_SN_LENGTH)
    {
		lLOG_DEBUG("no sn code in flash!\r\n");
    }
	else
	{
		lLOG_DEBUG("sn code : %s\r\n", (char *)boardSN );
	}
}

DEFINE_CLI_HANDLER1_FUNC(SetBoardSN, "\r\n SetBoardSN\r\n  \r\n", cli_set_board_sn_func)
DEFINE_CLI_HANDLER_FUNC(GetBoardSN, "\r\n GetBoardSN\r\n  \r\n", cli_get_board_sn_func)
DEFINE_CLI_HANDLER1(SetMsgStoreOn, EAP_CMD_SET_MSG_STORE_ON, "\r\n SetMsgStoreOn <enable>:\r\n (0:disable, 1:enable)\r\n")
DEFINE_CLI_HANDLER1(SetDrvStoreOn, EAP_CMD_SET_DRV_STORE_ON, "\r\n SetDrvStoreOn <enable>:\r\n (0:disable, 1:enable)\r\n")

void SysCmdproc::_RegisterCmd()
{
    Register(EAP_CMD_SET_WORK_MODE, (Cmdproc_Func)&SysCmdproc::_OnSetWorkModeCmd);
    Register(EAP_CMD_SET_SN_CODE, (Cmdproc_Func)&SysCmdproc::_OnSetSnCode, &SetBoardSN_cmd);
    Register(EAP_CMD_GET_SN_CODE, (Cmdproc_Func)&SysCmdproc::_OnGetSnCode, &GetBoardSN_cmd);
    Register(EAP_CMD_SET_MSG_STORE_ON, (Cmdproc_Func)&SysCmdproc::_OnSetMsgStoreOn, &SetMsgStoreOn_cmd);
    Register(EAP_CMD_SET_DRV_STORE_ON, (Cmdproc_Func)&SysCmdproc::_OnSetDrvStoreOn, &SetDrvStoreOn_cmd);
}

void SysCmdproc::_OnSetMsgStoreOn(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    if (0 == *(inMsg.data))
    {
        CmdprocBase::DisableStoreCmd();
    }
    else if (1 == *(inMsg.data))
    {
        CmdprocBase::EnableStoreCmd();
    }
}

void SysCmdproc::_OnSetDrvStoreOn(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    if (0 == *(inMsg.data))
    {
        HalBase::DisableStoreCmd();
    }
    else if (1 == *(inMsg.data))
    {
        HalBase::EnableStoreCmd();
    }
}

void SysCmdproc::_OnSetWorkModeCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    EapGetSys().SetWorkMode(*(inMsg.data));
    *(outMsg.data) = *(inMsg.data);
    SendAsynMsg(EAP_CMD_CHANGE_FREQUENCY_PARAM, _sendBuf, 1);
}

void SysCmdproc::_OnSetSnCode(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    Hal_Data inData = {inMsg.dataLen, inMsg.data};
    HalMgr* halMgr = EapGetSys().GetHalMgr();
    if (nullptr != halMgr && EAP_SUCCESS == halMgr->OnSet(EAP_DRVCODE_SET_SN, inData))
    {
        EapGetSys().SetSnName((char*)inMsg.data, inMsg.dataLen);
    }
}

void SysCmdproc::_OnGetSnCode(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    memcpy(outMsg.data, EapGetSys().GetSnName(), EAP_SN_LENGTH);
    outMsg.dataLen = EapGetSys().GetSnNameLen();
}

void SysCmdproc::_OnTimer(uint8_t timerType)
{
    if (EAP_CMDTYPE_TIMER_1 == timerType)
    {
        _QueryME();
    }
    else if (EAP_CMDTYPE_TIMER_2 == timerType)
    {
        //EapGetSys().DupResponse();
    }
}

void SysCmdproc::_QueryME()
{
    HalMgr* halMgr = EapGetSys().GetHalMgr();
    if (nullptr != halMgr)
    {
        EapGetSys().RefreshBatteryCapacity();
    }
}
