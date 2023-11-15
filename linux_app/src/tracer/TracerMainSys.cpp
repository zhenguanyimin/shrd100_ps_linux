#include "json_pb_converter.h"
#include "TracerMainSys.h"
#include "RemoteIdModule.h"
#include "Module.h"
#include "CliThread.h"
#include "RespMsgThread.h"
#include "AlinkModule.h"
#include "HeartBeatModule.h"
#include "DroneIdModule.h"
#include "TracerMainHalMgr.h"
#include "FrequencyModule.h"
#include "SysThread.h"
#include "SysCmdproc.h"
#include "DroneSnifferModule.h"
#include "AirBorneSnifferModule.h"
#include "BlueToothModule.h"
#include "BuzzerWarning.h"
#include "LedWarning.h"
#include "MotorWarning.h"
#include "SpectrumModule.h"
#include "HealthDetectThread.h"
#include "HealthDetectCmdproc.h"
#include "TimerThread.h"
#include "hal/remoteid_wifi/RemoteIdWifiThread.h"
#include "TracerDataPath.h"
#include "VehicleMountedModule.h"
extern "C"
{
#include "alg_init.h"
}

// Register callback for alg fft dma
int Pl_Fft_Dma_Calc(float *g_x_sp, float *g_y_sp)
{
    int ret = EAP_SUCCESS;
    ret = EapGetSys().PlGenDmaCalc(EAP_DRVCODE_DMA_FFT_DATA, (uint8_t *)g_x_sp, FFT_DMA_PACKET_LEN, (uint8_t *)g_y_sp, FFT_DMA_PACKET_LEN);
    return ret;
}

int TracerMainSys::PlGenDmaCalc(int drvCode, uint8_t *param_x, uint32_t leng_x, uint8_t *param_y, uint32_t leng_y)
{
    HalMgr* halMgr = EapGetSys().GetHalMgr();
    int ret = EAP_SUCCESS;

    Hal_Data inData = {leng_x, param_x};
    Hal_Data outData = {leng_y, param_y};
    if (nullptr != halMgr)
    {
        ret = halMgr->OnGet(drvCode, inData, outData);
    }

    return ret;
}

void TracerMainSys::_RegisterAlgCb()
{
    register_alg_dma_fft_cb(Pl_Fft_Dma_Calc);
}

void TracerMainSys::_CreateDebugLogDir()
{
    char cmd[128];

    // 创建log相关目录
    if (ProductDebugCfgGet().issupportspectrogram())
    {
        sprintf(cmd, "mkdir -p %s", ProductDebugCfgGet().spectrogramattr().file_path().c_str());
        system(cmd);
    }
    if (ProductDebugCfgGet().issupportalgcache())
    {
        sprintf(cmd, "mkdir -p %s", ProductDebugCfgGet().algcacheattr().file_path().c_str());
        system(cmd);
    }
    if (ProductDebugCfgGet().issupportalgexcept())
    {
        sprintf(cmd, "mkdir -p %s", ProductDebugCfgGet().algexceptattr().file_path().c_str());
        system(cmd);
    }
}

// Init的最后执行一些延迟任务
void TracerMainSys::_SetFrequencyParam()
{
    // 获取切频参数
    uint8_t workMode = GetWorkMode();
    SendAsynData(EAP_CMD_CHANGE_FREQUENCY_PARAM, &workMode, 1);

    // 设置切频模式(缺省按自定义切换,将来根据配置文件切换)
    uint8_t freqMode;
    if (ProductSwCfgGet().issupportairborne())
        freqMode = DEFINED_FREQ_MODE;
    else
        freqMode = DEFINED_SWEEP_FREQ_MODE;
    SendAsynData(EAP_CMD_SET_FREQUENCY_MODE, &freqMode, 1);

    // 频谱侦测开启第一次切频
    if ((workMode == WORKMODE_SPECTRUM_OMNI) || ProductSwCfgGet().issupportairborne())
    {
        SendAsynData(EAP_CMD_FREQUENCY, 0, 0);
        printf("频谱侦测开始工作...\n");

        // 开启DMA传输
        TracerMainHalMgr *halMgr = dynamic_cast<TracerMainHalMgr *>(EapGetSys().GetHalMgr());
        uint8_t data = 1;	// 0 - stop dma, 1 - start dma
        Hal_Data inData = {sizeof(uint8_t), (uint8_t*)&data};
        halMgr->OnSet(EAP_DRVCODE_DMA_DET_DATA, inData);
    }
}

TracerMainSys::TracerMainSys(uint8_t deviceId, int16_t processId) : SysBase(deviceId, processId,PRODUCT_CFG_STD_PATH,PRODUCT_CFG_DEBUG_PATH)
{
#ifdef __UNITTEST__
    _InstallModule(new Module(new FrequencyModule(0)));
    _InstallModule(new Module(new DroneIdModule(2)));
    _InstallModule(new Module(new RemoteIdModule(3)));
    _InstallModule(new Module(new AlinkModule(0)));
    _InstallModule(new Module(new SpectrumModule(0)));
    _InstallModule(new Module(new DroneSnifferModule(1)));
    // _InstallModule(new Module(new AirBorneSnifferModule(1)));
    //_InstallModule(new Module(new BlueToothModule(1)));
    //_InstallModule(new Module(new VehicleMountedModule(1)));
#else
    if (ProductSwCfgGet().issupportspectrum() || ProductSwCfgGet().issupportairborne())
    {
        _CreateDebugLogDir();
    }

    _InstallModule(new Module(new FrequencyModule(0)));
    if (ProductSwCfgGet().issupportdroneid())
    {
        _InstallModule(new Module(new DroneIdModule(2)));
    }
    if (ProductSwCfgGet().issupportremoteid())
    {
        _InstallModule(new Module(new RemoteIdModule(3)));
    }
    _InstallModule(new Module(new AlinkModule(0)));
    // 机载Tracer共用频谱模块
    if (ProductSwCfgGet().issupportspectrum() || ProductSwCfgGet().issupportairborne())
    {
        _InstallModule(new Module(new SpectrumModule(2)));
    }
    if (ProductSwCfgGet().issupportspectrumorient())
    {
        _InstallModule(new Module(new DroneSnifferModule(3)));
    }
    else if (ProductSwCfgGet().issupportairborneorient())
    {
        _InstallModule(new Module(new AirBorneSnifferModule(3)));
    }

    _InstallModule(new Module(new BlueToothModule(1)));
    
    if(DEV_TRACER_SEH100 == ProductSwCfgGet().devicetype()){
        _InstallModule(new Module(new VehicleMountedModule(1)));
    }
#endif
	

    HeartBeatModule* heartBeatModule = new HeartBeatModule(0);
    _InstallModule(new Module(heartBeatModule));

    _InstallHalMgr(new TracerMainHalMgr());

    heartBeatModule->InstallWarningObj(BuzzerWarning::Instance());
    heartBeatModule->InstallWarningObj(LedWarning::Instance());
    heartBeatModule->InstallWarningObj(MotorWarning::Instance());
}

void TracerMainSys::Init()
{
    ADD_THREAD_WITH_CMDPROC(SysThread, 0, SysCmdproc);
    ADD_THREAD_WITH_CMDPROC(HealthDetectThread, 0, HealthDetectCmdproc);

    ADD_THREAD(CliThread, 0);
    ADD_THREAD(RespMsgThread, 0);
    ADD_THREAD(TimerThread, 0);

#ifdef __UNITTEST__

#else
    if (ProductSwCfgGet().issupportremoteid())
    {
        // 没有加载RN440驱动时不启动RemoteID抓包，否则会段错误
        if (access(IFNAME_REMOTEID, F_OK) == 0)
        {
            ADD_THREAD(RemoteIdWifiThread, 3); // 注册wifi抓包线程
        }
        else
        {
            EAP_LOG_ERROR("***There is no interface(wlp1s0) of RN440 for RemoteID, please check whether enabled the gpio RN440_3V3_EN in UBOOT!***\n");
        }
    }
#endif

    SysBase::Init();

    SetSysStatus(SYS_DETECTION_TARGET_STATUS);
    _SetFrequencyParam();
}

