#include "LedWarning.h"

DEFINE_SINGLETON_NO_CONSTRUCT(LedWarning)

LedWarning::LedWarning()
{
    InitWarningLevelArray();
}

uint16_t LedWarning::_GetDrvCode()
{
    return EAP_DRVCODE_SET_LED_STATE;
}

void LedWarning::FillAlarmLevel(uint8_t& alarmLevelValue)
{
    // 2023-11-11 参考《tracer与C2和上位机通信协议V1.0.14.docx》
    // bit0~bit3：告警等级；bit6：隐蔽模式；这里不再上报LED状态
    // alarmLevelValue |= (_targetState & 0x0F);
}

void LedWarning::InitWarningLevelArray()
{
    //level 1
    _warningLevelArray[EAP_GEAR_POSITION_0][EAP_TARGET_STATE_NOT][DRONE]               = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_0][EAP_TARGET_STATE_NOT][OMNI]                = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_0][EAP_TARGET_STATE_NOT][OMNI_ORIE]           = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_0][EAP_TARGET_STATE_NOT][OMNI_ORIE_DISCOVERY] = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_0][EAP_TARGET_STATE_YES][DRONE]               = WARNING_LEVEL_LOW;
    _warningLevelArray[EAP_GEAR_POSITION_0][EAP_TARGET_STATE_YES][OMNI]                = WARNING_LEVEL_LOW;
    _warningLevelArray[EAP_GEAR_POSITION_0][EAP_TARGET_STATE_YES][OMNI_ORIE]           = WARNING_LEVEL_LOW;
    _warningLevelArray[EAP_GEAR_POSITION_0][EAP_TARGET_STATE_YES][OMNI_ORIE_DISCOVERY] = WARNING_LEVEL_HIGH;

    //level 2
    _warningLevelArray[EAP_GEAR_POSITION_1][EAP_TARGET_STATE_NOT][DRONE]               = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_1][EAP_TARGET_STATE_NOT][OMNI]                = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_1][EAP_TARGET_STATE_NOT][OMNI_ORIE]           = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_1][EAP_TARGET_STATE_NOT][OMNI_ORIE_DISCOVERY] = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_1][EAP_TARGET_STATE_YES][DRONE]               = WARNING_LEVEL_MEDIUM;
    _warningLevelArray[EAP_GEAR_POSITION_1][EAP_TARGET_STATE_YES][OMNI]                = WARNING_LEVEL_MEDIUM;
    _warningLevelArray[EAP_GEAR_POSITION_1][EAP_TARGET_STATE_YES][OMNI_ORIE]           = WARNING_LEVEL_LOW;
    _warningLevelArray[EAP_GEAR_POSITION_1][EAP_TARGET_STATE_YES][OMNI_ORIE_DISCOVERY] = WARNING_LEVEL_HIGH;

    //level 3
    _warningLevelArray[EAP_GEAR_POSITION_2][EAP_TARGET_STATE_NOT][DRONE]               = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_2][EAP_TARGET_STATE_NOT][OMNI]                = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_2][EAP_TARGET_STATE_NOT][OMNI_ORIE]           = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_2][EAP_TARGET_STATE_NOT][OMNI_ORIE_DISCOVERY] = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_2][EAP_TARGET_STATE_YES][DRONE]               = WARNING_LEVEL_HIGH;
    _warningLevelArray[EAP_GEAR_POSITION_2][EAP_TARGET_STATE_YES][OMNI]                = WARNING_LEVEL_HIGH;
    _warningLevelArray[EAP_GEAR_POSITION_2][EAP_TARGET_STATE_YES][OMNI_ORIE]           = WARNING_LEVEL_LOW;
    _warningLevelArray[EAP_GEAR_POSITION_2][EAP_TARGET_STATE_YES][OMNI_ORIE_DISCOVERY] = WARNING_LEVEL_HIGH;

    //level 4
    _warningLevelArray[EAP_GEAR_POSITION_3][EAP_TARGET_STATE_NOT][DRONE]               = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_3][EAP_TARGET_STATE_NOT][OMNI]                = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_3][EAP_TARGET_STATE_NOT][OMNI_ORIE]           = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_3][EAP_TARGET_STATE_NOT][OMNI_ORIE_DISCOVERY] = WARNING_LEVEL_NONE;
    _warningLevelArray[EAP_GEAR_POSITION_3][EAP_TARGET_STATE_YES][DRONE]               = WARNING_LEVEL_HIGH;
    _warningLevelArray[EAP_GEAR_POSITION_3][EAP_TARGET_STATE_YES][OMNI]                = WARNING_LEVEL_HIGH;
    _warningLevelArray[EAP_GEAR_POSITION_3][EAP_TARGET_STATE_YES][OMNI_ORIE]           = WARNING_LEVEL_LOW;
    _warningLevelArray[EAP_GEAR_POSITION_3][EAP_TARGET_STATE_YES][OMNI_ORIE_DISCOVERY] = WARNING_LEVEL_HIGH;
}

