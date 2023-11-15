#include "ThreadTest.h"

#include "ThreadBase.h"
#include "ThreadQueue.h"
#include "eap_sys_cmds.h"
#include "SysBase.h"
#include "CmdprocBase.h"
#include "StubFunc.h"

extern "C"
{
}

SysBase sys(1);

class SimuThread1 : public ThreadBase
{
    DECLARE_THREAD(SimuThread1)
public:
protected:
private:
};

DEFINE_THREAD(SimuThread1, TEST1)

void ThreadTest::SetUpTestSuite()
{
    Stub_ReInit(&sys);
}

void ThreadTest::TearDownTestSuite()
{
    Stub_Restore();
}

TEST_F_A(ThreadTest, AddThread, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    ADD_THREAD(SimuThread1, 1);

    uint8_t actualThreadCnt = EapGetSys().GetThreadMgr().GetThreadCnt();
    uint8_t actualThreadIndex = EapGetSys().GetThreadMgr().GetThreadIndex(EAP_THREAD_ID_TEST1);
    EXPECT_EQ(1, (int)actualThreadCnt);
    EXPECT_EQ(1, (int)actualThreadIndex);
    ThreadBase *thread = EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_TEST1);
    EXPECT_TRUE(thread != nullptr);
    EXPECT_EQ(EAP_THREAD_ID_TEST1, (int)thread->GetId());
    EXPECT_EQ(EAP_STACK_SIZE_TEST1, (int)thread->GetStackSize());
    EXPECT_EQ(EAP_THREAD_PRIOR_TEST1, (int)thread->GetPrior());
    EXPECT_TRUE(strcmp("SimuThread1", thread->GetName().c_str()) == 0);

	// ----- Post Test -----
    EapGetSys().GetThreadMgr().ClearThreads();
}

class SimuThread2 : public ThreadBase
{
    DECLARE_THREAD(SimuThread2)
    DECLARE_TIMER1()
    DECLARE_TIMER3()
public:
protected:
    virtual void _StartTimer()
    {
        _StartTimer1();
        _StartTimer3();
    }
    virtual void _InitQueue(uint16_t& num)
    {
        num = 1;
    }
private:
};

DEFINE_THREAD(SimuThread2, TEST2)
DEFINE_TIMER1(SimuThread2, 1000, true)
DEFINE_TIMER3(SimuThread2, 2000, false)

TEST_F_A(ThreadTest, AddTimerThread, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    ADD_THREAD(SimuThread2, 1);
    EapGetSys().GetThreadMgr().CreateThreads();

    uint8_t actualThreadCnt = EapGetSys().GetThreadMgr().GetThreadCnt();
    uint8_t actualThreadIndex = EapGetSys().GetThreadMgr().GetThreadIndex(EAP_THREAD_ID_TEST2);
    EXPECT_EQ(1, actualThreadCnt);
    EXPECT_EQ(1, actualThreadIndex);
    ThreadBase *thread = EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_TEST2);
    EXPECT_TRUE(thread != nullptr);

    timer_entry1();
    ThreadQueue* queue = thread->GetQueue();
    EXPECT_TRUE(queue != nullptr);
    Thread_Msg msg;
    queue->ConsumerMsg((uint8_t*)&msg, sizeof(Thread_Msg));
    EXPECT_EQ((uint32_t)EAP_CMDTYPE_TIMER_1, msg.head.cmdType);
    EXPECT_EQ(0, (int)msg.head.length);

    timer_entry3();
    queue->ConsumerMsg((uint8_t*)&msg, sizeof(Thread_Msg));
    EXPECT_EQ((uint32_t)EAP_CMDTYPE_TIMER_3, msg.head.cmdType);
    EXPECT_EQ(0, (int)msg.head.length);

	// ----- Post Test -----
    EapGetSys().GetThreadMgr().ClearThreads();
}
