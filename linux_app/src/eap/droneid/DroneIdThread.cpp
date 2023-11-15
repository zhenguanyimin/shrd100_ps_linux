#include "DroneIdThread.h"

DEFINE_THREAD(DroneIdThread, DRONEID)
DEFINE_TIMER1(DroneIdThread, EAP_DRONEID_FREQ_TIMER, true)
DEFINE_TIMER2(DroneIdThread, EAP_DRONEID_REP_TIMER, true)

void DroneIdThread::_InitQueue(uint16_t& num)
{
    num = 1000;
}

void DroneIdThread::_StartTimer()
{
    _StartTimer1();
    _StartTimer2();
}