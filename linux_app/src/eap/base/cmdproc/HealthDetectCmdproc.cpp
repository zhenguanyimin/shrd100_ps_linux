#include "HealthDetectCmdproc.h"
#include "SysBase.h"
#include "ThreadMgr.h"


void HealthDetectCmdproc::_RegisterCmd()
{
    Register(EAP_CMD_RUN_HEALTH_DETECT, (Cmdproc_Func)&HealthDetectCmdproc::_OnRunHealthDetect);
    Register(EAP_CMD_RUN_PAUSE, (Cmdproc_Func)&HealthDetectCmdproc::_OnThreadRunPause);
    Register(EAP_CMD_RUN_IDLING, (Cmdproc_Func)&HealthDetectCmdproc::_OnThreadRunIdling);
    Register(EAP_CMD_RUN_RESUME, (Cmdproc_Func)&HealthDetectCmdproc::_OnThreadRunResume);
}

void HealthDetectCmdproc::_OnRunHealthDetect(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    EapGetSys().GetThreadMgr().BeginDetectRunHealth();
    EapGetSys().GetThreadMgr().StartDelayTimer(_threadId, EAP_CMDTYPE_TIMER_1);
    cout << "\nBegin Detect Thread Run Health.";
}

void HealthDetectCmdproc::_OnThreadRunPause(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    uint16_t *threadId = (uint16_t *)(inMsg.data);
    EapGetSys().GetThreadMgr().Pause(*threadId);
}

void HealthDetectCmdproc::_OnThreadRunIdling(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    uint16_t *threadId = (uint16_t *)(inMsg.data);
    EapGetSys().GetThreadMgr().Idling(*threadId);
}

void HealthDetectCmdproc::_OnThreadRunResume(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    uint16_t *threadId = (uint16_t *)(inMsg.data);
    EapGetSys().GetThreadMgr().Resume(*threadId);
}

void HealthDetectCmdproc::_OnTimer(uint8_t timerType)
{
    if (EAP_CMDTYPE_TIMER_1 == timerType)
    {
        EapGetSys().GetThreadMgr().EndDetectRunHealth();
    }
}
