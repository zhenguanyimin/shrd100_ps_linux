#pragma once

#include <iostream>
#include <iomanip>
#include "eap_pub.h"

using namespace std;

class ModuleStrategy;
class Module
{
public:
    Module(ModuleStrategy* strategy);
    ~Module();
    void PreInit();
    ModuleStrategy& Strategy();
    uint8_t GetId() { return _moduleId; }
    void ShowInfo();
    void RequestShowInfo();
protected:
    ModuleStrategy* _strategy;
    uint8_t _moduleId;
private:
};
