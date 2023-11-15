#include "ProcessCommTest.h"

#include "ThreadBase.h"
#include "ThreadQueue.h"
#include "Module.h"
#include "ModuleReal.h"
#include "SysBase.h"
#include "eap_sys_cmds.h"
#include "ModuleProxy.h"
#include "RespMsgThread.h"
#include "CmdprocBase.h"
#include "StubFunc.h"

extern "C"
{
    extern Thread_Msg stubReqMsg;
}

class ProcessCommSimuCmdproc1 : public CmdprocBase
{
public:
    static int proc_cmdcode;

protected:
    virtual void _RegisterCmd()
    {
        Register(101, (Cmdproc_Func)&ProcessCommSimuCmdproc1::_SimuProcCmd1);
        Register(103, (Cmdproc_Func)&ProcessCommSimuCmdproc1::_SimuProcCmd3);
    }
    void _SimuProcCmd1(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
    {
        proc_cmdcode = 101;
        uint32_t* reqData = (uint32_t*)_GetSendDataBuf();
        *reqData = 0x1234;
        uint8_t respData = 0;
        uint32_t respDataLen = 1;
        SendSynMsg(_threadId, 102, (uint8_t*)_sendBuf, sizeof(uint32_t), &respData, &respDataLen);
        outMsg.dataLen = 0;
    }
    void _SimuProcCmd3(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
    {
        proc_cmdcode = 103;
        outMsg.dataLen = 10;
    }
};

int ProcessCommSimuCmdproc1::proc_cmdcode = 0;

class ProcessCommSimuThread1 : public ThreadBase
{
    DECLARE_THREAD(ProcessCommSimuThread1)
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
DEFINE_THREAD(ProcessCommSimuThread1, TEST1)

class ProcessCommSimuModule1 : public ModuleReal
{
public:
    ProcessCommSimuCmdproc1* _simuCmdproc1;
    ProcessCommSimuThread1* _simuThread1;
    ProcessCommSimuModule1(uint8_t coreId) : ModuleReal(coreId)
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
        _simuCmdproc1 = new ProcessCommSimuCmdproc1();
        return _simuCmdproc1;
    }
    ThreadBase* _CreateThread() override
    {
        _simuThread1 = (ProcessCommSimuThread1*)ADD_THREAD(ProcessCommSimuThread1, 1);
        return _simuThread1;
    }
};


class ProcessCommSimuModule2 : public ModuleProxy
{
public:
    ProcessCommSimuModule2(uint16_t threadId) : ModuleProxy(threadId){}
    virtual void Init()
    {
        CmdprocBase::ProxyRegister(102, _threadId);
    }
    uint8_t GetModuleId() override
    {
        return EAP_MODULE_ID_TEST2;
    }
};

extern SysBase* g_sys;

class SimuSys5 : public SysBase
{
public:
    SimuSys5(uint8_t deviceId, int16_t processId) : SysBase(deviceId, processId)
    {
        simuModule1 = new ProcessCommSimuModule1(1);
        _InstallModule(new Module(simuModule1));
        simuModule2 = new ProcessCommSimuModule2(6);
        _InstallModule(new Module(simuModule2));
    }
    void Init()
    {
        _respMsgThread = ADD_THREAD(RespMsgThread, 1);
        SysBase::Init();
    }
    ProcessCommSimuModule1* simuModule1;
    ProcessCommSimuModule2* simuModule2;
    ThreadBase* _respMsgThread;
};

SimuSys5 processCommSimuSys(1, 1);

void ProcessCommTest::SetUpTestSuite()
{
    Stub_ReInit(&processCommSimuSys);
}

void ProcessCommTest::TearDownTestSuite()
{
    Stub_Restore();
}

TEST_F_A(ProcessCommTest, SendProcessSyncMsgTest, wuqingA23199)
{
	// ----- Pre Test -----
    ProcessCommSimuCmdproc1::proc_cmdcode = 0;
    eap_set_processid_for_remote(6, 2);
    eap_set_queue_handle_for_remote(6, 2);

	// ----- Testing -----
    SendAsynCmdCode(101);
    processCommSimuSys.simuModule1->_simuThread1->Run();

    EXPECT_EQ(101, ProcessCommSimuCmdproc1::proc_cmdcode);
    EXPECT_EQ(EAP_GET_LOC(2, 6), stubReqMsg.head.to);

    Thread_Msg receiveMsg;
    eap_queue_receive(0, &receiveMsg, sizeof(receiveMsg), 0);
    EXPECT_EQ(EAP_GET_LOC(2, 6), receiveMsg.head.to);
    EXPECT_EQ(EAP_GET_LOC(1, EAP_THREAD_ID_TEST1), receiveMsg.head.from);
    EXPECT_EQ(4, (int)receiveMsg.head.length);

	// ----- Post Test -----
}

TEST_F_A(ProcessCommTest, SendRespMsgTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    Thread_Msg msg;
    msg.head.from = EAP_GET_LOC(1, EAP_THREAD_ID_TEST1);
    msg.head.to = EAP_GET_LOC(1, EAP_THREAD_ID_TEST1);
    msg.head.cmdType = EAP_CMDTYPE_MODULE;
    msg.head.reqType = EAP_CMD_RESPONSE;
    msg.head.length = 0;
    processCommSimuSys.GetThreadMgr().SendRespMsg(msg);

    memcpy(&stubReqMsg, &msg, sizeof(Thread_Msg));
    stubReqMsg.head.to = msg.head.from;
    stubReqMsg.head.from = msg.head.to;
    stubReqMsg.head.reqType = EAP_CMD_REQUEST;

    EXPECT_EQ(true, processCommSimuSys.simuModule1->_simuThread1->CheckMsg());

    msg.head.to = EAP_GET_LOC(2, 6);
    eap_set_resp_queuehandle_for_remote(2, 2);
    processCommSimuSys.GetThreadMgr().SendRespMsg(msg);
    Thread_Msg receiveMsg;
    eap_queue_receive(0, &receiveMsg, sizeof(receiveMsg), 0);
    EXPECT_EQ(EAP_GET_LOC(2, 6), receiveMsg.head.to);
    EXPECT_EQ(EAP_GET_LOC(1, EAP_THREAD_ID_TEST1), receiveMsg.head.from);
    EXPECT_EQ(0, (int)receiveMsg.head.length);

	// ----- Post Test -----
}

TEST_F_A(ProcessCommTest, RecvRespMsgTest, wuqingA23199)
{
	// ----- Pre Test -----
    Thread_Msg msg;
    msg.head.from = EAP_GET_LOC(2, 6);
    msg.head.to = EAP_GET_LOC(1, EAP_THREAD_ID_TEST1);
    msg.head.cmdType = EAP_CMDTYPE_MODULE;
    msg.head.reqType = EAP_CMD_RESPONSE;
    msg.head.length = 12;
    eap_queue_send(0, &msg, sizeof(msg.head) + 12, 0,0);

	// ----- Testing -----
    memcpy(&stubReqMsg, &msg, sizeof(Thread_Msg));
    stubReqMsg.head.to = msg.head.from;
    stubReqMsg.head.from = msg.head.to;
    stubReqMsg.head.reqType = EAP_CMD_REQUEST;
    EXPECT_EQ(true, processCommSimuSys._respMsgThread->Run());

    memcpy(&stubReqMsg, &msg, sizeof(Thread_Msg));
    stubReqMsg.head.to = msg.head.from;
    stubReqMsg.head.from = msg.head.to;
    stubReqMsg.head.reqType = EAP_CMD_REQUEST;
    EXPECT_EQ(true, processCommSimuSys.simuModule1->_simuThread1->CheckMsg());
	// ----- Post Test -----
}
