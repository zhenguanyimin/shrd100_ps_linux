#include "SysTest.h"

#include "ThreadBase.h"
#include "ThreadQueue.h"
#include "Module.h"
#include "ModuleReal.h"
#include "SysBase.h"
#include "eap_sys_cmds.h"
#include "CmdprocBase.h"
#include "StubFunc.h"

extern "C"
{
    extern Thread_Msg stubReqMsg;
}

extern SysBase& EapGetSys();

class SysSimuCmdproc1 : public CmdprocBase
{
public:
    static int proc_cmdcode;

protected:
    virtual void _RegisterCmd()
    {
        Register(101, (Cmdproc_Func)&SysSimuCmdproc1::_SimuProcCmd1);
        Register(102, (Cmdproc_Func)&SysSimuCmdproc1::_SimuProcCmd2);
        Register(103, (Cmdproc_Func)&SysSimuCmdproc1::_SimuProcCmd3);
    }
    void _SimuProcCmd1(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
    {
        proc_cmdcode = 101;
    }
    void _SimuProcCmd2(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
    {
        proc_cmdcode = 102;
        outMsg.dataLen = 10;
    }
    void _SimuProcCmd3(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
    {
        proc_cmdcode = 103;

        uint32_t* reqData = (uint32_t*)_GetSendDataBuf();
        *reqData = 0x1234;
        uint8_t respData = 0;
        uint32_t respDataLen = 1;
        SendSynMsg(_threadId, 104, _sendBuf, sizeof(uint32_t), &respData, &respDataLen);
        outMsg.dataLen = 10;
    }
};

class SysSimuCmdproc2 : public CmdprocBase
{
public:
    static int proc_cmdcode;

protected:
    virtual void _RegisterCmd()
    {
        Register(104, (Cmdproc_Func)&SysSimuCmdproc2::_SimuProcCmd4);
    }
    void _SimuProcCmd4(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
    {
        uint32_t reqData = *((uint32_t*)(inMsg.data));
        EXPECT_EQ(0x1234, (int)reqData);
        EXPECT_EQ(4, (int)inMsg.dataLen);
        proc_cmdcode = 104;
        outMsg.dataLen = 20;
    }
};

int SysSimuCmdproc1::proc_cmdcode = 0;
int SysSimuCmdproc2::proc_cmdcode = 0;

class SysSimuThread1 : public ThreadBase
{
    DECLARE_THREAD(SysSimuThread1)
    DECLARE_TIMER1()
public:
    bool CheckMsg()
    {
        return _IsMatched(stubReqMsg);
    }
protected:
    virtual void _StartTimer()
    {
        _StartTimer1();
    }
    virtual void _InitQueue(uint16_t& num)
    {
        num = 1;
    }
};
DEFINE_THREAD(SysSimuThread1, TEST1)
DEFINE_TIMER1(SysSimuThread1, 1000, true)

class SysSimuThread2 : public ThreadBase
{
    DECLARE_THREAD(SysSimuThread2)
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
DEFINE_THREAD(SysSimuThread2, TEST2)

class SysSimuModule1 : public ModuleReal
{
public:
    SysSimuCmdproc1* _simuCmdproc1;
    SysSimuThread1* _simuThread1;
    SysSimuModule1(uint8_t coreId) : ModuleReal(coreId)
    {
        _simuCmdproc1 = nullptr;
        _simuThread1 = nullptr;
    }
    uint8_t GetModuleId() override
    {
        return EAP_MODULE_ID_TEST1;
    }
protected:
    CmdprocBase* _CreateCmdproc() override
    {
        _simuCmdproc1 = new SysSimuCmdproc1();
        return _simuCmdproc1;
    }
    ThreadBase* _CreateThread() override
    {
        _simuThread1 = (SysSimuThread1*)ADD_THREAD(SysSimuThread1, 1);
        return _simuThread1;
    }
};

class SysSimuModule2 : public ModuleReal
{
public:
    SysSimuCmdproc2* _simuCmdproc2;
    SysSimuThread2* _simuThread2;
    SysSimuModule2(uint8_t coreId) : ModuleReal(coreId)
    {
        _simuCmdproc2 = nullptr;
        _simuThread2 = nullptr;
    }
    uint8_t GetModuleId() override
    {
        return EAP_MODULE_ID_TEST2;
    }
protected:
    CmdprocBase* _CreateCmdproc() override
    {
        _simuCmdproc2 = new SysSimuCmdproc2();
        return _simuCmdproc2;
    }
    ThreadBase* _CreateThread() override
    {
        _simuThread2 = (SysSimuThread2*)ADD_THREAD(SysSimuThread2, 1);
        return _simuThread2;
    }
};

class SimuSys : public SysBase
{
public:
    SysSimuModule1* simuModule1;
    SysSimuModule2* simuModule2;
    SimuSys(uint8_t deviceId) : SysBase(deviceId)
    {
        simuModule1 = new SysSimuModule1(1);
        _InstallModule(new Module(simuModule1));
        simuModule2 = new SysSimuModule2(1);
        _InstallModule(new Module(simuModule2));
    }
};

SimuSys sysSimuSys(1);
extern SysBase* g_sys;

void SysTest::SetUpTestSuite()
{
    Stub_ReInit(&sysSimuSys);
}

void SysTest::TearDownTestSuite()
{
    Stub_Restore();
}

TEST_F_A(SysTest, SysInit, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    EXPECT_EQ(2, (int)EapGetSys().GetModulesNum());
    EXPECT_EQ(sysSimuSys.simuModule1->_simuThread1->GetId(), sysSimuSys.simuModule1->_simuCmdproc1->GetThreadId());
    EXPECT_EQ(sysSimuSys.simuModule1->_simuCmdproc1, sysSimuSys.simuModule1->_simuThread1->GetCmdproc());
    EXPECT_EQ(sysSimuSys.simuModule2->_simuThread2->GetId(), sysSimuSys.simuModule2->_simuCmdproc2->GetThreadId());
    EXPECT_EQ(sysSimuSys.simuModule2->_simuCmdproc2, sysSimuSys.simuModule2->_simuThread2->GetCmdproc());

	// ----- Post Test -----
}

TEST_F_A(SysTest, ConsumerCmdTest, wuqingA23199)
{
	// ----- Pre Test -----
    ThreadQueue* queue = sysSimuSys.simuModule1->_simuThread1->GetQueue();

	// ----- Testing -----
    Thread_Msg data;
    data.head.cmdType = 0;
    data.head.length = 1;
    queue->ProduceMsg((uint8_t*)&data, sizeof(data.head) + 1);
    sysSimuSys.simuModule1->_simuThread1->Run();

	// ----- Post Test -----
}

TEST_F_A(SysTest, ConsumerTimerCmdTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    timer_entry1();
    sysSimuSys.simuModule1->_simuThread1->Run();

	// ----- Post Test -----
}

TEST_F_A(SysTest, SendMsgNoRespTest, wuqingA23199)
{
	// ----- Pre Test -----
    SysSimuCmdproc1::proc_cmdcode = 0;

	// ----- Testing -----
    SendAsynCmdCode(101);
    sysSimuSys.simuModule1->_simuThread1->Run();

//    uint16_t respLen = stub_get_response_msg_len();
//    EXPECT_EQ(0, respLen);
    EXPECT_EQ(101, SysSimuCmdproc1::proc_cmdcode);

	// ----- Post Test -----
}

TEST_F_A(SysTest, SendSyncMsgTest, wuqingA23199)
{
	// ----- Pre Test -----
    SysSimuCmdproc1::proc_cmdcode = 0;

	// ----- Testing -----
    uint8_t data;
    uint32_t dataLen = 1;
    SendSynData(EAP_THREAD_ID_TEST1, 102, nullptr, 0, &data, &dataLen);
    sysSimuSys.simuModule1->_simuThread1->Run();

    EXPECT_EQ(102, SysSimuCmdproc1::proc_cmdcode);
    EXPECT_EQ(true, sysSimuSys.simuModule1->_simuThread1->CheckMsg());
    
	// ----- Post Test -----
}

TEST_F_A(SysTest, SendRecvSyncMsgTest, wuqingA23199)
{
	// ----- Pre Test -----
    SysSimuCmdproc1::proc_cmdcode = 0;
    SysSimuCmdproc2::proc_cmdcode = 0;

	// ----- Testing -----
    uint8_t data = 0;
    uint32_t dataLen = 1;
    SendSynData(EAP_THREAD_ID_TEST2, 103, nullptr, 0, &data, &dataLen);
    sysSimuSys.simuModule1->_simuThread1->Run();
    sysSimuSys.simuModule2->_simuThread2->Run();

    EXPECT_EQ(103, SysSimuCmdproc1::proc_cmdcode);
    EXPECT_EQ(104, SysSimuCmdproc2::proc_cmdcode);
    EXPECT_EQ(true, sysSimuSys.simuModule1->_simuThread1->CheckMsg());

	// ----- Post Test -----
}
