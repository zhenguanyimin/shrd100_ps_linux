#include "gtest/gtest.h"

#include "HalMgr.h"
#include "eap_os_adapter.h"
#include "SysBase.h"
#include "HalBase.h"
#include "StubFunc.h"

extern "C"
{
    extern Thread_Msg stubReqMsg;
}

class SimuHal1 : public HalBase
{
public:
    SimuHal1(HalMgr* halMgr) : HalBase(halMgr){}
    virtual void Init()
    {
        _Register(100, (Hal_Func)&SimuHal1::_HalFunc1);
        _Register(101, (Hal_Func)&SimuHal1::_HalFunc2);
    }
protected:
    int _HalFunc1(const Hal_Data& inData, Hal_Data& outData)
    {
        return 100;
    }
    int _HalFunc2(const Hal_Data& inData, Hal_Data& outData)
    {
        *outData.buf = 101;
        return 101;
    }
private:
};
HalBase *simuHal = nullptr;
class SimuHalMgr : public HalMgr
{
public:
    SimuHalMgr()
    {
    }
protected:
    void _InstallHals()
    {
        simuHal = new SimuHal1(this);
        AddHal(simuHal);
    }
private:
};

SimuHalMgr halMgr;

TEST_A(HalMgrTest, SendProcessSyncMsgTest, wuqingA23199)
{
	// ----- Pre Test -----
    Stub_ReInit(nullptr);
    halMgr.Init();

	// ----- Testing -----
    Hal_Data inData = {0};
    int out = 0;
    Hal_Data outData = {sizeof(out), (uint8_t*)&out};
    int ret = halMgr.OnSet(100, inData);
    EXPECT_EQ(100, ret);
    ret = halMgr.OnGet(101, inData, outData);
    EXPECT_EQ(101, ret);
    EXPECT_EQ(101, out);
    ret = halMgr.OnGet(102, inData, outData);
    EXPECT_EQ(EAP_FAIL, ret);
    ret = simuHal->Dispatch(102, inData, outData);
    EXPECT_EQ(EAP_FAIL, ret);

	// ----- Post Test -----
    Stub_Restore();
}

