#include "CmdprocBase.h"

extern "C"
{
#include "sfDroneSniffer.h"
}

class DroneSnifferCmdproc : public CmdprocBase
{
public:
    DroneSnifferCmdproc(uint32_t droneNum);
    ~DroneSnifferCmdproc();
    void SetSendBuf(uint32_t bufLen) override;
protected:
    void _RegisterCmd() override;
    void _OnDroneSnifferCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    int32_t _GetDroneMaxNum();
private:
    droneResult_t* _droneInfo;
    int32_t _droneNum;
};
