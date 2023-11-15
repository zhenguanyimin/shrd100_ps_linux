#include "AlinkModule.h"
#include "AlinkCmdproc.h"
#include "AlinkThread.h"

AlinkModule::AlinkModule(uint8_t coreId)
    : ModuleReal(coreId)
{
}

uint8_t AlinkModule::GetModuleId()
{
    return EAP_MODULE_ID_ALINK;
}

CmdprocBase* AlinkModule::_CreateCmdproc()
{
    return new AlinkCmdproc();
}

ThreadBase* AlinkModule::_CreateThread()
{
    return ADD_THREAD(AlinkThread, _coreId);
}
