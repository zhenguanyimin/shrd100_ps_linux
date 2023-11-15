#include "SpectrumThread.h"
#include "SysBase.h"

DEFINE_THREAD(SpectrumThread, SPECTRUM)
DEFINE_TIMER1(SpectrumThread, EAP_SPECTRUM_REP_TIMER, true)

void SpectrumThread::_InitQueue(uint16_t& num)
{
    num = 2000;
}

void SpectrumThread::_StartTimer()
{
    _StartTimer1();
}