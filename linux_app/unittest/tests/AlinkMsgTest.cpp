#include "AlinkMsgTest.h"
#include "TracerMainSys.h"
#include "ThreadMgr.h"
#include "ThreadBase.h"
#include "StubFunc.h"
#include "StubHal.h"
#include "simu_app.h"
#include "AlinkCmdproc.h"
#include "HeartBeatCmdproc.h"
#include "eap_os_adapter.h"
#include "HeartBeatModule.h"

extern TracerMainSysStub tracerSysStub;

void AlinkMsgTest::SetUpTestSuite()
{
    Stub_ReInit(&tracerSysStub);
}

void AlinkMsgTest::TearDownTestSuite()
{
    Stub_Restore();
}

void AlinkMsgTest::SetUp()
{
    ClearDatas();
}

void AlinkMsgTest::TearDown()
{
    ClearDatas();
}

/*
TEST_F_A(AlinkMsgTest, DupBroadRequestTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    alink_msg_t alinkReq = {0};
    alink_msg_t alinkResp = {0};
	AlinkDupBroadcastReq *data = (AlinkDupBroadcastReq *)alinkReq.buffer;
    data->protocol = EAP_ALINK_PROTOBUF;
    Alink_DeviceStatusInfo_Func((uint8_t*)&alinkReq, (uint8_t*)&alinkResp);
    EapGetSys().DupResponse();

    EXPECT_EQ(1, (int)g_stubHalInDatasNum);
    EXPECT_EQ(EAP_DRVCODE_RPT_INFO_BY_ALINK, (int)g_stubHalInDatas[0].drvCode);
	EXPECT_TRUE(Stub_CheckRptInfoHeadByAlink(g_stubHalInDatas[0].buf, EAP_ALINK_RPT_DEVICE_STATUS_INFO, sizeof(AlinkDupBroadcastResp)));
	EXPECT_TRUE(Stub_CheckDupBoradcastResponse(g_stubHalInDatas[0].buf, data->protocol, EapGetSys().GetDeviceType()));
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_SYS)->SendTimerMsg2Self(EAP_CMDTYPE_TIMER_2);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_SYS)->Run();
    EXPECT_EQ(2, (int)g_stubHalInDatasNum);
    EXPECT_EQ(EAP_DRVCODE_RPT_INFO_BY_ALINK, (int)g_stubHalInDatas[1].drvCode);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_SYS)->SendTimerMsg2Self(EAP_CMDTYPE_TIMER_2);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_SYS)->Run();
    EXPECT_EQ(3, (int)g_stubHalInDatasNum);
    EXPECT_EQ(EAP_DRVCODE_RPT_INFO_BY_ALINK, (int)g_stubHalInDatas[2].drvCode);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_SYS)->SendTimerMsg2Self(EAP_CMDTYPE_TIMER_2);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_SYS)->Run();
    EXPECT_EQ(3, (int)g_stubHalInDatasNum);
	
    // ----- Post Test -----
    EapGetSys().SetDupProtocol(0);
}
*/

TEST_F_A(AlinkMsgTest, GetDevInfoTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    alink_msg_t alink = {0};
	AlinkDevInfoResp *data = (AlinkDevInfoResp *)alink.buffer;
    int len = Alink_GetDevInfo_Func(nullptr, (uint8_t*)&alink);

    EXPECT_EQ(len, (int)sizeof(AlinkDevInfoResp));
    EXPECT_TRUE(memcmp(data->Company_name, EAP_EMBED_COMPANY_NAME, strlen(EAP_EMBED_COMPANY_NAME)) == 0);
    EXPECT_TRUE(memcmp(data->Device_name, EAP_EMBED_DEVICE_NAME, strlen(EAP_EMBED_DEVICE_NAME)) == 0);
    EXPECT_TRUE(memcmp(data->PS_version, EAP_EMBED_SOFTWARE_PS_VERSION_STR, strlen(EAP_EMBED_SOFTWARE_PS_VERSION_STR)) == 0);

    // ----- Post Test -----
}

TEST_F_A(AlinkMsgTest, Get_Log_ListTest, caozhengA23180)
{
    // ----- Pre Test -----

	// ----- Testing -----
    alink_msg_t alinkReq = {0};
    alink_msg_t alinkResp = {0};
    uint32_t index = 0x00 ;

    uint32_t ret = Alink_Get_Log_List_Func((uint8_t*)&alinkReq, (uint8_t*)&alinkResp);
    uint32_t Pkg_total_num = (uint32_t)alinkResp.buffer[index]
					+ ((uint32_t)alinkResp.buffer[index+1] << 8)
					+ ((uint32_t)alinkResp.buffer[index+2] << 16)
					+ ((uint32_t)alinkResp.buffer[index+3] << 24);
    index += 4;
    uint32_t Pkg_cur_num = (uint32_t)alinkResp.buffer[index]
					+ ((uint32_t)alinkResp.buffer[index+1] << 8)
					+ ((uint32_t)alinkResp.buffer[index+2] << 16)
					+ ((uint32_t)alinkResp.buffer[index+3] << 24);
    EXPECT_EQ(ret,true);
    EXPECT_EQ(Pkg_total_num,1);
    EXPECT_EQ(Pkg_cur_num,0);
    // ----- Post Test -----
}

TEST_F_A(AlinkMsgTest, Get_Log_DataTest, caozhengA23180)
{
    // ----- Pre Test -----

	// ----- Testing -----
    alink_msg_t alinkReq = {0};
    alink_msg_t alinkResp = {0};
    uint32_t* p_request = (uint32_t*)alinkReq.buffer;
    *p_request = 123;
    p_request++;
    AlinkLogReq* req = (AlinkLogReq*)p_request;
    req->Log_id = 0x10;
    req->Log_name_len = 30;
    memcpy(req->Log_name,"debug_20230505205959_0002.txt",sizeof(30));
    uint32_t ret = Alink_Get_Log_Data_Func((uint8_t*)&alinkReq, (uint8_t*)&alinkResp);
    AlnikLogResp * resp = (AlnikLogResp*)alinkResp.buffer;
    EXPECT_EQ(ret,true);
    EXPECT_EQ(req->Log_id,resp->Log_id);
    // ----- Post Test -----
}

TEST_F_A(AlinkMsgTest,DelTracerLogTest,caozhengA23180)
{
     // ----- Pre Test -----

	// ----- Testing -----
    alink_msg_t alinkReq = {0};
    alink_msg_t alinkResp = {0};
    uint32_t ret = Alink_Delete_Log_Func((uint8_t*)&alinkReq, (uint8_t*)&alinkResp);
    EXPECT_EQ(ret,true);
    EXPECT_EQ(alinkResp.buffer[0],true);
    // ----- Post Test -----
}

//TEST_F_A(AlinkMsgTest, C2HeartBeatMsgCheck, caozhengA23180)
//{
	// ----- Pre Test -----
//    HeartBeatModule* module = nullptr;
//    module = dynamic_cast<HeartBeatModule*>(tracerSysStub.GetModuleStrategy(EAP_MODULE_ID_HEARTBEAT));
	// ----- Testing -----
//    SendAsynData(EAP_CMD_UPDATE_C2_HEARTBEAT_TIME, 0, 0);
    //EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_HEARTBEAT)->Run();
    //EXPECT_EQ(true,dynamic_cast<HeartBeatCmdproc*>(module->GetCmdProcPtr())->_GetHeartBeatFlag());
	// ----- Post Test -----
//}
