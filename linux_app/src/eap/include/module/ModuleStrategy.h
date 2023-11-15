#pragma once

#include <iostream>
#include <iomanip>
#include "eap_pub.h"

using namespace std;

class ModuleStrategy
{
public:
    virtual ~ModuleStrategy(){}
    virtual void Init() = 0;
    virtual uint8_t GetModuleId() = 0;
    virtual void ShowInfo(uint8_t level){}
    virtual void RequestShowInfo(){}
    virtual void ShowKeyInfo(uint8_t level){}

    virtual bool isLogByPass(const log_level_t level) const {return (level > _logFilterLevel);}

    virtual log_level_t getLogFilterLevel(void) const {return _logFilterLevel;}
    virtual void setLogFilterLevel(const log_level_t level) {_logFilterLevel = level;}

protected:
private:
    log_level_t _logFilterLevel = LL_ANY;
};

