#pragma once

#include "eap_pub.h"

class SpectrumModule;

#define LEVEL_CNT_MAX 128
#define PITCH_CNT_MAX 128

enum DATA_TYPE
{
	DATA_ANGLE = 0x00,
	DATA_DIR_AMP,
	DATA_OMNI_AMP,
};

class OrientationProc
{
public:
    OrientationProc(SpectrumModule& module);
    void Proc(float curDetectFreq);
    int32_t GetLevelAngleCen() { return _levelAngleCen; }
    int32_t GetPitchAngleCen() { return _pitchAngleCen; }
    void EnterAdminOri() { _adminEntryOri = true; }
    void Clear();
    bool IsOpenOrientation();
    void ShowInfo(uint8_t level);
protected:
    void SetOrientateDetMode();
private:
    void _ProcEnterOrientateStep(float curDetectFreq, float actualFreq, int32_t amp, uint8_t& oldDetectMode);
    void _ProcAzimuthEnoughStep();
    void _ProcAzimuthAimedStep(int32_t angle);
    void _ProcPitchEnoughStep(int32_t angle);
    void _ProcPitchAimedStep(int32_t pitch, int32_t angle);
    void _SaveAngleInfo(uint8_t oldDetectMode, int32_t angle, int32_t pitch, int32_t amp);
    uint8_t _state;
    uint8_t _step;
    bool _adminEntryOri;
    uint8_t _detectMode;
    SpectrumModule& _module;
    int32_t _levelDetects[3][LEVEL_CNT_MAX];
    int32_t _pitchAngles[3][PITCH_CNT_MAX];
    uint8_t _groupCnt;
    int32_t _levelAmpMax;
    int32_t _pitchAmpMax;
    int32_t _levelAngleCen;
    int32_t _pitchAngleCen;

    uint8_t _droneLocationMode;
    int32_t _droneAngle;
    int32_t _dronePitch;
};