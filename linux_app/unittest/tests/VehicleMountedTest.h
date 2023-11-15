#include <stdio.h>
#include <stdlib.h>
#include "ModuleReal.h"
#include "CmdprocBase.h"


#if 1
class VehicleMountedCmdproc : public CmdprocBase
{
public:
	VehicleMountedCmdproc();
	~VehicleMountedCmdproc();

	void VehicleMountedSetFiredTime(uint8_t *firetime);
	void VehicleMountedGetFiredTime(uint8_t *firetime);

	void VehicleMountedSetFiredMode(uint8_t *fireMode);
	void VehicleMountedGetFiredMode(uint8_t *fireMode);

	void VehicleMountedAutoFire(SpectrumAlgResult *spect, int num);


};





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

#endif
