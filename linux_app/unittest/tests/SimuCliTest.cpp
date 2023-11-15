#include "SimuCliTest.h"

#include "eap_os_adapter.h"
#include "SysBase.h"
#include "ThreadBase.h"
#include "CmdprocBase.h"
#include "ModuleReal.h"
#include "Module.h"
#include "CliThread.h"
#include "CLI_Template.h"
#include "ModuleProxy.h"
#include "HalMgr.h"
#include "HalBase.h"
#include "StubFunc.h"

extern "C"
{
    extern Thread_Msg stubReqMsg;
}

DEFINE_CLI_HANDLER1(TEST1, 101, "\r\n TEST1 <enable>:\r\n (0:not, 1:yes)\r\n")
DEFINE_CLI_HANDLER2(TEST2, 102, "\r\n TEST2 <enable1>:\r\n (0:not, 1:yes)\r\n <enable2>:\r\n (0:not, 1:yes)\r\n")

class CliSimuCmdproc : public CmdprocBase
{
public:
    static int proc_cmdcode;

protected:
    virtual void _RegisterCmd()
    {
        Register(101, (Cmdproc_Func)&CliSimuCmdproc::_SimuProcCmd1, &TEST1_cmd);
    }
    void _SimuProcCmd1(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
    {
        proc_cmdcode = 101;
    }
};

int CliSimuCmdproc::proc_cmdcode = 0;

class CliSimuThread : public ThreadBase
{
    DECLARE_THREAD(CliSimuThread)
public:
    bool CheckMsg()
    {
        return _IsMatched(stubReqMsg);
    }
protected:
    virtual void _InitQueue(uint16_t& num)
    {
        num = 1;
    }
};
DEFINE_THREAD(CliSimuThread, TEST1)

class CliSimuModule : public ModuleReal
{
public:
    CliSimuCmdproc* _simuCmdproc;
    CliSimuThread* _simuThread;
    CliSimuModule(uint8_t coreId) : ModuleReal(coreId)
    {
        _simuCmdproc = nullptr;
        _simuThread = nullptr;
    }
    uint8_t GetModuleId() override
    {
        return EAP_MODULE_ID_TEST1;
    }
protected:
    CmdprocBase* _CreateCmdproc() override
    {
        _simuCmdproc = new CliSimuCmdproc();
        return _simuCmdproc;
    }
    ThreadBase* _CreateThread() override
    {
        _simuThread = (CliSimuThread*)ADD_THREAD(CliSimuThread, 1);
        return _simuThread;
    }
};

class CliSimuModule2 : public ModuleProxy
{
public:
    CliSimuModule2(uint16_t threadId) : ModuleProxy(threadId){}
    virtual void Init()
    {
        CmdprocBase::ProxyRegister(102, _threadId, &TEST2_cmd);
    }
    uint8_t GetModuleId() override
    {
        return EAP_MODULE_ID_TEST2;
    }
};

Hal_Data simuInData;
class CliSimuHal : public HalBase
{
public:
    CliSimuHal(HalMgr* halMgr) : HalBase(halMgr){}
    virtual void Init()
    {
        _Register(EAP_DRVCODE_CLI_OUT, (Hal_Func)&CliSimuHal::_CliOut);
    }
protected:
    int _CliOut(const Hal_Data& inData, Hal_Data& outData)
    {
        memcpy(&simuInData, &inData, sizeof(simuInData));
        return EAP_SUCCESS;
    }
private:
};

class CliSimuHalMgr : public HalMgr
{
public:
    CliSimuHalMgr()
    {
    }
protected:
    void _InstallHals() override
    {
        AddHal(new CliSimuHal(this));
    }
private:
};

class CliSimuSys : public SysBase
{
public:
    CliSimuSys(uint8_t deviceId, int16_t processId) : SysBase(deviceId, processId)
    {
        simuModule = new CliSimuModule(1);
        _InstallModule(new Module(simuModule));
        simuModule2 = new CliSimuModule2(6);
        _InstallModule(new Module(simuModule2));
        _InstallHalMgr(new CliSimuHalMgr());
    }
    void Init()
    {
        cliThread = (CliThread*)ADD_THREAD(CliThread, 0);
        SysBase::Init();
    }
    CliSimuModule* simuModule;
    CliSimuModule2* simuModule2;
    CliThread* cliThread;
};

CliSimuSys simuSys(1, 1);

void SimuCliTest::SetUpTestSuite()
{
    Stub_ReInit(&simuSys);
}

void SimuCliTest::TearDownTestSuite()
{
    Stub_Restore();
}

TEST_F_A(SimuCliTest, LocalCliTest, wuqingA23199)
{
	// ----- Pre Test -----
    CliSimuCmdproc::proc_cmdcode = 0;

	// ----- Testing -----
    char data[100] = "TEST1 100";
    uint32_t dataLen = strlen(data);    
    SendAsynData(101, (uint8_t*)&data, dataLen);
    simuSys.cliThread->Run();
    simuSys.simuModule->_simuThread->Run();

    EXPECT_EQ(101, CliSimuCmdproc::proc_cmdcode);
    EXPECT_STREQ("OK", simuSys.cliThread->GetOutputString());
    EXPECT_EQ(2, (int)simuInData.length);
    EXPECT_STREQ("OK", (char *)simuInData.buf);
	// ----- Post Test -----
}

TEST_F_A(SimuCliTest, RemoteCliTest, wuqingA23199)
{
	// ----- Pre Test -----
    eap_set_processid_for_remote(6, 2);
    eap_set_queue_handle_for_remote(6, 2);

	// ----- Testing -----
    char data[100] = "TEST2 200 201";
    uint32_t dataLen = strlen(data);
    SendAsynData(102, (uint8_t*)data, dataLen);
    simuSys.cliThread->Run();
 
    Thread_Msg receiveMsg;
    eap_queue_receive(0, &receiveMsg, sizeof(Thread_Msg), 0);
    EXPECT_EQ(EAP_GET_LOC(2, 6), receiveMsg.head.to);
    EXPECT_EQ(EAP_GET_LOC(1, UINT16_INVALID), receiveMsg.head.from);
    EXPECT_EQ(8, (int)receiveMsg.head.length);
    EXPECT_EQ(200, (int)(*(uint32_t*)receiveMsg.buf));
    EXPECT_EQ(201, (int)(*(uint32_t*)(receiveMsg.buf + 4)));

	// ----- Post Test -----
}

