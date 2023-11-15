#include "RemoteIdTest.h"
#include "TracerMainSys.h"
#include "ThreadMgr.h"
#include "ThreadBase.h"
#include "StubFunc.h"
#include "RemoteIdModule.h"
#include "StubHal.h"
#include "simu_app.h"

extern TracerMainSysStub tracerSysStub;

static RemoteIdModule* module = nullptr;
void RemoteIdTest::SetUpTestSuite()
{
    Stub_ReInit(&tracerSysStub);
    module = dynamic_cast<RemoteIdModule*>(tracerSysStub.GetModuleStrategy(EAP_MODULE_ID_REMOTEID));
}

void RemoteIdTest::TearDownTestSuite()
{
    Stub_Restore();
}

TEST_F_A(RemoteIdTest, RemoteIdLiteCmdTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----

uint8_t data[1024] = { 0x80,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0xEB,0x51,0x6F,0x9A,0x01,0x66,0x66,0x51,0x6F,0x9A,0x01,0x88,0x88,0x80,0x50,
                            0x90,0x37,0xAB,0x46,0x00,0x00,0x00,0x00,0xFF,0xFF,0x21,0x04,
                            0x00,0x0C,0x64,0x65,0x66,0x61,0x75,0x6C,0x74,0x2D,0x73,0x73,0x69,0x64,
                            0xDD,0xB7,0xFA,0x0B,0xBC,0x0D,
                            0x00,0xF0,0x19,0x07,
                            0x02,0x13,0x4C,0x54,0x4F,0x39,0x32,0x32,0x32,0x38,0x35,0x34,0x39,0x37,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x02,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x12,0x12,0xB4,0x00,0x00,0x35,0x29,0x78,0x0D,0x71,0xAB,0xF2,0x43,0x48,0x09,0x48,0x09,0x6A,0x07,0x6C,0x26,0x54,0x38,0x05,0x00,
                            0x22,0x10,0x00,0x00,0x90,0x6E,0xD6,0x1A,0x00,0x32,0xAA,0x66,0x9D,0x76,0x60,0x34,0x27,0xD9,0x4A,0x80,0xF3,0x00,0x00,0x00,0x00,
                            0x32,0x00,0x75,0x6E,0x73,0x65,0x74,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x42,0x00,0x90,0x08,0x78,0x0D,0x00,0xBA,0xF2,0x43,0x01,0x00,0x00,0x6A,0x07,0x6A,0x07,0x00,0x48,0x09,0x90,0x6E,0xD6,0x1A,0x00,
                            0x52,0x00,0x75,0x6E,0x73,0x65,0x74,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                          };
    SendAsynData(EAP_CMD_REMOTEID, (uint8_t*)&data, 1024);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_REMOTEID)->Run();
    EXPECT_EQ(1, (int)module->GetRemoteIdNum());
	
    // ----- Post Test -----
    module->Clear();
}

TEST_F_A(RemoteIdTest, RemoteIdParrotCmdTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----

uint8_t data[1024] = { 0x80,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0xEB,0x51,0x6F,0x9A,0x01,0x66,0x66,0x51,0x6F,0x9A,0x01,0x88,0x88,0x80,0x50,
                            0x90,0x37,0xAB,0x46,0x00,0x00,0x00,0x00,0xFF,0xFF,0x21,0x04,
                            0x00,0x0C,0x64,0x65,0x66,0x61,0x75,0x6C,0x74,0x2D,0x73,0x73,0x69,0x64,
                            0xDD,0xB7,0xFA,0x0B,0xBC,0x0D,
                            0x00,0xF0,0x19,0x07,
                            0x02,0x13,0x4C,0x54,0x4F,0x39,0x32,0x32,0x32,0x38,0x35,0x34,0x39,0x37,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x02,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x12,0x12,0xB4,0x00,0x00,0x35,0x29,0x78,0x0D,0x71,0xAB,0xF2,0x43,0x48,0x09,0x48,0x09,0x6A,0x07,0x6C,0x26,0x54,0x38,0x05,0x00,
                            0x22,0x10,0x00,0x00,0x90,0x6E,0xD6,0x1A,0x00,0x32,0xAA,0x66,0x9D,0x76,0x60,0x34,0x27,0xD9,0x4A,0x80,0xF3,0x00,0x00,0x00,0x00,
                            0x32,0x00,0x75,0x6E,0x73,0x65,0x74,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x42,0x00,0x90,0x08,0x78,0x0D,0x00,0xBA,0xF2,0x43,0x01,0x00,0x00,0x6A,0x07,0x6A,0x07,0x00,0x48,0x09,0x90,0x6E,0xD6,0x1A,0x00,
                            0x52,0x00,0x75,0x6E,0x73,0x65,0x74,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                          };
    
    SendAsynData(EAP_CMD_REMOTEID, (uint8_t*)&data, 1024);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_REMOTEID)->Run();
    EXPECT_EQ(1, (int)module->GetRemoteIdNum());
	
    // ----- Post Test -----
    module->Clear();
}

TEST_F_A(RemoteIdTest, RemoteIdErrStartCmdTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    uint8_t data[1024] = {0x81, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0xFF,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0x71, 2, 0, 0, 0, 0, 0, 0,
                          0xDD, 0x53, 0, 0, 0, 0xD, 0, 0xF0, 0x19, 3,
                          0, 0x10, 1, 2, 3, 4, 5, 6, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0x10, 1, 2, 3, 4,
                          5, 0, 0, 0, 6, 0, 0, 0, 7, 0,
                          8, 0, 9, 0, 0, 0, 0, 0, 0, 0,
                          0x40, 0, 1, 0, 0, 0, 2, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0
                          };
    
    SendAsynData(EAP_CMD_REMOTEID, (uint8_t*)&data, 1024);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_REMOTEID)->Run();
    EXPECT_EQ(0, (int)module->GetRemoteIdNum());
	
    // ----- Post Test -----
}

#if 0
TEST_F_A(RemoteIdTest, RemoteIdErrTypeCmdTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    uint8_t data[1024] = {0x80, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0xFF,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0x72, 2, 0, 0, 0, 0, 0, 0,
                          0xDD, 0x53, 0, 0, 0, 0xD, 0, 0xF0, 0x19, 3,
                          0, 0x10, 1, 2, 3, 4, 5, 6, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0x10, 1, 2, 3, 4,
                          5, 0, 0, 0, 6, 0, 0, 0, 7, 0,
                          8, 0, 9, 0, 0, 0, 0, 0, 0, 0,
                          0x40, 0, 1, 0, 0, 0, 2, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0
                          };
    
    SendAsynData(EAP_CMD_REMOTEID, (uint8_t*)&data, 1024);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_REMOTEID)->Run();
    EXPECT_EQ(0, (int)module->GetRemoteIdNum());
	
    // ----- Post Test -----
}

TEST_F_A(RemoteIdTest, RemoteIdErrLenCmdTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    uint8_t data[1024] = {0x80, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0x4, 1,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0x64, 0, 0, 0, 0, 0, 0, 0,
                          0xDD, 0x53, 0, 0, 0, 0xD, 0, 0xF0, 0x19, 3,
                          0, 0x10, 1, 2, 3, 4, 5, 6, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0x10, 1, 2, 3, 4,
                          5, 0, 0, 0, 6, 0, 0, 0, 7, 0,
                          8, 0, 9, 0, 0, 0, 0, 0, 0, 0,
                          0x40, 0, 1, 0, 0, 0, 2, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0
                          };
    
    SendAsynData(EAP_CMD_REMOTEID, (uint8_t*)&data, 1024);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_REMOTEID)->Run();
    EXPECT_EQ(0, (int)module->GetRemoteIdNum());
	
    // ----- Post Test -----
    module->Clear();
}
#endif
TEST_F_A(RemoteIdTest, RemoteIdErrMsgCmdTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    uint8_t data[1024] = {0x80, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0xFF,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0x64, 0, 0, 0, 0, 0, 0, 0,
                          0xDE, 0x53, 0, 0, 0, 0xD, 0, 0xF0, 0x19, 3,
                          0, 0x10, 1, 2, 3, 4, 5, 6, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0x10, 1, 2, 3, 4,
                          5, 0, 0, 0, 6, 0, 0, 0, 7, 0,
                          8, 0, 9, 0, 0, 0, 0, 0, 0, 0,
                          0x40, 0, 1, 0, 0, 0, 2, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0
                          };
    
    SendAsynData(EAP_CMD_REMOTEID, (uint8_t*)&data, 1024);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_REMOTEID)->Run();
    EXPECT_EQ(0, (int)module->GetRemoteIdNum());
	
    // ----- Post Test -----
    module->Clear();
}

TEST_F_A(RemoteIdTest, RemoteIdErrMsgLenCmdTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    uint8_t data[1024] = {0x80, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0xFF,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0x64, 0, 0, 0, 0, 0, 0, 0,
                          0xDD, 0x53, 0, 0, 0, 0xD, 0, 0xF0, 0x19, 4,
                          0, 0x10, 1, 2, 3, 4, 5, 6, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0x10, 1, 2, 3, 4,
                          5, 0, 0, 0, 6, 0, 0, 0, 7, 0,
                          8, 0, 9, 0, 0, 0, 0, 0, 0, 0,
                          0x40, 0, 1, 0, 0, 0, 2, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0
                          };
    
    SendAsynData(EAP_CMD_REMOTEID, (uint8_t*)&data, 1024);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_REMOTEID)->Run();
    EXPECT_EQ(0, (int)module->GetRemoteIdNum());
	
    // ----- Post Test -----
    module->Clear();
}

TEST_F_A(RemoteIdTest, RemoteIdDataInValidCmdTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    uint8_t data[1024] = {0x80, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0xFF,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0x64, 0, 0, 0, 0, 0, 0, 0,
                          0xDD, 0x53, 0, 0, 0, 0xD, 0, 0xF0, 0x19, 3,
                          0x20, 0x10, 1, 2, 3, 4, 5, 6, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0x10, 1, 2, 3, 4,
                          5, 0, 0, 0, 6, 0, 0, 0, 7, 0,
                          8, 0, 9, 0, 0, 0, 0, 0, 0, 0,
                          0x40, 0, 1, 0, 0, 0, 2, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0
                          };
    
    SendAsynData(EAP_CMD_REMOTEID, (uint8_t*)&data, 1024);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_REMOTEID)->Run();
    EXPECT_EQ(0, (int)module->GetRemoteIdNum());
	
    // ----- Post Test -----
    module->Clear();
}

#if 0
TEST_F_A(RemoteIdTest, RptRemoteIdInfoTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    uint8_t data[1024] = {0x80, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0xFF,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0x64, 0, 0, 0, 0, 0, 0, 0,
                          0xDD, 0x53, 0, 0, 0, 0xD, 0, 0xF0, 0x19, 3,
                          0, 0x10, 1, 2, 3, 4, 5, 6, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0x10, 1, 2, 3, 4,
                          5, 0, 0, 0, 6, 0, 0, 0, 7, 0,
                          8, 0, 9, 0, 0, 0, 0, 0, 0, 0,
                          0x40, 0, 1, 0, 0, 0, 2, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0
                          };
    
    SendAsynData(EAP_CMD_REMOTEID, (uint8_t*)&data, 1024);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_REMOTEID)->Run();
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_REMOTEID)->SendTimerMsg2Self(EAP_CMDTYPE_TIMER_1);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_REMOTEID)->Run();
    EXPECT_EQ(1, (int)g_stubHalInDatasNum);
    EXPECT_EQ(EAP_DRVCODE_RPT_INFO_BY_ALINK, (int)g_stubHalInDatas[0].drvCode);
	EXPECT_TRUE(Stub_CheckRptInfoHeadByAlink(g_stubHalInDatas[0].buf, EAP_ALINK_RPT_REMOTEID, (26 + sizeof(Rpt_RemoteInfo))));
	EXPECT_TRUE(Stub_CheckRptTargetNum(g_stubHalInDatas[0].buf, 1));
    // ----- Post Test -----
    module->Clear();
}
#endif