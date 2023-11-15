#include "VehicleMountedModule.h"
#include "VehicleMountedCmdproc.h"
#include "VehicleMountedThread.h"

#define VEHICLE_MOUNTED_DATA_LEN 100

extern "C" 
{
	VehicleMountedModule* GetVehicleMountedModuler()
	{
		return dynamic_cast<VehicleMountedModule*>(EapGetSys().GetModuleStrategy(EAP_MODULE_ID_VEHICLE_MOUNTED));
	}
}

VehicleMountedModule::VehicleMountedModule(uint8_t coreId)
	: ModuleReal(coreId)
{
}

uint8_t VehicleMountedModule::GetModuleId()
{
	return EAP_MODULE_ID_VEHICLE_MOUNTED;
}

CmdprocBase* VehicleMountedModule::_CreateCmdproc()
{
	CmdprocBase* cmdproc = new VehicleMountedCmdproc();
	cmdproc->SetRecvBuf(VEHICLE_MOUNTED_DATA_LEN + sizeof(Thread_Msg_Head));
	cmdproc->SetSendBuf(VEHICLE_MOUNTED_DATA_LEN + sizeof(Thread_Msg_Head));
	_cmdproc = cmdproc;
	
	return cmdproc;
}

CmdprocBase* VehicleMountedModule::getCmdProc(void)
{
	return _cmdproc;
}

ThreadBase* VehicleMountedModule::_CreateThread()
{
	return ADD_THREAD(VehicleMountedThread, _coreId);
}


