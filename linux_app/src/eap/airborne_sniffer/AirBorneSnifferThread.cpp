#include "AirBorneSnifferThread.h"

DEFINE_THREAD(AirBorneSnifferThread, AIRBORNE_SNIFFER)

void AirBorneSnifferThread::_InitQueue(uint16_t& num)
{
    num = 10;
}

