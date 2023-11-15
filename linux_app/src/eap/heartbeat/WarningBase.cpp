#include <iostream>
#include <iomanip>
#include "WarningBase.h"
#include "MotorWarning.h"
#include "SysBase.h"
#include "HalMgr.h"

using namespace std;

void WarningBase::ShowInfo(uint8_t level)
{
    cout << EAP_BLANK_PREFIX(level) << "Class: " << typeid(*this).name();
    cout << EAP_BLANK_PREFIX(level) << "_targetState: " << (uint16_t)_targetState;
}

void WarningBase::TargetProc(uint8_t targetAppear)
{
    HalMgr* halMgr = EapGetSys().GetHalMgr();
    _targetState = targetAppear;
    if (nullptr != halMgr)
    {
        uint8_t level = _GetWarningLevel();
        Hal_Data inData = {sizeof(level), &level};
        halMgr->OnSet(_GetDrvCode(), inData);
    }
}

void WarningBase::OrientationStatusProc(uint8_t workMode)
{
    HalMgr* halMgr = EapGetSys().GetHalMgr();
    if (nullptr != halMgr && workMode != _workMode)
    {
        _workMode = workMode;
        uint8_t level = _GetWarningLevel();
        Hal_Data inData = {sizeof(level), &level};
        halMgr->OnSet(_GetDrvCode(), inData);
    }
}

void WarningBase::GearPositionProc(uint8_t gearPosition)
{
    HalMgr* halMgr = EapGetSys().GetHalMgr();
    if (nullptr != halMgr && gearPosition != _gearPosition)
    {
        _gearPosition = gearPosition;
        uint8_t level = _GetWarningLevel();
        Hal_Data inData = {sizeof(level), &level};
        halMgr->OnSet(_GetDrvCode(), inData);
    }
}

void WarningBase::SetWarningLevel(uint8_t gearPosition,uint8_t targetState,uint8_t workMode,uint8_t level)
{
    if(gearPosition < EAP_GEAR_POSITION_NUM && targetState < EAP_TARGET_STATE_NUM
        && workMode > 0 && workMode <= EAP_WARN_WORK_MODE_NUM && level < EAP_WARNING_LEVEL_MAX)
    {
        _warningLevelArray[gearPosition][targetState][workMode-1] = level;
    }
}

uint8_t WarningBase::_GetWarningLevel()
{
    if (_workMode > 0)
        return _warningLevelArray[_gearPosition][_targetState][_workMode-1];
    else
        return WARNING_LEVEL_NONE;
}

void WarningBase::InitWarningLevelArray()
{
    //level 1
    _warningLevelArray[EAP_GEAR_POSITION_0][EAP_TARGET_STATE_NOT][DRONE]               = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_0][EAP_TARGET_STATE_NOT][OMNI]                = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_0][EAP_TARGET_STATE_NOT][OMNI_ORIE]           = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_0][EAP_TARGET_STATE_NOT][OMNI_ORIE_DISCOVERY] = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_0][EAP_TARGET_STATE_YES][DRONE]               = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_0][EAP_TARGET_STATE_YES][OMNI]                = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_0][EAP_TARGET_STATE_YES][OMNI_ORIE]           = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_0][EAP_TARGET_STATE_YES][OMNI_ORIE_DISCOVERY] = WARNING_LEVEL_NONE;

    //level 2
    _warningLevelArray[EAP_GEAR_POSITION_1][EAP_TARGET_STATE_NOT][DRONE]               = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_1][EAP_TARGET_STATE_NOT][OMNI]                = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_1][EAP_TARGET_STATE_NOT][OMNI_ORIE]           = WARNING_LEVEL_LOW;
    _warningLevelArray[EAP_GEAR_POSITION_1][EAP_TARGET_STATE_NOT][OMNI_ORIE_DISCOVERY] = WARNING_LEVEL_LOW;
    _warningLevelArray[EAP_GEAR_POSITION_1][EAP_TARGET_STATE_YES][DRONE]               = WARNING_LEVEL_LOW;
    _warningLevelArray[EAP_GEAR_POSITION_1][EAP_TARGET_STATE_YES][OMNI]                = WARNING_LEVEL_LOW;
    _warningLevelArray[EAP_GEAR_POSITION_1][EAP_TARGET_STATE_YES][OMNI_ORIE]           = WARNING_LEVEL_LOW;
    _warningLevelArray[EAP_GEAR_POSITION_1][EAP_TARGET_STATE_NOT][OMNI_ORIE_DISCOVERY] = WARNING_LEVEL_LOW;

    //level 3
    _warningLevelArray[EAP_GEAR_POSITION_2][EAP_TARGET_STATE_NOT][DRONE]               = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_2][EAP_TARGET_STATE_NOT][OMNI]                = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_2][EAP_TARGET_STATE_NOT][OMNI_ORIE]           = WARNING_LEVEL_LOW;
    _warningLevelArray[EAP_GEAR_POSITION_2][EAP_TARGET_STATE_NOT][OMNI_ORIE_DISCOVERY] = WARNING_LEVEL_LOW;
    _warningLevelArray[EAP_GEAR_POSITION_2][EAP_TARGET_STATE_YES][DRONE]               = WARNING_LEVEL_MEDIUM;
    _warningLevelArray[EAP_GEAR_POSITION_2][EAP_TARGET_STATE_YES][OMNI]                = WARNING_LEVEL_MEDIUM;
    _warningLevelArray[EAP_GEAR_POSITION_2][EAP_TARGET_STATE_YES][OMNI_ORIE]           = WARNING_LEVEL_MEDIUM;
    _warningLevelArray[EAP_GEAR_POSITION_2][EAP_TARGET_STATE_YES][OMNI_ORIE_DISCOVERY] = WARNING_LEVEL_MEDIUM;

    //level 4
    _warningLevelArray[EAP_GEAR_POSITION_3][EAP_TARGET_STATE_NOT][DRONE]               = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_3][EAP_TARGET_STATE_NOT][OMNI]                = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_3][EAP_TARGET_STATE_NOT][OMNI_ORIE]           = WARNING_LEVEL_LOW;
    _warningLevelArray[EAP_GEAR_POSITION_3][EAP_TARGET_STATE_NOT][OMNI_ORIE_DISCOVERY] = WARNING_LEVEL_LOW;
    _warningLevelArray[EAP_GEAR_POSITION_3][EAP_TARGET_STATE_YES][DRONE]               = WARNING_LEVEL_HIGH;
    _warningLevelArray[EAP_GEAR_POSITION_3][EAP_TARGET_STATE_YES][OMNI]                = WARNING_LEVEL_HIGH;
    _warningLevelArray[EAP_GEAR_POSITION_3][EAP_TARGET_STATE_YES][OMNI_ORIE]           = WARNING_LEVEL_HIGH;
    _warningLevelArray[EAP_GEAR_POSITION_3][EAP_TARGET_STATE_YES][OMNI_ORIE_DISCOVERY] = WARNING_LEVEL_HIGH;

}
