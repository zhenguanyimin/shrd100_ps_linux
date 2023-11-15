#include "ModuleReal.h"
#include "ThreadBase.h"
#include "CmdprocBase.h"

ModuleReal::ModuleReal(uint8_t coreId)
    : _cmdproc(nullptr), _thread(nullptr), _coreId(coreId)
{

}

ModuleReal::~ModuleReal()
{
    EAP_DELETE(_cmdproc);
}

void ModuleReal::Init()
{
    _thread = _CreateThread();
    _cmdproc = _CreateCmdproc();
    if (nullptr != _thread && nullptr != _cmdproc)
    {
        _SetRelation(*_thread, *_cmdproc);
    }
}

void ModuleReal::_SetRelation(ThreadBase& thread, CmdprocBase& cmdproc)
{
    cmdproc.SetModule(this);
    thread.SetCmdproc(&cmdproc);
    cmdproc.SetThreadId(thread.GetId());
}

void ModuleReal::ShowInfo(uint8_t level)
{
    ShowKeyInfo(level);
    cout << EAP_BLANK_PREFIX(level) << "_coreId: " << (uint16_t)_coreId;
    if (nullptr != _cmdproc)
    {
        cout << EAP_BLANK_PREFIX(level) << "_cmdproc: ";
        _cmdproc->ShowInfo(level + 1);
    }
    if (nullptr != _thread)
    {
        cout << EAP_BLANK_PREFIX(level) << "_thread: ";
        _thread->ShowInfo(level + 1);
    }
}

void ModuleReal::ShowKeyInfo(uint8_t level)
{
    cout << EAP_BLANK_PREFIX(level) << "Class: " << typeid(*this).name();
    cout << EAP_BLANK_PREFIX(level) << "ModuleId: " << (uint16_t)GetModuleId();
}