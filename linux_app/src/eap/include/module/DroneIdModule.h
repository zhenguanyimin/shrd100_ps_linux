#pragma once

#include <map>
#include <string>

#include "ModuleReal.h"
#include "RptController.h"

extern "C"
{
#include "droneID_system_param.h"
}

using namespace std;

class DroneIdModule : public ModuleReal
{
public:
    DroneIdModule(uint8_t coreId);
    void Init() override;
    void RunDroneAlg(eDATA_PATH_INTR_FLAG flag,int channel);
    void RunDroneAlgNew(eDATA_PATH_INTR_FLAG flag,int channel);
    int16_t FillDroneIdInfo(uint8_t* buf, uint16_t bufLen);
    uint16_t GetDroneIdNum();
    void Clear();
    void OnTimer();
    uint8_t GetModuleId() override;
    void RequestShowInfo() override;
    void ShowInfo(uint8_t level) override;
protected:
    int16_t _FillDroneIdInfo(uint8_t* buf, uint16_t bufLen);
    int16_t _FillDroneIdInfoPB(uint8_t* buf, uint16_t bufLen);
    ThreadBase* _CreateThread() override;
    CmdprocBase* _CreateCmdproc() override;
    void _SetAlgFreq(uint64_t sigFreq, uint32_t freqOffset);
    void _RunAlg(eDATA_PATH_INTR_FLAG flag,int channel);
    void _RunAlgNew(eDATA_PATH_INTR_FLAG flag,int channel);
    void _TargetWarningProc(uint16_t oldTargetNum);
    void _TargetWarningProcWhitelist();
private:
    uint32_t _curFreq;
    uint64_t _bufferAdc[EAP_DRONEID_BUFFER_LEN];
    DroneID_ParseAllInfo_t _parseAllInfo[EAP_DRONEID_MAX_NUM];
    uint32_t _droneIdNum;
    RptController _rptCtrl;
    uint32_t _enemyDroneIdNum;
};

