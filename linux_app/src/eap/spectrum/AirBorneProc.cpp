#include "json_pb_converter.h"
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <math.h>
#include "AirBorneProc.h"
#include "AirBorneSnifferModule.h"
#include "HalMgr.h"
#include "SysBase.h"
#include "SpectrumModule.h"
#include "TracerAdcHal.h"
#include "TracerDataPath.h"
#include "File2Struct.h"

using namespace std;

#define MODULO( x ) (x % 10)
#define EQUAL( x , y )  fabs((x) - (y)) < 0.000001
#define PR(x, type) cout << EAP_BLANK_PREFIX(level + 1) << "_meansureInfo." #x ": " << (type)_meansureInfo.x;

const char StrCtrlStates[3][16] = {"关闭侦测", "打开侦测", "进入定向"};


AirBorneProc::AirBorneProc(SpectrumModule& module) : _module(module)
{
    char file_path[256];
    int ret;

    memset(&_ctrlState, 0, sizeof(ioController_t));
    _mCnt = 0;

    snprintf(file_path, 256, "%s/%s", ProductSwCfgGet().algcompensateattr().file_path().c_str(), ProductSwCfgGet().algcompensateattr().file_name().c_str());
    ret = file2struct(file_path, _bcData);
    if (ret == 0)
        flag_no_bcData = 0;
    else
    {
        printf("***获取补偿数据失败! ret=file2struct(%s)=%d***\n", file_path, ret);
        flag_no_bcData = 1;
    }

    if (access(FLAG_DEBUG_AIRBORNE_FILE, F_OK) == 0)
        flag_debug_airborne = 1;
    else
        flag_debug_airborne = 0;
    printf("%sAirBorne调试功能\n", flag_debug_airborne?"开启":"关闭");
}

void AirBorneProc::Proc(float curDetectFreq)
{
    HalMgr* halMgr = EapGetSys().GetHalMgr();
    Drv_AngleInfo angleInfo = {0};
    Hal_Data inData = {0, nullptr};
    Hal_Data outData = {sizeof(Drv_AngleInfo), (uint8_t*)&angleInfo};
    if (nullptr != halMgr && EAP_SUCCESS == halMgr->OnGet(EAP_DRVCODE_GET_ANGLE_INFO, inData, outData))
    {
    }
}

void AirBorneProc::Clear()
{
    _ctrlState.workMode = ARBN_WORK_STATE_IDLE;

    uint8_t freqMode = DEFINED_SWEEP_FREQ_MODE;
    SendAsynData(EAP_CMD_SET_FREQUENCY_MODE, &freqMode, 1);
}

int32_t AirBorneProc::SetCtrlState(void *data)
{
    std::vector<int> myVector = {2420, 2460, 5745, 5785, 5825};
    ioController_t *state= (ioController_t *)data;
    uint8_t freqMode;
    float freq;
    int i;

    // 大于等于9000的指令属于功能控制指令
    if (state->workMode >= 9000)
    {
        // 9001和9002命令需要在配置文件中开启采集时频图数据功能
        if (ProductDebugCfgGet().issupportspectrogram())
        {
            // 9001命令采集数据
            if (state->workMode == ARBN_COLLECT_DATA)
            {
                EAP_LOG_DEBUG("收到UAV发送的%d指令,开始采集数据.\n", ARBN_COLLECT_DATA);
                HalMgr* halMgr = EapGetSys().GetHalMgr();
                char suffixName[MAX_NAME_LEN];
                snprintf(suffixName, MAX_NAME_LEN, "%u-%u", state->dxNum, state->dxFreq);
                Hal_Data inData = {MAX_NAME_LEN, (uint8_t*)&suffixName};
                if (nullptr != halMgr) halMgr->OnSet(EAP_DRVCODE_COLLECT_DATA, inData);
                return EAP_SUCCESS;
            }
            // 9002命令清除所有采集数据
            if (state->workMode == ARBN_DELETE_DATA)
            {
                EAP_LOG_DEBUG("收到UAV发送的%d指令,删除所有采集数据.\n", ARBN_DELETE_DATA);
    #ifndef __UNITTEST__
                char cmd[256];
                sprintf(cmd, "rm -rf %s/*", ProductDebugCfgGet().spectrogramattr().file_path().c_str());
                system(cmd);
                sync();
    #endif
                return EAP_SUCCESS;
            }
        }
        // 9003和9004命令已在接收alink消息的地方处理开启/关闭WIFI
        EAP_LOG_DEBUG("不支持的控制命令%d.\n", state->workMode);
        return EAP_FAIL;
    }
    // 0~8999的指令目前只支持0,1,2结尾的指令
    else if (MODULO(state->workMode) > 2)
    {
        EAP_LOG_DEBUG("不支持的工作模式%d.\n", state->workMode);
        return EAP_FAIL;
    }

    if (_ctrlState.workMode != state->workMode)
        EAP_LOG_DEBUG("切换工作模式从%s到%s.\n", StrCtrlStates[MODULO(_ctrlState.workMode)], StrCtrlStates[MODULO(state->workMode)]);
    else
        EAP_LOG_DEBUG("工作模式%s没变不做切换.\n", StrCtrlStates[MODULO(_ctrlState.workMode)]);

    if (state->workMode == ARBN_WORK_STATE_IDLE)
    {
        Clear();
    }
    else if (MODULO(state->workMode) == ARBN_WORK_STATE_DECT)
    {
        if (_ctrlState.workMode != state->workMode)
        {
            EapGetSys().SetWorkMode(WORKMODE_AIRBORNE_OMNI);
            freqMode = DEFINED_SWEEP_FREQ_MODE;
            SendAsynData(EAP_CMD_SET_FREQUENCY_MODE, &freqMode, 1);
            SendAsynData(EAP_CMD_FREQUENCY, 0, 0);
            _ctrlState.workMode = state->workMode;
            EAP_LOG_DEBUG("收到指令%d,工作模式切换为%s.\n", state->workMode, StrCtrlStates[MODULO(_ctrlState.workMode)]);
        }
    }
    else if (MODULO(state->workMode) == ARBN_WORK_STATE_ORIENT)
    {
        if (_ctrlState.workMode == ARBN_WORK_STATE_IDLE)
        {
            EAP_LOG_DEBUG("不允许从关闭侦测跳转到进入定向,请先打开侦测!\n");
            return EAP_FAIL;
        }
        // 检查频率必须为:2420,2460,5745,5785,5825
        for (i = 0; i < myVector.size(); i++) {
            if (state->dxFreq == myVector[i]) {
                break;
            }
        }
        if (i >= myVector.size())
        {
            EAP_LOG_DEBUG("定频频率只能为2420,2460,5745,5785,5825!\n");
            return EAP_FAIL;
        }
        if (_ctrlState.workMode != state->workMode)
        {
            EapGetSys().SetWorkMode(WORKMODE_AIRBORNE_OMNI_ORIE);
            freqMode = DEFINED_FREQ_MODE;
            SendAsynData(EAP_CMD_SET_FREQUENCY_MODE, &freqMode, 1);
            _ctrlState.workMode = state->workMode;
            EAP_LOG_DEBUG("收到指令%d,工作模式切换为%s.\n", state->workMode, StrCtrlStates[MODULO(_ctrlState.workMode)]);
        }
        if (_ctrlState.dxNum != state->dxNum)
        {
            EAP_LOG_DEBUG("定向目标编号%d改为%d.\n", _ctrlState.dxNum, state->dxNum);
            _ctrlState.dxNum = state->dxNum;
        }
        if (_ctrlState.dxFreq != state->dxFreq)
        {
            EAP_LOG_DEBUG("定向侦测频率%d改为%d.\n", _ctrlState.dxFreq, state->dxFreq);
            freq = (float)state->dxFreq;
            SendAsynData(EAP_CMD_SET_FREQUENCY, (uint8_t *)&freq, sizeof(freq));
            _ctrlState.dxFreq = state->dxFreq;
        }
    }
    else
    {
        EAP_LOG_DEBUG("不支持的工作模式%d不做切换.\n", state->workMode);
        return EAP_FAIL;
    }
    return EAP_SUCCESS;
}

int32_t AirBorneProc::GetParam(void *data)
{
    AirBorne_param_t *param = (AirBorne_param_t *)data;
    _antStatus = param->cenFreq < 4000 ? ANTENNA_SWITCH_MODE4 : ANTENNA_SWITCH_MODE6;
    param->antStatus = _antStatus;
    param->mCnt = ++_mCnt;
    param->inC = _ctrlState;

    if (flag_debug_airborne)
    {
        if (_ctrlState.workMode != ARBN_WORK_STATE_ORIENT)
            printf("antStatus=%d, mCnt=%d, workMode=%d\n", param->antStatus, param->mCnt, param->inC.workMode);
        if (_ctrlState.workMode == ARBN_WORK_STATE_ORIENT)
            printf("antStatus=%d, mCnt=%d, workMode=%d, dxNum=%d, dxFreq=%d\n", param->antStatus, param->mCnt, param->inC.workMode, param->inC.dxNum, param->inC.dxFreq);
    }

    // 获取飞行姿态
    HalMgr* halMgr = EapGetSys().GetHalMgr();
    Hal_Data inData = {0, nullptr};
    Hal_Data outData = {sizeof(ioFlyStatus_t), (uint8_t*)&param->inF};
    if (nullptr != halMgr && EAP_SUCCESS == halMgr->OnGet(EAP_DRVCODE_GET_ANGLE_INFO_EX, inData, outData))
    {
        if (flag_debug_airborne)
        {
            EAP_LOG_DEBUG("***获取飞行姿态成功!***\n");
            uint8_t level = 1;
            cout << EAP_BLANK_PREFIX(level) << "_flyStatus: ";
            cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.yaw: " << (int16_t)param->inF.yaw;
            cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.pitch: " << (int16_t)param->inF.pitch;
            cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.roll: " << (int16_t)param->inF.roll;
            cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.wY: " << (int16_t)param->inF.wY;
            cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.wP: " << (int16_t)param->inF.wP;
            cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.wR: " << (int16_t)param->inF.wR;
            cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.longitude: " << (int32_t)param->inF.longitude;
            cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.latitude: " << (int32_t)param->inF.latitude;
            cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.altitude: " << (int32_t)param->inF.altitude;
            cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.vX: " << (int16_t)param->inF.vX;
            cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.vY: " << (int16_t)param->inF.vY;
            cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.vZ: " << (int16_t)param->inF.vZ;
        }
    }
    else if (_ctrlState.workMode > ARBN_WORK_STATE_IDLE)
    {
        // EAP_LOG_DEBUG("***获取飞行姿态失败!***\n");
        memset(&param->inF, 0, sizeof(ioFlyStatus_t));
    }

    // 获取补偿数据
    if (flag_no_bcData)
    {
        if (flag_no_bcData == 1)
        {
            // 装载模块的时候还没加载HAL层，只能等运行后再设LED状态
            flag_no_bcData++;
            EapGetSys().SetLedStatus(LED_STATUS_NORMAL, 0);
            EapGetSys().SetLedStatus(LED_STATUS_ABNORMAL, 1);
        }
        return EAP_FAIL;
    }
    else
        memcpy(&param->bcData, &_bcData, sizeof(CompensateData_t));

    return EAP_SUCCESS;
}

void AirBorneProc::ShowInfo(uint8_t level)
{
    cout << EAP_BLANK_PREFIX(level) << "_ctrlState.workMode: " << (uint16_t)_ctrlState.workMode;
    cout << EAP_BLANK_PREFIX(level) << "_ctrlState.dxNum: " << (uint16_t)_ctrlState.dxNum;
    cout << EAP_BLANK_PREFIX(level) << "_ctrlState.dxFreq: " << (uint16_t)_ctrlState.dxFreq;

#if 0
    cout << EAP_BLANK_PREFIX(level) << "_flyStatus: ";
    cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.yaw: " << (int16_t)_flyStatus.yaw;
    cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.pitch: " << (int16_t)_flyStatus.pitch;
    cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.roll: " << (int16_t)_flyStatus.roll;
    cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.wY: " << (int16_t)_flyStatus.wY;
    cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.wP: " << (int16_t)_flyStatus.wP;
    cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.wR: " << (int16_t)_flyStatus.wR;
    cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.longitude: " << (int32_t)_flyStatus.longitude;
    cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.latitude: " << (int32_t)_flyStatus.latitude;
    cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.altitude: " << (int32_t)_flyStatus.altitude;
    cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.vX: " << (int16_t)_flyStatus.vX;
    cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.vY: " << (int16_t)_flyStatus.vY;
    cout << EAP_BLANK_PREFIX(level + 1) << "_flyStatus.vZ: " << (int16_t)_flyStatus.vZ;
#endif

    cout << EAP_BLANK_PREFIX(level) << "_meansureInfo: ";
    cout << EAP_BLANK_PREFIX(level + 1) << "_meansureInfo.workStatus: " << (uint16_t)_meansureInfo.workStatus;
    cout << EAP_BLANK_PREFIX(level + 1) << "_meansureInfo.tarNum: " << (uint16_t)_meansureInfo.tarNum;

    PR(t1Num, uint16_t);
    PR(t1Freq, uint16_t);
    PR(t1Amp, uint16_t);
    PR(t1mFw, int16_t);
    PR(t1mFy, int16_t);
    PR(t1fFw, int16_t);
    PR(t1fFy, int16_t);
    PR(t2Num, uint16_t);
    PR(t2Freq, uint16_t);
    PR(t2Amp, uint16_t);
    PR(t2mFw, int16_t);
    PR(t2mFy, int16_t);
    PR(t2fFw, int16_t);
    PR(t2fFy, int16_t);
    PR(t3Num, uint16_t);
    PR(t3Freq, uint16_t);
    PR(t3Amp, uint16_t);
    PR(t3mFw, int16_t);
    PR(t3mFy, int16_t);
    PR(t3fFw, int16_t);
    PR(t3fFy, int16_t);
    PR(t4Num, uint16_t);
    PR(t4Freq, uint16_t);
    PR(t4Amp, uint16_t);
    PR(t4mFw, int16_t);
    PR(t4mFy, int16_t);
    PR(t4fFw, int16_t);
    PR(t4fFy, int16_t);
}