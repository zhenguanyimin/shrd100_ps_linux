#pragma once

#include <map>
#include <string>
#include "ModuleReal.h"
#include "RptController.h"

extern "C"
{
}

using namespace std;

typedef struct SpectrumDrone_Info
{
    float detectFreq;
    float actualFreq;
    float amp;
    uint8_t remainingTimes;
} SpectrumDrone_Info;

class OrientationProc;
class AirBorneProc;
class SpectrumModule : public ModuleReal
{
public:
    SpectrumModule(uint8_t coreId);
    ~SpectrumModule();
    int32_t get_angle_info();   // 全向模式临时从陀螺仪获取水平角
    int16_t FillSpectrumInfo(uint8_t* buf, uint16_t bufLen);
    void Sensing(float curDetectFreq, SpectrumAlgResult* result, uint16_t num);
    const SpectrumDrone_Info* GetOrientateDrone();
    void ClearOrientate();
    void AdminOrientate();
    void AirBorneCtrl(void *data);
    int32_t AirBorneGetParam(void *data);
    /* tracer-s SHRDl06L */
    void FiredDroneAuto(SpectrumAlgResult* result, uint16_t num);
    int FiredDroneGetFireMode(void);
    void Init() override;
    uint8_t GetModuleId() override;
    void RequestShowInfo() override;
    void ShowInfo(uint8_t level) override;
protected:
    ThreadBase* _CreateThread() override;
    CmdprocBase* _CreateCmdproc() override;
    void _SaveAlgResult(float curDetectFreq, SpectrumAlgResult* result, uint16_t num);
    void _TargetWarningProc(uint16_t oldTargetNum);
    void _FindDirectDrone();
    void _RefreshOrientateDrone();
    void _OldSpectrumDrone();
    OrientationProc& OriProc();
    AirBorneProc& ArbnProc();
private:
    map<string, SpectrumDrone_Info> _spectrumInfos;
    map<string, SpectrumDrone_Info>::iterator _orientateDroneIter;
    bool _hasOrientateDrone;
    OrientationProc* _oriProc;
    AirBorneProc* _arbnProc;
    RptController _rptCtrl;
};

