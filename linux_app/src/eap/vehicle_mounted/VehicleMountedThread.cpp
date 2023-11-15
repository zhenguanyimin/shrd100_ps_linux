#include "VehicleMountedThread.h"

DEFINE_THREAD(VehicleMountedThread, VEHICLEMOUNTED)

void VehicleMountedThread::_InitQueue(uint16_t& num)
{
	num = 10;
}

