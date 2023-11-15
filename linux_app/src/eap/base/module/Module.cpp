#include <assert.h>
#include "Module.h"
#include "ModuleStrategy.h"

Module::Module(ModuleStrategy* strategy) : _strategy(strategy)
{
    _moduleId = _strategy->GetModuleId();
}

Module::~Module()
{
    EAP_DELETE(_strategy);
}

ModuleStrategy& Module::Strategy()
{
    assert(nullptr != _strategy);
    return *_strategy;
}

void Module::PreInit()
{
    Strategy().Init();
}

void Module::ShowInfo()
{
    cout << "\n" << "_moduleId: " << (uint16_t)_moduleId;
    if (nullptr != _strategy)
    {
        cout << EAP_BLANK_PREFIX(1) << "_strategy: ";
        _strategy->ShowInfo(2);
    }
}

void Module::RequestShowInfo()
{
    cout << "\n" << "_moduleId: " << (uint16_t)_moduleId;
    if (nullptr != _strategy)
    {
        cout << EAP_BLANK_PREFIX(1) << "_strategy: ";
        _strategy->RequestShowInfo();
    }
}