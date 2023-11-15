#pragma once

#include "WarningBase.h"

class MotorWarning : public WarningBase
{
    DECLARE_SINGLETON(MotorWarning)
public:
    virtual void InitWarningLevelArray() override;

    void FillAlarmLevel(uint8_t& alarmLevelValue) override;

protected:
    uint16_t _GetDrvCode() override;
private:
};
