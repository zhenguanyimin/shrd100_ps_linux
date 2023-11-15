#include "DroneIdCmdproc.h"
#include "DroneIdModule.h"
#include "AlinkCmdproc.h"
#include "HalMgr.h"
#include "SysBase.h"
#include "CLI_Template.h"
#include "hal.h"

extern "C"
{
#include "droneID_utils.h"
}

DEFINE_CLI_HANDLER1_SYNC(AlgDebugPrintMode, EAP_CMD_DRONEID_SET_ALG_PRINT, \
    "\r\n AlgDebugPrintMode <enableSwitch>:\r\n set algorithm debug print mode\r\n", \
    "OK, set algorithm debug print mode: %d", \
    "ERROR, set algorithm debug print mode: %d, exceed!")

DEFINE_CLI_HANDLER1_SYNC(DroneIDRemoveThreSet, EAP_CMD_DRONEID_SET_REMOVE_THRE, \
    "\r\n DroneIDRemoveThreSet <value>:\r\n set droneID remove threshold value\r\n", \
    "OK, set droneID remove threshold value: %d", \
    "ERROR, set droneID remove threshold value: %d")

DEFINE_CLI_HANDLER1_SYNC(DroneIDRemoveThreLostDurSet, EAP_CMD_DRONEID_SET_REMOVE_THRE_LOST_DUR, \
    "\r\n DroneIDRemoveThreLostDurSet <value>:\r\n set droneID remove threshold value, lost duration \r\n", \
    "OK, set droneID remove threshold value, lost duration(ms): %d", \
    "ERROR, set droneID remove threshold value: %d")

DEFINE_CLI_HANDLER1_SYNC(DroneIDOutputSnModeSet, EAP_CMD_DRONEID_SET_OUTPUT_SN_MODE, \
    "\r\n DroneIDOutputSnModeSet <value>:\r\n set droneID output sn mode\r\n", \
    "OK, set droneID output sn mode: %d", \
    "ERROR, set droneID output sn mode: %d, exceed!")

DEFINE_CLI_HANDLER1_SYNC(DroneIDTurboDecIterNumSet, EAP_CMD_DRONEID_SET_TURBO_DEC_ITER_NUM, \
    "\r\n DroneIDTurboDecIterNumSet <value>:\r\n set droneID turbo decode iteration num \r\n", \
    "OK, set droneID turbo decode iteration num : %d", \
    "ERROR, set turbo decode iteration num: %d, exceed!")

void DroneIdCmdproc::_RegisterCmd()
{
    Register(EAP_CMD_DRONEID, (Cmdproc_Func)&DroneIdCmdproc::_OnDroneIdCmd);
    Register(EAP_CMD_DRONEID_SET_ALG_PRINT, (Cmdproc_Func)&DroneIdCmdproc::_OnSetAlgorithmPrint, &AlgDebugPrintMode_cmd);
    Register(EAP_CMD_DRONEID_SET_REMOVE_THRE, (Cmdproc_Func)&DroneIdCmdproc::_OnSetRemoveThreshold, &DroneIDRemoveThreSet_cmd);
    Register(EAP_CMD_DRONEID_SET_REMOVE_THRE_LOST_DUR, (Cmdproc_Func)&DroneIdCmdproc::_OnSetRemoveThresholdLostDuration, &DroneIDRemoveThreLostDurSet_cmd);
    Register(EAP_CMD_DRONEID_SET_OUTPUT_SN_MODE, (Cmdproc_Func)&DroneIdCmdproc::_OnSetOutputSnMode, &DroneIDOutputSnModeSet_cmd);
    Register(EAP_CMD_DRONEID_SET_TURBO_DEC_ITER_NUM, (Cmdproc_Func)&DroneIdCmdproc::_OnSetTurboDecIterNum, &DroneIDTurboDecIterNumSet_cmd);
    Register(EAP_CMD_SHOW_DRONEID_MODULE, (Cmdproc_Func)&DroneIdCmdproc::_OnShowDroneIdModuleCmd);
}

void DroneIdCmdproc::_OnSetTurboDecIterNum(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    int32_t* iterNum = (int32_t*)(inMsg.data);
    if ((*iterNum > 0) && (*iterNum < 10))
    {
        SetDroneIDTurboDecIterNum(*iterNum);
        *(outMsg.data) = EAP_SUCCESS;
    }
    else
    {
        *(outMsg.data) = EAP_FAIL;
    }
    outMsg.dataLen = 1;
}

void DroneIdCmdproc::_OnSetOutputSnMode(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    int32_t* snMode = (int32_t*)(inMsg.data);
    if ((*snMode >= 0) && (*snMode < DRONEID_OUTPUT_SN_MODE_NUM))
    {
        SetDroneIDOutputSnMode(*snMode);
        *(outMsg.data) = EAP_SUCCESS;
    }
    else
    {
        *(outMsg.data) = EAP_FAIL;
    }
    outMsg.dataLen = 1;
}

void DroneIdCmdproc::_OnSetRemoveThresholdLostDuration(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    int32_t* threValue = (int32_t*)(inMsg.data);
    if (*threValue >= 0)
    {
        SetDroneIDRemoveThreLostDur(*threValue);
        *(outMsg.data) = EAP_SUCCESS;
    }
    else
    {
        *(outMsg.data) = EAP_FAIL;
    }
    outMsg.dataLen = 1;
}

void DroneIdCmdproc::_OnSetRemoveThreshold(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    int32_t* threValue = (int32_t*)(inMsg.data);
    if (*threValue >= 0)
    {
        SetDroneIDRemoveThre(*threValue);
        *(outMsg.data) = EAP_SUCCESS;
    }
    else
    {
        *(outMsg.data) = EAP_FAIL;
    }
    outMsg.dataLen = 1;
}

void DroneIdCmdproc::_OnSetAlgorithmPrint(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    int32_t* printMode = (int32_t*)(inMsg.data);
    if ((*printMode >= 0) && (*printMode < ALG_PRINT_MODE_NUM))
    {
        SetAlgDebugPrintMode(*printMode);
        *(outMsg.data) = EAP_SUCCESS;
    }
    else
    {
        *(outMsg.data) = EAP_FAIL;
    }
    outMsg.dataLen = 1;
}

void DroneIdCmdproc::_OnDroneIdCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
	burst_data_event    *event = (burst_data_event    *)(inMsg.data);
    switch (event->flag)
    {
    case eDATA_PATH_DDR_BURST:
    case eDATA_PATH_TIMER:
        {
            DroneIdModule* droneIdModule = dynamic_cast<DroneIdModule*>(_module);
            if (nullptr != droneIdModule) droneIdModule->RunDroneAlgNew(event->flag,event->channel);
        }
        break;
    default:
        {
            HalMgr* halMgr = EapGetSys().GetHalMgr();
            Hal_Data inData = {1, inMsg.data};
            if (nullptr != halMgr) halMgr->OnSet(EAP_DRVCODE_DRONE_PL_TEST, inData);
        }
        break;
    }
}

void DroneIdCmdproc::_OnTimer(uint8_t timerType)
{
    if (EAP_CMDTYPE_TIMER_1 == timerType)
    {
        _OnFreqTimer();
    }
    else if (EAP_CMDTYPE_TIMER_2 == timerType)
    {
        _ReportDroneIdsInfo();
    }
}

void DroneIdCmdproc::_OnFreqTimer()
{
    if (WORKMODE_DRONE == EapGetSys().GetWorkMode())
    {
        SendAsynMsg(EAP_CMD_FREQUENCY, _sendBuf, 0);
    }

    DroneIdModule* droneIdModule = dynamic_cast<DroneIdModule*>(_module);
    if (nullptr != droneIdModule) droneIdModule->OnTimer();    
}

void DroneIdCmdproc::_ReportDroneIdsInfo()
{
    DroneIdModule* droneIdModule = dynamic_cast<DroneIdModule*>(_module);
    if (nullptr != droneIdModule)
    {
        alink_msg_t msg = {0};
        msg.channelType = EAP_CHANNEL_TYPE_TCP | EAP_CHANNEL_TYPE_TYPEC;
        msg.channelId   = UINT16_ALL;
        int16_t dataLen = droneIdModule->FillDroneIdInfo(msg.buffer, sizeof(msg.buffer) - 4);
        if (-1 == dataLen) return;
        AlinkCmdproc::SendAlinkCmd(msg, dataLen, EAP_ALINK_RPT_DRONEID);
    }
}

void DroneIdCmdproc::_OnShowDroneIdModuleCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    _DoShowModuleInfo();
}