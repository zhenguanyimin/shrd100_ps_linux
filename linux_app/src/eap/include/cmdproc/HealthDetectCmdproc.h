#include "CmdprocBase.h"

class HealthDetectCmdproc : public CmdprocBase
{
public:
protected:
    void _RegisterCmd() override;
    void _OnRunHealthDetect(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnThreadRunPause(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnThreadRunIdling(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnThreadRunResume(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnTimer(uint8_t timerType) override;
};
