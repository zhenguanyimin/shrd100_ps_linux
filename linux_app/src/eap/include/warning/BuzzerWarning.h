#pragma once

#include "WarningBase.h"

class BuzzerWarning : public WarningBase
{
    DECLARE_SINGLETON(BuzzerWarning)
public:
    virtual void InitWarningLevelArray() override;

    void FillAlarmLevel(uint8_t& alarmLevelValue) override;

protected:
    uint16_t _GetDrvCode() override;
private:
};
