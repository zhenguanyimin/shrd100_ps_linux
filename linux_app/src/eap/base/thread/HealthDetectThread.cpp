#include "HealthDetectThread.h"
#include "SysBase.h"

DEFINE_THREAD(HealthDetectThread, HEALTH_DETECT)
DEFINE_TIMER1(HealthDetectThread, EAP_HEALTH_DETECT_TIMER, false)

void HealthDetectThread::_StartTimer()
{
    _StartTimer1();
}

void HealthDetectThread::StartDelayTimer(uint8_t timerCode)
{
    if (EAP_CMDTYPE_TIMER_1 == timerCode) _StartTimer1();
}