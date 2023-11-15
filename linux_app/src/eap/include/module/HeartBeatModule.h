#pragma once

#include <map>
#include <set>
#include "ModuleReal.h"
extern "C"
{
}

using namespace std;

class WarningBase;
class HeartBeatCmdproc;

class HeartBeatModule : public ModuleReal
{
public:
    HeartBeatModule(uint8_t coreId);
    int16_t FillInfo(uint8_t* buf, uint16_t bufLen);
    void SetWorkStatus(uint8_t status) { _workStatus = status; }
    void SetFaultStatus(uint8_t status) { _faultStatus = status; }
    void SetCovertMode(uint8_t mode) { _covertMode = mode; }
    uint8_t GetModuleId() override;
    void TargetAppear(uint8_t targetType, uint8_t appear);
    void InstallWarningObj(WarningBase* warning);
    void RequestShowInfo() override;
    void ShowInfo(uint8_t level) override;
    CmdprocBase* GetCmdProcPtr(){return ModuleReal::_cmdproc;}
    void Init() override;

    virtual void OnSilentAction(void);  // do actions when mute switch is on
protected:
    ThreadBase* _CreateThread() override;
    CmdprocBase* _CreateCmdproc() override;
    uint8_t _GetAlarmLevel(uint8_t gearPosition);
    uint8_t _GetTargetState();
    void _TargetWarningProc(uint8_t targetAppear);
private:
    uint8_t _workStatus;
    uint8_t _faultStatus;
    map<uint8_t, uint8_t> _targets;
    set<WarningBase*> _warnings;
    uint8_t _workMode;
    uint8_t _covertMode;    // 隐蔽模式
};

