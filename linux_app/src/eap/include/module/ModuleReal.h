#pragma once

#include "ModuleStrategy.h"

class ThreadBase;
class CmdprocBase;

class ModuleReal : public ModuleStrategy
{
public:
    ModuleReal(uint8_t coreId);
    ~ModuleReal();
    virtual void Init();
    void ShowInfo(uint8_t level) override;
    void ShowKeyInfo(uint8_t level) override;
protected:
    virtual ThreadBase* _CreateThread(){ return nullptr; }
    virtual CmdprocBase* _CreateCmdproc(){ return nullptr; }
    virtual void _RegisterCli(){}
    void _SetRelation(ThreadBase& thread, CmdprocBase& cmdproc);
    CmdprocBase* _cmdproc;
    ThreadBase* _thread;
    uint8_t _coreId;
private:
};
