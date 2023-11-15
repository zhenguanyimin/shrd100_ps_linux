#include "DroneSnifferCmdproc.h"
#include "DroneSnifferModule.h"

extern "C"
{
#include "sfDroneSniffer.h"
}

DroneSnifferCmdproc::DroneSnifferCmdproc(uint32_t droneNum)
{
    _droneNum = droneNum;
    if (_droneNum > 0)
    {
        _droneInfo = new droneResult_t[_droneNum];
    }
    else
    {
        _droneInfo = nullptr;
    }
}

DroneSnifferCmdproc::~DroneSnifferCmdproc()
{
    EAP_DELETE_ARRAY(_droneInfo);
}

int32_t DroneSnifferCmdproc::_GetDroneMaxNum()
{
    if (_sendBufLen > sizeof(Thread_Msg_Head) + 2)
    {
        return (_sendBufLen - sizeof(Thread_Msg_Head) - 2) / sizeof(SpectrumAlgResult);
    }
    return 0;
}

void DroneSnifferCmdproc::SetSendBuf(uint32_t bufLen)
{
    CmdprocBase::SetSendBuf(bufLen);
    int32_t maxNum = _GetDroneMaxNum();
    if (maxNum < _droneNum)
    {
        _droneNum = maxNum;
    }
}

void DroneSnifferCmdproc::_RegisterCmd()
{
    Register(EAP_CMD_DRONE_SNIFFER, (Cmdproc_Func)&DroneSnifferCmdproc::_OnDroneSnifferCmd);
}

void DroneSnifferCmdproc::_OnDroneSnifferCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    DroneSniffer_param_t *param = (DroneSniffer_param_t *)(inMsg.data);
    int32_t nDrone = 0;
    // 计算测幅频率范围
    int useRows = 1500;
    float tmpF = 0;
    float tmpBW = 0;
    float dtF;
    float tmpUp = 0;
    float tmpDown = 0;
    float tmpML;
    float tmpMW;
    float tmpAm;
    unsigned short output[1024];
	unsigned int flagMark;

    // printf("%s,%d: 调用新算法sfDroneSnifferV1()处理频谱侦测数据.\n", __func__, __LINE__);
    sfV2DroneSniffer(_droneInfo, &nDrone, param->specMat, param->upMat, param->cenFreq, param->fs, param->bw, param->gain, param->useRow, output, &flagMark);

    if (nDrone > _droneNum)
    {
        EAP_LOG_ERROR("dronenum(%d > %d) is error", nDrone, _droneNum);
        return;
    }

    uint16_t *droneNum = (uint16_t *)(outMsg.data);
    *droneNum = nDrone;
    SpectrumAlgResult* result = (SpectrumAlgResult* )(outMsg.data + 2);
    for (int i = 0; i < nDrone; ++i)
    {
        memcpy(result[i].name, _droneInfo[i].name, sizeof(result[i].name));
        result[i].freq[0] = _droneInfo[i].freq[0];
        result[i].amp = _droneInfo[i].amp;
        // EAP_LOG_DEBUG("nDrone=%d, _droneInfo[i=%d].name=[%s], freq=%0.1f, tmpAm=%0.3f.\n", nDrone, i, _droneInfo[i].name, _droneInfo[i].freq[0], tmpAm);
    }
    outMsg.dataLen = nDrone * sizeof(SpectrumAlgResult) + 2;
    // EAP_LOG_DEBUG("返回算法计算结果: param->nDrone=%d.\n", param->nDrone);
}
