#include "SysThread.h"
#include "SysBase.h"

DEFINE_THREAD(SysThread, SYS)
DEFINE_TIMER1(SysThread, EAP_SYS_QUERY_DRV_TIMER, true)
DEFINE_TIMER2(SysThread, EAP_SYS_DUP_RESPONSE_TIMER, true)

void SysThread::_InitQueue(uint16_t& num)
{
    num = 10;
}

void SysThread::_StartTimer()
{
    //_StartTimer1();
    _StartTimer2();
}