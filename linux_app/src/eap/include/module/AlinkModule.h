#pragma once

#include "ModuleReal.h"

class AlinkModule : public ModuleReal
{
public:
    AlinkModule(uint8_t coreId);
    uint8_t GetModuleId() override;
protected:
    ThreadBase* _CreateThread() override;
    CmdprocBase* _CreateCmdproc() override;
private:
};

