#include "VehicleMountedTest.h"

#if 1
VehicleMountedModule::VehicleMountedModule(uint8_t coreId)
	: ModuleReal(coreId)
{

}

uint8_t VehicleMountedModule::GetModuleId()
{
	return 0;
}

CmdprocBase* VehicleMountedModule::_CreateCmdproc()
{
	return nullptr;
}

CmdprocBase* VehicleMountedModule::getCmdProc(void)
{
	return nullptr;
}

ThreadBase* VehicleMountedModule::_CreateThread()
{
	return nullptr;
}


void VehicleMountedCmdproc::VehicleMountedSetFiredMode(uint8_t *fireMode)
{
	return ;
}




void VehicleMountedCmdproc::VehicleMountedGetFiredMode(uint8_t *fireMode)
{
	return ;
}

void VehicleMountedCmdproc::VehicleMountedSetFiredTime(uint8_t *firetime)
{
	return ;
}

void VehicleMountedCmdproc::VehicleMountedGetFiredTime(uint8_t *firetime)
{
	return;
}

void VehicleMountedCmdproc::VehicleMountedAutoFire(SpectrumAlgResult *spect, int num)
{
	return ;
}

extern "C" 
{
	VehicleMountedModule* GetVehicleMountedModuler()
	{
		return nullptr;
	}
}

#endif

