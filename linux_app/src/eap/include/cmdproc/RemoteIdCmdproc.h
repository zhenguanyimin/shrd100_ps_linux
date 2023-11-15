#include "CmdprocBase.h"

class RemoteIdBufScanStrategy;

class RemoteIdCmdproc : public CmdprocBase
{
public:
    RemoteIdCmdproc();
    void SetScanStrategy(RemoteIdBufScanStrategy* scanStrategy) { _scanStrategy = scanStrategy; }
protected:
    void _OnTimer(uint8_t timerType) override;
    void _RegisterCmd() override;
    void _OnRemoteIdCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
	void _OnRemoteIdCmdRidCapture(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnShowRemoteIdModuleCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _ReportRemoteIdsInfo();
    int32_t _GetRemoteIdData(const Cmdproc_Data& inMsg, uint8_t* &data, int32_t& dataLen);
    RemoteIdBufScanStrategy* _scanStrategy;
};

uint32_t Remoteid_GetRemoteIdChnValue();

int rid_checkfilewhetherexist(char *filename);