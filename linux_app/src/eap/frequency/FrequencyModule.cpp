#include "FrequencyModule.h"
#include "SysBase.h"
#include "HalMgr.h"
#include "FreqDetector.h"
#include "FrequencyCmdproc.h"
#include "FrequencyThread.h"
#include "DefinedFreqDetector.h"

extern "C"
{
}

FrequencyModule::FrequencyModule(uint8_t coreId) : ModuleReal(coreId)
{
    _detector = nullptr;
}

uint8_t FrequencyModule::GetModuleId()
{
    return EAP_MODULE_ID_FREQUENCY;
}

void FrequencyModule::ChangeFreq()
{
    if (nullptr != _detector)
    {
        float nextFreq = _detector->NextFreq();
        HalMgr* halMgr = EapGetSys().GetHalMgr();
        Drv_SetDetectFreq data;
        data.mode = EapGetSys().GetWorkMode();
        data.freqIndex = _detector->GetCurIndex();
        data.freq = nextFreq;
        Hal_Data inData = {sizeof(Drv_SetDetectFreq), (uint8_t*)&data};
        if (nullptr != halMgr) halMgr->OnSet(EAP_DRVCODE_SET_DETECT_FREQ, inData);
        EapGetSys().SetCurDetectFreq(nextFreq);
    }
}

void FrequencyModule::SetFreq(float freq)
{
    if (nullptr != _detector)
    {
        HalMgr* halMgr = EapGetSys().GetHalMgr();
        Drv_SetDetectFreq data;
        data.mode = EapGetSys().GetWorkMode();
        data.freqIndex = _detector->GetCurIndex();
        data.freq = freq;
        Hal_Data inData = {sizeof(Drv_SetDetectFreq), (uint8_t*)&data};
        if (nullptr != halMgr) halMgr->OnSet(EAP_DRVCODE_SET_DETECT_FREQ, inData);

        DefinedFreqDetector* newDetector = dynamic_cast<DefinedFreqDetector*>(_detector);
        if (nullptr != newDetector)
        {
            newDetector->SetDefinedFreq((uint32_t)freq);
            EAP_LOG_DEBUG("设置固定频率为%0.1f:\n", freq);
        }
    }
}

CmdprocBase* FrequencyModule::_CreateCmdproc()
{
    return new FrequencyCmdproc();
}

ThreadBase* FrequencyModule::_CreateThread()
{
    return ADD_THREAD(FrequencyThread, _coreId);
}

void FrequencyModule::SetFreqDetector(FreqDetector* detector)
{
    if (_detector == detector) return;
    DefinedFreqDetector* newDetector = dynamic_cast<DefinedFreqDetector*>(detector);
    if (nullptr != newDetector && nullptr != _detector)
    {
        newDetector->SetDefinedFreq((uint32_t)EapGetSys().GetCurDetectFreq());
    }

    _detector = detector;
}

void FrequencyModule::RequestShowInfo()
{
    SendAsynCmdCode(EAP_CMD_SHOW_FREQUENCY_MODULE);
}

void FrequencyModule::ShowInfo(uint8_t level)
{
    ModuleReal::ShowInfo(level);
    if (nullptr != _detector)
    {
        cout << "\n" << "_detector: ";
        _detector->ShowInfo(level + 1);
    }
}
