#pragma once

#include "eap_pub.h"
#include "SysBase.h"

class WarningBase
{
public:
    WarningBase() { _targetState = 0; _workMode = 0; _gearPosition = 0; InitWarningLevelArray();}
    virtual ~WarningBase(){}
    void TargetProc(uint8_t targetAppear);
    void OrientationStatusProc(uint8_t workMode);
    void GearPositionProc(uint8_t gearPosition);
    uint8_t _GetWarningLevel();
    virtual void FillAlarmLevel(uint8_t& alarmLevelValue) {}
    virtual void ShowInfo(uint8_t level);
    virtual void InitWarningLevelArray();
    void SetWarningLevel(uint8_t gearPosition,uint8_t targetState,uint8_t workMode,uint8_t level);
protected:
    virtual uint16_t _GetDrvCode() = 0;
    uint8_t _targetState;
    uint8_t _workMode;
    uint8_t _gearPosition;

    /* 数组结构[拨码档位 0-3][目标状态 0 1][工作模式 1 2 3 对应数组索引 0 1 2] */
    uint8_t _warningLevelArray[EAP_GEAR_POSITION_NUM][EAP_TARGET_STATE_NUM][EAP_WARN_WORK_MODE_NUM];
private:
};

