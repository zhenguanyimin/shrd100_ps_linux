#include "FrequencyThread.h"

DEFINE_THREAD(FrequencyThread, FREQUENCY)

void FrequencyThread::_InitQueue(uint16_t& num)
{
    num = 100;
}
