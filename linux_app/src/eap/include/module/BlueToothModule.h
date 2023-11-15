#pragma once

#include "eap_pub.h"
#include "ModuleReal.h"

extern "C"
{

}

class BlueToothModule : public ModuleReal
{
public:
    BlueToothModule(uint8_t coreId);
	uint8_t GetModuleId(void);

protected:

	ThreadBase* _CreateThread() override;
	CmdprocBase* _CreateCmdproc() override;
private:

};
