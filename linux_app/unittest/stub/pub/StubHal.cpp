#include <map>
#include "StubHal.h"
#include "StubFunc.h"

void StubHal::Init()
{
    _Register(EAP_DRVCODE_RPT_INFO_BY_ALINK, (Hal_Func)&StubHal::_OnRptInfoByAlink);
    _Register(EAP_DRVCODE_GET_DRONE_FREQ, (Hal_Func)&StubHal::_OnGetDroneFreq);
    _Register(EAP_DRVCODE_GET_DRONE_ADC, (Hal_Func)&StubHal::_OnGetDroneAdc);
}

int StubHal::_OnGetDroneAdc(const Hal_Data& inData, Hal_Data& outData)
{
    if (g_stubOutputListNum > 0)
    {
        memcpy(outData.buf, &g_stubOutputList, sizeof(g_stubOutputList));
        outData.length = sizeof(g_stubOutputList);
    }
    else
    {
        outData.length = 0;
    }
    return EAP_SUCCESS;
}

int StubHal::_OnGetDroneFreq(const Hal_Data& inData, Hal_Data& outData)
{
    Drv_Drone_Freq* freq = (Drv_Drone_Freq*)(outData.buf);
    freq->sigFreq = g_stubDroneFreq.sigFreq;
    freq->freqOffset = g_stubDroneFreq.freqOffset;
    return EAP_SUCCESS;
}

int StubHal::_Dispatch(uint16_t drvCode, const Hal_Data& inData, Hal_Data& outData)
{
    if (g_stubHalInDatasNum >= 100) return EAP_FAIL;
    
    g_stubHalInDatas[g_stubHalInDatasNum].drvCode = drvCode;
    if (inData.length > 0)
    {
        g_stubHalInDatas[g_stubHalInDatasNum].buf = new uint8_t[inData.length];
        g_stubHalInDatas[g_stubHalInDatasNum].bufLen = inData.length;
        memcpy(g_stubHalInDatas[g_stubHalInDatasNum].buf, inData.buf, inData.length);
    }
    else
    {
        g_stubHalInDatas[g_stubHalInDatasNum].buf = nullptr;
        g_stubHalInDatas[g_stubHalInDatasNum].bufLen = 0;
    }
    ++g_stubHalInDatasNum;
    int ret = HalBase::_Dispatch(drvCode, inData, outData);
    if (outData.length > 0)
    {
        g_stubHalOutDatas[g_stubHalOutDatasNum].drvCode = drvCode;
        g_stubHalOutDatas[g_stubHalOutDatasNum].buf = new uint8_t[outData.length];
        g_stubHalOutDatas[g_stubHalOutDatasNum].bufLen = outData.length;
        memcpy(g_stubHalOutDatas[g_stubHalOutDatasNum].buf, outData.buf, outData.length);
        ++g_stubHalOutDatasNum;
    }
    return ret;
}

int StubHal::_OnRptInfoByAlink(const Hal_Data& inData, Hal_Data& outData)
{
    return EAP_SUCCESS;
}
