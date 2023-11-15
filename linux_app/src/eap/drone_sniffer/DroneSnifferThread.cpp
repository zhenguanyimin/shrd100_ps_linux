#include "DroneSnifferThread.h"

DEFINE_THREAD(DroneSnifferThread, DRONE_SNIFFER)

void DroneSnifferThread::_InitQueue(uint16_t& num)
{
    num = 10;
}

