#pragma once

#include "ModuleReal.h"

class FreqDetector;
class FrequencyModule : public ModuleReal
{
public:
    FrequencyModule(uint8_t coreId);
    void ChangeFreq();
    void SetFreq(float freq);
    void SetFreqDetector(FreqDetector* detector);
    uint8_t GetModuleId() override;
    void RequestShowInfo() override;
    void ShowInfo(uint8_t level) override;
protected:
    ThreadBase* _CreateThread() override;
    CmdprocBase* _CreateCmdproc() override;
private:
    FreqDetector* _detector;
};

