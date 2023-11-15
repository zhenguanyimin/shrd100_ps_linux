#pragma once

#include "ModuleStrategy.h"

class ModuleProxy : public ModuleStrategy
{
public:
    ModuleProxy(uint16_t threadId);
protected:
    uint16_t _threadId;
private:
};
