#pragma once

#include "eap_pub.h"
#include "ModuleReal.h"


class VehicleMountedModule : public ModuleReal
{
public:
	VehicleMountedModule(uint8_t coreId);
	uint8_t GetModuleId(void);
	CmdprocBase *getCmdProc(void);
protected:

	ThreadBase* _CreateThread() override;
	CmdprocBase* _CreateCmdproc() override;
private:
	CmdprocBase* _cmdproc;
};

extern "C" {
	VehicleMountedModule* GetVehicleMountedModuler();
}

