#include "HeartBeatModule.h"

#include "HeartBeatCmdproc.h"
#include "HeartBeatThread.h"
#include "SysBase.h"
#include "HalMgr.h"
#include "WarningBase.h"
#include "../../srv/wit/WitService.h"

HeartBeatModule::HeartBeatModule(uint8_t coreId) : ModuleReal(coreId)
{
    _workStatus = SYS_IDLE_STATUS;
    _faultStatus = SYS_IDLE_STATUS;
    _covertMode = 0;
}

uint8_t HeartBeatModule::GetModuleId()
{
    return EAP_MODULE_ID_HEARTBEAT;
}

CmdprocBase* HeartBeatModule::_CreateCmdproc()
{
    return new HeartBeatCmdproc();
}

ThreadBase* HeartBeatModule::_CreateThread()
{
    return ADD_THREAD(HeartBeatThread, _coreId);
}

int16_t HeartBeatModule::FillInfo(uint8_t* buf, uint16_t bufLen)
{
    int16_t dataLen = sizeof(Rpt_HeartBeat);
    if (dataLen > bufLen) return -1;

    uint8_t workMode = EapGetSys().GetWorkMode();
    if (0 == workMode) return -1;

    EapGetSys().RefreshBatteryCapacity();

    Hal_Data inData = {0, nullptr};
    uint8_t gearPosition = 0;
    Hal_Data outData = {sizeof(uint8_t), &gearPosition};
    if (EAP_SUCCESS != EapGetSys().GetHalMgr()->OnGet(EAP_DRVCODE_GET_ALARM_MODE, inData, outData))
    {
        return -1;
    }

    for (auto& warningobj : _warnings)
    {
        warningobj->OrientationStatusProc(workMode);  //进入定向，开启定向规律报警显示；退出定向，关闭显示
        warningobj->GearPositionProc(gearPosition);
    }

    Rpt_HeartBeat* info = (Rpt_HeartBeat *)buf;
    memcpy(info->snName, EapGetSys().GetSnName(), sizeof(info->snName));
    uint32_t timeStamp = eap_get_cur_time_ms();
    info->timeStamp     = CONVERT_32_REV(timeStamp);
    info->electricity   = EapGetSys().GetBatteryCapacity();
    info->batteryStatus = EapGetSys().GetBatAndAdapterState();
    info->workMode      = workMode;
    info->workStatus = ((_workStatus & 0xF) | ((_faultStatus << 4) & 0xF0));
    info->alarmLevel = _GetAlarmLevel(gearPosition);

    // OnSilentAction();

    return dataLen;
}

// do actions when mute switch is on
void HeartBeatModule::OnSilentAction(void)
{
/*
    const WIT_STATUS_t vitStatus= wit_status();
    Hal_Data inData = {0, nullptr};
    std::uint8_t isMute = 0;
    Hal_Data outData = {sizeof(std::uint8_t), &isMute};

    if (EAP_SUCCESS != EapGetSys().GetHalMgr()->OnGet(EAP_DRVCODE_GET_SILENT_MODE, inData, outData))
    {
        EAP_LOG_ERROR("get hardware silent mode failed!\n");
        return;
    }

    if ((isMute) && (vitStatus != WIT_STATUS_DISABLE)) // mute is on, close wit
    {
        if (0 != wit_disable())
        {
            EAP_LOG_ERROR("wit disable failed!\n");
        }
    }

    if ((!isMute) && (vitStatus == WIT_STATUS_DISABLE)) // mute is off, open wit
    {
        if (0 != wit_enable())
        {
            EAP_LOG_ERROR("wit enable failed!\n");
        }
    }
*/
    return;
}

uint8_t HeartBeatModule::_GetAlarmLevel(uint8_t gearPosition)
{
    uint8_t alarmLevelValue = 0;
    for (auto& warningobj : _warnings)
    {
        warningobj->FillAlarmLevel(alarmLevelValue);
    }
    // 2023-11-11 参考《tracer与C2和上位机通信协议V1.0.14.docx》
    // bit0~bit3：告警等级；bit6：隐蔽模式
    alarmLevelValue |= gearPosition;
    alarmLevelValue |= (_covertMode << 6);
    return alarmLevelValue;
}

void HeartBeatModule::TargetAppear(uint8_t targetType, uint8_t appear)
{
    uint8_t oldTargetState = _GetTargetState();
    _targets[targetType] = appear;
    uint8_t newTargetState = _GetTargetState();
    if (oldTargetState != newTargetState)
    {
        _TargetWarningProc(newTargetState);
    }
}

uint8_t HeartBeatModule::_GetTargetState()
{
    for (auto& it : _targets)
    {
        if (it.second > 0)
        {
            return 1;
        }
    }
    return 0;
}

void HeartBeatModule::_TargetWarningProc(uint8_t targetAppear)
{
    //报警停止标志  1.模式切换  2.目标消失
    for (auto& warningobj : _warnings)
    {
        warningobj->TargetProc(targetAppear);
    }
}

void HeartBeatModule::InstallWarningObj(WarningBase* warning)
{
    _warnings.insert(warning);
}

void HeartBeatModule::RequestShowInfo()
{
    SendAsynCmdCode(EAP_CMD_SHOW_HEARTBEAT_MODULE);
}

void HeartBeatModule::ShowInfo(uint8_t level)
{
    ModuleReal::ShowInfo(level);
    cout << EAP_BLANK_PREFIX(level) << "_workStatus: " << (uint16_t)_workStatus;
    cout << EAP_BLANK_PREFIX(level) << "_faultStatus: " << (uint16_t)_faultStatus;
    cout << EAP_BLANK_PREFIX(level) << "TargetNum: " << _targets.size();
    cout << EAP_BLANK_PREFIX(level) << "_targets: [";
    for (auto target : _targets)
    {
        cout << (uint16_t)target.first << ": " << (uint16_t)target.second << ", ";
    }
    if (_targets.size() > 0) cout << EAP_OUT_BACKSPACE;
    cout << "]";
    cout << EAP_BLANK_PREFIX(level) << "_warnings: ";
    for (auto warning : _warnings)
    {
        cout << EAP_BLANK_PREFIX(level + 1) << "Warning: ";
        warning->ShowInfo(level + 2);
    }
}

void HeartBeatModule::Init()
{
    ModuleReal::Init();
    // WarningBase::InitWarningLevelArray();
}

extern "C"
{
	void SetWarningLevel(uint8_t gearPosition,uint8_t targetState,uint8_t workMode,uint8_t level)
	{
        (void)gearPosition;
        (void)targetState;
        (void)workMode;
        (void)level;
        // To be moved to LedWarning::Instance()->...
        // WarningBase::SetWarningLevel(gearPosition, targetState, workMode, level);
	}
}
