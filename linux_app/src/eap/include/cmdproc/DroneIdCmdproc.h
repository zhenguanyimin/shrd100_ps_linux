#include "CmdprocBase.h"

class DroneIdCmdproc : public CmdprocBase
{
public:
protected:
    void _OnTimer(uint8_t timerType) override;
    void _RegisterCmd() override;
    void _OnDroneIdCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnShowDroneIdModuleCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnSetAlgorithmPrint(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnSetRemoveThreshold(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnSetRemoveThresholdLostDuration(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnSetOutputSnMode(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnSetTurboDecIterNum(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnFreqTimer();
    void _ReportDroneIdsInfo();
};
