#include "CmdprocBase.h"
extern "C"
{
#include "sfAirBorneSniffer.h"
}

class SpectrumCmdproc : public CmdprocBase
{
public:
    SpectrumCmdproc();
    ~SpectrumCmdproc();
    void SetTargetMaxNum(uint16_t num);
protected:
    void _RegisterCmd() override;
    void _OnSpectrumCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnClearOrientateCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnShowSpectrumModuleCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _SetSpecMatrix(uint32_t *specData);
    void _OnTimer(uint8_t timerType) override;
    void _ReportSpectrumDroneInfo();

    // For AirBorne
    void _OnAirBorneCtrlCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnAirBorneFlyStatusCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _SetAirBorneMatrix(uint32_t *specData);
    void _ReportAirBorneInfo(ioMeasureInfo_t* result);
private:
    uint16_t _targetMaxNum;
    uint8_t* _respData;
    uint32_t _respMaxLen;
};
