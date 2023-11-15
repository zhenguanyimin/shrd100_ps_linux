#include "AlinkThread.h"

DEFINE_THREAD(AlinkThread, ALINK)

void AlinkThread::_InitQueue(uint16_t& num)
{
    num = 10000;
}

