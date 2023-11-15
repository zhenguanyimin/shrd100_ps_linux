#include "HeartBeatThread.h"
#include "CmdprocBase.h"

DEFINE_THREAD(HeartBeatThread, HEARTBEAT)
DEFINE_TIMER1(HeartBeatThread, EAP_HEARTBEAT_TIMER, true)
DEFINE_TIMER2(HeartBeatThread, EAP_C2_HEARTBEAT_CHECK_TIMER, true)
DEFINE_TIMER3(HeartBeatThread, EAP_STD_ERR_LOG_REFRESH_TIMER, true)

void HeartBeatThread::_InitQueue(uint16_t& num)
{
    num = 1000;
}

void HeartBeatThread::_StartTimer()
{
    _StartTimer1();
    _StartTimer2();
    _StartTimer3();
}