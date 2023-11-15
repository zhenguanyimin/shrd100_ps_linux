#include "CmdprocBase.h"

class SysCmdproc : public CmdprocBase
{
public:
protected:
    void _RegisterCmd() override;
    void _OnSetWorkModeCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnSetSnCode(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnGetSnCode(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnSetMsgStoreOn(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnSetDrvStoreOn(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnTimer(uint8_t timerType) override;
    void _QueryME();
};
