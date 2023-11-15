#include "DroneIdTest.h"
#include "TracerMainSys.h"
#include "ThreadMgr.h"
#include "ThreadBase.h"
#include "StubFunc.h"
#include "DroneIdModule.h"
#include "StubHal.h"
#include "simu_app.h"
#include "MavAlink.pb.h"

extern TracerMainSysStub tracerSysStub;

static DroneIdModule* module = nullptr;
void DroneIdTest::SetUpTestSuite()
{
    Stub_ReInit(&tracerSysStub);
    module = dynamic_cast<DroneIdModule*>(tracerSysStub.GetModuleStrategy(EAP_MODULE_ID_DRONEID));
}

void DroneIdTest::TearDownTestSuite()
{
    Stub_Restore();
}

void DroneIdTest::SetUp()
{
    ClearDatas();
}

void DroneIdTest::TearDown()
{
    ClearDatas();
}

TEST_F_A(DroneIdTest, DroneIdCmdTest, wuqingA23199)
{
	// ----- Pre Test -----
    Stub_SetDroneFreq(1, 2);

	// ----- Testing -----
    uint8_t type = eDATA_PATH_DDR_BURST;
    SendAsynData(EAP_CMD_DRONEID, &type, 1);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_DRONEID)->Run();
    EXPECT_EQ(2, (int)g_stubHalInDatasNum);
    EXPECT_EQ(EAP_DRVCODE_GET_DRONE_FREQ, (int)g_stubHalInDatas[0].drvCode);
	EXPECT_EQ(EapGetSys().GetWorkMode(), g_stubHalInDatas[0].buf[0]);
	EXPECT_EQ(1, (int)g_stubHalInDatas[0].bufLen);
    EXPECT_EQ(1, (int)g_stubHalOutDatasNum);
    EXPECT_EQ(EAP_DRVCODE_GET_DRONE_FREQ, (int)g_stubHalOutDatas[0].drvCode);
	EXPECT_EQ(sizeof(Drv_Drone_Freq), g_stubHalOutDatas[0].bufLen);
    Drv_Drone_Freq* freq = (Drv_Drone_Freq*)g_stubHalOutDatas[0].buf;
	EXPECT_EQ(1, (int)freq->sigFreq);
	EXPECT_EQ(2, (int)freq->freqOffset);
	
    // ----- Post Test -----
    module->Clear();
}

TEST_F_A(DroneIdTest, RptDroneIdCmdTest, wuqingA23199)
{
	// ----- Pre Test -----
    Rpt_DroneInfo output;
    output.productType = 1;
    strcpy((char*)output.droneName, "1");
    strcpy((char*)output.serialNum, "1");
    output.droneLongitude     = 1;
    output.droneLatitude      = 1;
    output.droneHeight        = 1;
    output.droneYawAngle      = 1;
    output.droneSpeed         = 1;
    output.droneVerticalSpeed = 1;
    output.pilotLongitude     = 1;
    output.pilotLatitude      = 1;
    output.uFreq              = 1;
    output.dangerLevels       = 1;
    Stub_SetOutputList(&output);

	// ----- Testing -----
    uint8_t type = eDATA_PATH_DDR_BURST;
    SendAsynData(EAP_CMD_DRONEID, &type, 1);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_DRONEID)->Run();
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_DRONEID)->SendTimerMsg2Self(EAP_CMDTYPE_TIMER_2);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_DRONEID)->Run();
    EXPECT_EQ(3, (int)g_stubHalInDatasNum);
    EXPECT_EQ(EAP_DRVCODE_GET_DRONE_FREQ, (int)g_stubHalOutDatas[0].drvCode);
    EXPECT_EQ(EAP_DRVCODE_GET_DRONE_ADC, (int)g_stubHalInDatas[1].drvCode);
    EXPECT_EQ(EAP_DRVCODE_RPT_INFO_BY_ALINK, (int)g_stubHalInDatas[2].drvCode);
	EXPECT_TRUE(Stub_CheckRptInfoHeadByAlink(g_stubHalInDatas[2].buf, EAP_ALINK_RPT_DRONEID, (26 + sizeof(Rpt_DroneInfo))));
	EXPECT_TRUE(Stub_CheckRptTargetNum(g_stubHalInDatas[2].buf, 1));
	
    // ----- Post Test -----
    module->Clear();
}

TEST_F_A(DroneIdTest, RptDroneIdPbCmdTest, wuqingA23199)
{
	// ----- Pre Test -----
    Rpt_DroneInfo output;
    output.productType = 1;
    strcpy((char*)output.droneName, "1");
    strcpy((char*)output.serialNum, "1");
    output.droneLongitude     = 1;
    output.droneLatitude      = 1;
    output.droneHeight        = 1;
    output.droneYawAngle      = 1;
    output.droneSpeed         = 1;
    output.droneVerticalSpeed = 1;
    output.pilotLongitude     = 1;
    output.pilotLatitude      = 1;
    output.uFreq              = 1;
    output.dangerLevels       = 1;
    Stub_SetOutputList(&output);
    EapGetSys().SetDupProtocol(EAP_ALINK_PROTOBUF);

	// ----- Testing -----
    uint8_t type = eDATA_PATH_DDR_BURST;
    SendAsynData(EAP_CMD_DRONEID, &type, 1);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_DRONEID)->Run();
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_DRONEID)->SendTimerMsg2Self(EAP_CMDTYPE_TIMER_2);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_DRONEID)->Run();
    EXPECT_EQ(3, (int)g_stubHalInDatasNum);
    EXPECT_EQ(EAP_DRVCODE_GET_DRONE_FREQ, (int)g_stubHalOutDatas[0].drvCode);
    EXPECT_EQ(EAP_DRVCODE_GET_DRONE_ADC, (int)g_stubHalInDatas[1].drvCode);
    EXPECT_EQ(EAP_DRVCODE_RPT_INFO_BY_ALINK, (int)g_stubHalInDatas[2].drvCode);
    ReportDroneIdPB pb;
    pb.ParseFromArray(Stub_GetAlinkBuf(g_stubHalInDatas[2].buf), Stub_GetAlinkBufLen(g_stubHalInDatas[2].bufLen));
    int16_t dataLen = pb.ByteSizeLong();
	EXPECT_TRUE(Stub_CheckRptInfoHeadByAlink(g_stubHalInDatas[2].buf, EAP_ALINK_RPT_DRONEID, dataLen));
    EXPECT_EQ(1, pb.udronenum());
	
    // ----- Post Test -----
    module->Clear();
    EapGetSys().SetDupProtocol(0);
}

TEST_F_A(DroneIdTest, Cli_AlgDebugPrintModeTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    char data1[100] = "AlgDebugPrintMode 100";
    uint32_t dataLen = strlen(data1);
    SendAsynData(EAP_CMD_DRONEID_SET_ALG_PRINT, (uint8_t*)&data1, dataLen);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->Run();
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_DRONEID)->Run();
    Thread_Msg respMsg = EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->GetRespMsg();
    EXPECT_EQ(1, (int)respMsg.head.length);
    EXPECT_EQ(EAP_FAIL, *((int8_t*)&respMsg.buf[0]));
    
    char data2[100] = "AlgDebugPrintMode 1";
    dataLen = strlen(data2);
    SendAsynData(EAP_CMD_DRONEID_SET_ALG_PRINT, (uint8_t*)&data2, dataLen);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->Run();
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_DRONEID)->Run();
    respMsg = EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->GetRespMsg();
    EXPECT_EQ(1, (int)respMsg.head.length);
    EXPECT_EQ(EAP_SUCCESS, *((int8_t*)&respMsg.buf[0]));

	// ----- Post Test -----
}

TEST_F_A(DroneIdTest, Cli_DroneIDRemoveThreSetTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    char data1[100] = "DroneIDRemoveThreSet -1";
    uint32_t dataLen = strlen(data1);
    SendAsynData(EAP_CMD_DRONEID_SET_REMOVE_THRE, (uint8_t*)&data1, dataLen);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->Run();
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_DRONEID)->Run();
    Thread_Msg respMsg = EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->GetRespMsg();
    EXPECT_EQ(1, (int)respMsg.head.length);
    EXPECT_EQ(EAP_FAIL, *((int8_t*)&respMsg.buf[0]));
    
    char data2[100] = "DroneIDRemoveThreSet 1";
    dataLen = strlen(data2);
    SendAsynData(EAP_CMD_DRONEID_SET_REMOVE_THRE, (uint8_t*)&data2, dataLen);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->Run();
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_DRONEID)->Run();
    respMsg = EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->GetRespMsg();
    EXPECT_EQ(1, (int)respMsg.head.length);
    EXPECT_EQ(EAP_SUCCESS, *((int8_t*)&respMsg.buf[0]));

	// ----- Post Test -----
}

TEST_F_A(DroneIdTest, Cli_DroneIDRemoveThreLostDurSetTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    char data1[100] = "DroneIDRemoveThreLostDurSet -1";
    uint32_t dataLen = strlen(data1);
    SendAsynData(EAP_CMD_DRONEID_SET_REMOVE_THRE_LOST_DUR, (uint8_t*)&data1, dataLen);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->Run();
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_DRONEID)->Run();
    Thread_Msg respMsg = EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->GetRespMsg();
    EXPECT_EQ(1, (int)respMsg.head.length);
    EXPECT_EQ(EAP_FAIL, *((int8_t*)&respMsg.buf[0]));
    
    char data2[100] = "DroneIDRemoveThreLostDurSet 1";
    dataLen = strlen(data2);
    SendAsynData(EAP_CMD_DRONEID_SET_REMOVE_THRE_LOST_DUR, (uint8_t*)&data2, dataLen);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->Run();
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_DRONEID)->Run();
    respMsg = EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->GetRespMsg();
    EXPECT_EQ(1, (int)respMsg.head.length);
    EXPECT_EQ(EAP_SUCCESS, *((int8_t*)&respMsg.buf[0]));

	// ----- Post Test -----
}

TEST_F_A(DroneIdTest, Cli_DroneIDOutputSnModeSetTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    char data1[100] = "DroneIDOutputSnModeSet 100";
    uint32_t dataLen = strlen(data1);
    SendAsynData(EAP_CMD_DRONEID_SET_OUTPUT_SN_MODE, (uint8_t*)&data1, dataLen);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->Run();
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_DRONEID)->Run();
    Thread_Msg respMsg = EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->GetRespMsg();
    EXPECT_EQ(1, (int)respMsg.head.length);
    EXPECT_EQ(EAP_FAIL, *((int8_t*)&respMsg.buf[0]));
    
    char data2[100] = "DroneIDOutputSnModeSet 1";
    dataLen = strlen(data2);
    SendAsynData(EAP_CMD_DRONEID_SET_OUTPUT_SN_MODE, (uint8_t*)&data2, dataLen);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->Run();
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_DRONEID)->Run();
    respMsg = EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->GetRespMsg();
    EXPECT_EQ(1, (int)respMsg.head.length);
    EXPECT_EQ(EAP_SUCCESS, *((int8_t*)&respMsg.buf[0]));

	// ----- Post Test -----
}

TEST_F_A(DroneIdTest, Cli_DroneIDTurboDecIterNumSetTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    char data1[100] = "DroneIDTurboDecIterNumSet 10";
    uint32_t dataLen = strlen(data1);
    SendAsynData(EAP_CMD_DRONEID_SET_TURBO_DEC_ITER_NUM, (uint8_t*)&data1, dataLen);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->Run();
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_DRONEID)->Run();
    Thread_Msg respMsg = EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->GetRespMsg();
    EXPECT_EQ(1, (int)respMsg.head.length);
    EXPECT_EQ(EAP_FAIL, *((int8_t*)&respMsg.buf[0]));
    
    char data2[100] = "DroneIDTurboDecIterNumSet 1";
    dataLen = strlen(data2);
    SendAsynData(EAP_CMD_DRONEID_SET_TURBO_DEC_ITER_NUM, (uint8_t*)&data2, dataLen);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->Run();
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_DRONEID)->Run();
    respMsg = EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_CLI)->GetRespMsg();
    EXPECT_EQ(1, (int)respMsg.head.length);
    EXPECT_EQ(EAP_SUCCESS, *((int8_t*)&respMsg.buf[0]));

	// ----- Post Test -----
}

TEST_F_A(DroneIdTest, ThreadRunHealthOkTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    OpenThreadRunDetect();
    
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_HEALTH_DETECT)->Run();
    uint8_t health = EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_SYS)->GetRunHealthState();
    EXPECT_EQ(THREAD_RUN_TESTING, (int)health);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_SYS)->Run();
    health = EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_SYS)->GetRunHealthState();
    EXPECT_EQ(THREAD_RUN_WELL, (int)health);

	// ----- Post Test -----
}

TEST_F_A(DroneIdTest, ThreadRunHealthFailTest, wuqingA23199)
{
	// ----- Pre Test -----

	// ----- Testing -----
    OpenThreadRunDetect();
    
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_HEALTH_DETECT)->Run();
    uint8_t health = EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_SYS)->GetRunHealthState();
    EXPECT_EQ(THREAD_RUN_TESTING, (int)health);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_HEALTH_DETECT)->SendTimerMsg2Self(EAP_CMDTYPE_TIMER_1);
    EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_HEALTH_DETECT)->Run();
    health = EapGetSys().GetThreadMgr().GetThread(EAP_THREAD_ID_SYS)->GetRunHealthState();
    EXPECT_EQ(THREAD_RUN_BAD, (int)health);

	// ----- Post Test -----
}

TEST_F_A(DroneIdTest, TelnetCmdTest, wuqingA23199)
{
	// ----- Testing -----
    //ShowOneModuleInfoUnSafe(EAP_MODULE_ID_SPECTRUM);
    //ShowOneCmdStat(0);
    //ShowAllThreadsMsgStats();
    //ShowOneThreadMsgStats(11);
    //ShowAllThreadsInfo();
    //ShowCmdprocRegisterInfo();
    //ShowAllCmdStat();
    //ShowHalRegisterInfo();
    //eap_help();
    //eap_do(1, 1, 1);
    //ShowAlinkCmdRegisterInfo();
    //ShowCliRegisteredCmds();
}