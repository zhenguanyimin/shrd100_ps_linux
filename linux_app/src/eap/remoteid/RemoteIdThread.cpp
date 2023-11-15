#include "RemoteIdThread.h"

DEFINE_THREAD(RemoteIdThread, REMOTEID)
DEFINE_TIMER1(RemoteIdThread, EAP_REMOTEID_REP_TIMER, true)
DEFINE_TIMER2(RemoteIdThread, EAP_REMOTEID_REP_TIMER_WIFI, true)

void RemoteIdThread::_InitQueue(uint16_t& num)
{
    num = 2000;
}

void RemoteIdThread::_StartTimer()
{
    _StartTimer1();
    _StartTimer2();
}
