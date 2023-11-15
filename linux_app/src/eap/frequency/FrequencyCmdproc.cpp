#include "FrequencyCmdproc.h"
#include "FrequencyModule.h"
#include "SweepFreqDetector.h"
#include "AutoFreqDetector.h"
#include "DefinedFreqDetector.h"
#include "HalMgr.h"
#include "SysBase.h"
#include <stdio.h>

void FrequencyCmdproc::_RegisterCmd()
{
    Register(EAP_CMD_FREQUENCY, (Cmdproc_Func)&FrequencyCmdproc::_OnFrequencyCmd);
    Register(EAP_CMD_SET_FREQUENCY, (Cmdproc_Func)&FrequencyCmdproc::_OnSetFrequencyCmd);
    Register(EAP_CMD_SET_FREQUENCY_MODE, (Cmdproc_Func)&FrequencyCmdproc::_OnSetFrequencyModeCmd);
    Register(EAP_CMD_CHANGE_FREQUENCY_PARAM, (Cmdproc_Func)&FrequencyCmdproc::_OnChgFrequencyParamCmd);
    Register(EAP_CMD_SHOW_FREQUENCY_MODULE, (Cmdproc_Func)&FrequencyCmdproc::_OnShowFrequencyModuleCmd);
}

void FrequencyCmdproc::_OnFrequencyCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    FrequencyModule* frequencyModule = dynamic_cast<FrequencyModule*>(_module);
    if (nullptr == frequencyModule) return;

    // 打击状态下进入保护模式，还有故障状态也不做切频操作
    uint8_t status = EapGetSys().GetSysStatus();
    if (status == SYS_HIT_TARGET_STATUS || status == SYS_MALFUNCTION_STATUS)
    {
        EAP_LOG_INFO("当前系统状态为%d, 不进行切频操作!\n", status);
        return;
    }

    frequencyModule->ChangeFreq();
}

/* 使用说明:
 * 先调用EAP_CMD_SET_FREQUENCY_MODE设置为定频模式DEFINED_FREQ_MODE,然后再调用EAP_CMD_SET_FREQUENCY设置指定频率
 */
void FrequencyCmdproc::_OnSetFrequencyCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    FrequencyModule* frequencyModule = dynamic_cast<FrequencyModule*>(_module);
    if (nullptr == frequencyModule) return;

    float freq = *((float *)inMsg.data);
    frequencyModule->SetFreq(freq);
}

void FrequencyCmdproc::_OnSetFrequencyModeCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    FrequencyModule* frequencyModule = dynamic_cast<FrequencyModule*>(_module);
    if (nullptr == frequencyModule) return;

    FreqDetector* detector = nullptr;
    uint8_t detectMode = *(inMsg.data);
    switch (detectMode)
    {
    case DEFINED_SWEEP_FREQ_MODE:
        detector = SweepFreqDetector::Instance();
        break;
    case AUTO_SWEEP_FREQ_MODE:
        detector = AutoFreqDetector::Instance();
        break;
    case DEFINED_FREQ_MODE:
        detector = DefinedFreqDetector::Instance();
        break;
    default:
        break;
    }
    if (nullptr == detector) return;
    detector->Init();
    frequencyModule->SetFreqDetector(detector);
}

void FrequencyCmdproc::_OnChgFrequencyParamCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    HalMgr* halMgr = EapGetSys().GetHalMgr();
    SYS_FrequencyParam_t params;
    Hal_Data inData = {1, inMsg.data};
    Hal_Data outData = {sizeof(SYS_FrequencyParam_t), (uint8_t*)&params};
    if (nullptr != halMgr && EAP_SUCCESS == halMgr->OnGet(EAP_DRVCODE_GET_DETECTION_PARAM, inData, outData))
    {
        DefinedFreqDetector::Instance()->SetParams(params.DefinedFreqParam.DefinedFreqIndex, params.DefinedFreqParam.DefinedFreq);
        SweepFreqDetector::Instance()->SetParams(params.DefinedSweepParam.DefinedFreq, params.DefinedSweepParam.DefinedFreqValideNo);
        AutoFreqDetector::Instance()->SetParams(params.AutoSweepParam.StartCenterFreq, params.AutoSweepParam.EndCenterFreq,
            params.AutoSweepParam.CenterFreqInc);
    }
}

void FrequencyCmdproc::_OnShowFrequencyModuleCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    _DoShowModuleInfo();
}