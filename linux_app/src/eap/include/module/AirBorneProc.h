#pragma once

#include "eap_pub.h"

extern "C"
{
#include "sfAirBorneSniffer.h"
}

enum
{
    ARBN_WORK_STATE_IDLE = 0,      // 0-关闭侦测
    ARBN_WORK_STATE_DECT = 1,      // 1-打开侦测
    ARBN_WORK_STATE_ORIENT = 2,    // 2-进入定向
    ARBN_TEST_STATE_DECT = 8001,   // 8001-打开侦测调试模式
    ARBN_TEST_STATE_ORIENT = 8002, // 8002-进入定向调试模式
    ARBN_COLLECT_DATA = 9001,      // 9001-采集1次数据,最多保存100份后覆盖
    ARBN_DELETE_DATA = 9002,       // 9002-删除所有采集数据
    ARBN_SET_WIFI_ENABLE  = 9003,  // 9003-使能WIFI/BT
    ARBN_SET_WIFI_DISABLE = 9004,  // 9004-禁止WIFI/BT
};

class SpectrumModule;

class AirBorneProc
{
public:
    AirBorneProc(SpectrumModule& module);
    int32_t SetCtrlState(void *data);
    int32_t GetParam(void *data);
    void Proc(float curDetectFreq);
    void Clear();
    void ShowInfo(uint8_t level);

protected:

private:
    int _antStatus;
    unsigned int _mCnt;
    ioController_t _ctrlState;
    CompensateData_t _bcData;
    ioMeasureInfo_t _meansureInfo;
    SpectrumModule& _module;
    int flag_debug_airborne;
    int flag_no_bcData;         // 没有补偿数据文件时应该走异常处理流程亮红灯或报告C2
};