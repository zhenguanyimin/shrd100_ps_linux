#include "BlueToothThread.h"

DEFINE_THREAD(BlueToothThread, BLUETOOTH)

void BlueToothThread::_InitQueue(uint16_t& num)
{
    num = 10;
}

