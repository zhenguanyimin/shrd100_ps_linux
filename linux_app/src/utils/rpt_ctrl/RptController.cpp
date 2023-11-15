#include "RptController.h"

RptController::RptController()
{
    _intervalTimes = 0;
}

void RptController::Refresh(uint16_t oldNums, uint16_t newNums)
{
    if (oldNums == newNums) return;
    if (0 == newNums) _intervalTimes = EAP_RPT_INTERVAL_NO_TARGET;
}

bool RptController::Report(uint16_t curNums)
{
    if (0 == curNums && _intervalTimes < EAP_RPT_INTERVAL_NO_TARGET)
    {
        ++_intervalTimes;
        return false;
    }
    return true;
}