#include "json_pb_converter.h"
#include "TracerUavMsg.pb.h"
#include "SpectrumCmdproc.h"
#include "SpectrumModule.h"
#include "AirBorneProc.h"
#include "AirBorneSnifferModule.h"
#include "DroneSnifferModule.h"
#include "AlinkCmdproc.h"
#include "HalMgr.h"
#include "checksum.h"

SpectrumCmdproc::SpectrumCmdproc()
{
    if (ProductSwCfgGet().issupportspectrum())
    {
        _targetMaxNum = EAP_SPECTRUM_TARGET_MAX_NUM;
        _respMaxLen = _targetMaxNum * sizeof(SpectrumAlgResult) + 2;
        _respData = new uint8_t[_respMaxLen];
    }
    else if (ProductSwCfgGet().issupportairborne())
    {
        _targetMaxNum = 4;
        _respMaxLen = sizeof(ioMeasureInfo_t);
        _respData = new uint8_t[_respMaxLen];
        printf("%s,%d: 设置接收消息长度为%d.\n", __func__, __LINE__, _respMaxLen);
    }
}

SpectrumCmdproc::~SpectrumCmdproc()
{
    EAP_DELETE_ARRAY(_respData);
}

void SpectrumCmdproc::_RegisterCmd()
{
    Register(EAP_CMD_SPECTRUM, (Cmdproc_Func)&SpectrumCmdproc::_OnSpectrumCmd);
    if (ProductSwCfgGet().issupportspectrumorient())
        Register(EAP_CMD_CLEAR_ORIENTATE, (Cmdproc_Func)&SpectrumCmdproc::_OnClearOrientateCmd);
    Register(EAP_CMD_SHOW_SPECTRUM_MODULE, (Cmdproc_Func)&SpectrumCmdproc::_OnShowSpectrumModuleCmd);

    // 机载Tracer状态控制
    if (ProductSwCfgGet().issupportairborneorient())
        Register(EAP_CMD_AIRBORNE_STATE_CTRL, (Cmdproc_Func)&SpectrumCmdproc::_OnAirBorneCtrlCmd);
}

void SpectrumCmdproc::_OnAirBorneCtrlCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    ioController_t* state = (ioController_t*)inMsg.data;
    SpectrumModule* spectrumModule = dynamic_cast<SpectrumModule*>(_module);
    if (nullptr != spectrumModule)
    {
        spectrumModule->AirBorneCtrl(state);
    }
}

void SpectrumCmdproc::_OnClearOrientateCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    SpectrumModule* spectrumModule = dynamic_cast<SpectrumModule*>(_module);
    if (nullptr != spectrumModule)
    {
        spectrumModule->ClearOrientate();
    }
}

void SpectrumCmdproc::_OnSpectrumCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    if (EapGetSys().GetSysStatus() != SYS_DETECTION_TARGET_STATUS) return;

    Spectrum_DataPingPong* BufMsg = (Spectrum_DataPingPong*)inMsg.data;
    uint32_t *specData = (uint32_t *)BufMsg->buff1.pData;
    // printf("%s,%d: 收到频谱侦测数据.\n", __func__, __LINE__);
    if (ProductSwCfgGet().issupportspectrum())
    {
        _SetSpecMatrix(specData);
    }
    else if (ProductSwCfgGet().issupportairborne())
    {
        _SetAirBorneMatrix(specData);
    }
}

void SpectrumCmdproc::_SetSpecMatrix(uint32_t *specData)
{
    DroneSniffer_param_t *param = (DroneSniffer_param_t *)(_sendBuf + sizeof(Thread_Msg_Head));
    param->cenFreq = EapGetSys().GetCurDetectFreq();
    param->fs = 51.2f;
    param->bw = 48.0f;
    param->gain = 10.0f;
    param->useRow = 3000;
    param->SNR = 0.0f;
    uint32_t respLen = _respMaxLen;
    uint32_t (*threeDArray)[EAP_SPECTRUM_LINE_NUM][EAP_SPECTRUM_COLUMN_NUM] = (uint32_t (*)[EAP_SPECTRUM_LINE_NUM][EAP_SPECTRUM_COLUMN_NUM])specData;

    for (int z = 0; z <= EAP_SPECTRUM_N; z++)
    {
        for (int i = 0; i < EAP_SPECTRUM_LINE_NUM; i++)
        {
            for (int j = 0; j < EAP_SPECTRUM_COLUMN_NUM; j++)
            {
                if (z == 0)
                    param->specMat[0][i][j] = (unsigned short)((((*threeDArray)[i][j]) & 0xFFFF0000) >> 16);
                else
                    param->upMat[0][i][j] = (unsigned short)((((*threeDArray+z)[i][j]) & 0xFFFF0000) >> 16);
            }
        }
    }

    // 开始切频
    SendAsynMsg(EAP_CMD_FREQUENCY, _sendBuf, 0);
    // 传给算法处理数据
    SendSynMsg(_threadId, EAP_CMD_DRONE_SNIFFER, _sendBuf, DRONEID_SNIFFER_PARAM_LEN, _respData, &respLen);
    uint16_t* droneNum = (uint16_t*)_respData;
    SpectrumAlgResult* result = (SpectrumAlgResult*)(_respData + 2);
    if ((*droneNum) * sizeof(SpectrumAlgResult) + 2 != respLen && respLen > _respMaxLen)
    {
        EAP_LOG_ERROR("respLen(%d) is error!", respLen);
        return;
    }
    SpectrumModule* spectrumModule = dynamic_cast<SpectrumModule*>(_module);
    if (nullptr != spectrumModule) spectrumModule->Sensing(param->cenFreq, result, *droneNum);
}

void SpectrumCmdproc::_SetAirBorneMatrix(uint32_t *specData)
{
    AirBorne_param_t *param = (AirBorne_param_t *)(_sendBuf + sizeof(Thread_Msg_Head));
    uint32_t respLen = _respMaxLen;
    uint32_t (*threeDArray)[EAP_SPECTRUM_LINE_NUM][EAP_SPECTRUM_COLUMN_NUM] = (uint32_t (*)[EAP_SPECTRUM_LINE_NUM][EAP_SPECTRUM_COLUMN_NUM])specData;
    int ret;

    for (int z = 0; z <= EAP_SPECTRUM_N; z++)
    {
        for (int i = 0; i < EAP_SPECTRUM_LINE_NUM; i++)
        {
            for (int j = 0; j < EAP_SPECTRUM_COLUMN_NUM; j++)
            {
                if (z == 0)
                {
                    param->am1Mat[0][i][j] = (unsigned short)(((*threeDArray)[i][j]) & 0x0000FFFF);
                    param->am2Mat[0][i][j] = (unsigned short)((((*threeDArray)[i][j]) & 0xFFFF0000) >> 16);
                }
                else
                {
                    param->ph1Mat[0][i][j] = (unsigned short)(((*(threeDArray+z))[i][j]) & 0x0000FFFF);
                    param->ph2Mat[0][i][j] = (unsigned short)((((*(threeDArray+z))[i][j]) & 0xFFFF0000) >> 16);
                }
            }
        }
    }
    param->cenFreq = EapGetSys().GetCurDetectFreq();

    SpectrumModule* spectrumModule = dynamic_cast<SpectrumModule*>(_module);
    if (nullptr != spectrumModule)
        ret = spectrumModule->AirBorneGetParam(param);
    else
        ret = -1;
    if (ret < 0)
    {
        EAP_LOG_ERROR("error(%d) Failed to get param!", ret);
        return;
    }

    // 开始切频(没有补偿数据时将不再工作)
    SendAsynMsg(EAP_CMD_FREQUENCY, _sendBuf, 0);

    // printf("%s,%d: 发送给AirBorne Sniffer模块处理.\n", __func__, __LINE__);
    // 传给算法处理数据
    SendSynMsg(_threadId, EAP_CMD_AIRBORNE_SNIFFER, _sendBuf, AIRBORNE_PARAM_LEN, _respData, &respLen);

    // printf("%s,%d: 发送结果给无人机.\n", __func__, __LINE__);
    // 将结果发送给UAV
    ioMeasureInfo_t* result = (ioMeasureInfo_t*)_respData;
    // 需要将结构体封装成Pb格式再发送给UAV
    _ReportAirBorneInfo(result);
    // printf("%s,%d: 处理结束.\n", __func__, __LINE__);
}

void SpectrumCmdproc::_ReportAirBorneInfo(ioMeasureInfo_t* result)
{
    const std::uint32_t maxLength = 512;
    std::uint8_t buffer[512] = {0};
    protobuf::traceruav::MeasureInfoPb meaInfoData;
    protobuf::traceruav::TracerUavMessagePB uavMsgData;

    meaInfoData.set_workstatus(result->workStatus);
    meaInfoData.set_tarnum(result->tarNum);

    // 没有无人机也要发送数据
    // if (result->tarNum >= 1)
    {
        protobuf::traceruav::MeasureInfoPb_MeasureInfoData *meaData = meaInfoData.add_meainfodata();
        meaData->set_tnum(result->t1Num);
        meaData->set_tfreq(result->t1Freq);
        meaData->set_tamp(result->t1Amp);
        meaData->set_tmfw(result->t1mFw);
        meaData->set_tmfy(result->t1mFy);
        meaData->set_tffw(result->t1fFw);
        meaData->set_tffy(result->t1fFy);
    }
    // if (result->tarNum >= 2)
    {
        protobuf::traceruav::MeasureInfoPb_MeasureInfoData *meaData2 = meaInfoData.add_meainfodata();
        meaData2->set_tnum(result->t2Num);
        meaData2->set_tfreq(result->t2Freq);
        meaData2->set_tamp(result->t2Amp);
        meaData2->set_tmfw(result->t2mFw);
        meaData2->set_tmfy(result->t2mFy);
        meaData2->set_tffw(result->t2fFw);
        meaData2->set_tffy(result->t2fFy);
    }
    // if (result->tarNum >= 3)
    {
        protobuf::traceruav::MeasureInfoPb_MeasureInfoData *meaData3 = meaInfoData.add_meainfodata();
        meaData3->set_tnum(result->t3Num);
        meaData3->set_tfreq(result->t3Freq);
        meaData3->set_tamp(result->t3Amp);
        meaData3->set_tmfw(result->t3mFw);
        meaData3->set_tmfy(result->t3mFy);
        meaData3->set_tffw(result->t3fFw);
        meaData3->set_tffy(result->t3fFy);
    }
    // if (result->tarNum >= 4)
    {
        protobuf::traceruav::MeasureInfoPb_MeasureInfoData *meaData4 = meaInfoData.add_meainfodata();
        meaData4->set_tnum(result->t4Num);
        meaData4->set_tfreq(result->t4Freq);
        meaData4->set_tamp(result->t4Amp);
        meaData4->set_tmfw(result->t4mFw);
        meaData4->set_tmfy(result->t4mFy);
        meaData4->set_tffw(result->t4fFw);
        meaData4->set_tffy(result->t4fFy);
    }

    uavMsgData.mutable_measureinfo()->CopyFrom(meaInfoData);
    uavMsgData.set_msgchannel("");
    uavMsgData.set_msgtype(protobuf::traceruav::TRACERUAVMSG_MEASURE_INFO_TYPE);
    uavMsgData.set_msgname("meaInfoData");
    uavMsgData.set_msghandle(0);
    uavMsgData.set_msgseq(1);

    std::size_t dataLength = uavMsgData.ByteSizeLong();
    if (dataLength <= maxLength)
    {
        uavMsgData.SerializeToArray(buffer, dataLength);
    }

    alink_msg_t msg = {0};
    msg.channelType = EAP_CHANNEL_TYPE_TYPEC;
    msg.channelId   = UINT16_ALL;
    memcpy((void*)msg.buffer, buffer, dataLength);
    AlinkCmdproc::SendAlinkCmd(msg, dataLength, EAP_ALINK_RPT_TRACER_UAV_MESSAGE);
}

void SpectrumCmdproc::_OnTimer(uint8_t timerType)
{
    if (EAP_CMDTYPE_TIMER_1 == timerType)
    {
        if (ProductSwCfgGet().issupportspectrum())
        {
            _ReportSpectrumDroneInfo();
        }
    }
}

void SpectrumCmdproc::_ReportSpectrumDroneInfo()
{
    SpectrumModule* spectrumModule = dynamic_cast<SpectrumModule*>(_module);
    if (nullptr != spectrumModule)
    {
        alink_msg_t msg = {0};
        msg.channelType = EAP_CHANNEL_TYPE_TCP | EAP_CHANNEL_TYPE_TYPEC;
        msg.channelId   = UINT16_ALL;
        int16_t dataLen = spectrumModule->FillSpectrumInfo(msg.buffer, sizeof(msg.buffer) - 2);
        if (-1 == dataLen) return;
        AlinkCmdproc::SendAlinkCmd(msg, dataLen, EAP_ALINK_RPT_SPECTRUM);
    }
}

void SpectrumCmdproc::SetTargetMaxNum(uint16_t num)
{
    if (ProductSwCfgGet().issupportspectrum())
    {
        if (num > _targetMaxNum)
        {
            EAP_DELETE_ARRAY(_respData);
            _targetMaxNum = num;
            _respMaxLen = _targetMaxNum * sizeof(SpectrumAlgResult) + 2;
            _respData = new uint8_t[_respMaxLen];
        }
    }
    else if (ProductSwCfgGet().issupportairborne())
    {
        EAP_DELETE_ARRAY(_respData);
        _targetMaxNum = 4;
        _respMaxLen = sizeof(ioMeasureInfo_t) + 2;
        _respData = new uint8_t[_respMaxLen];
    }
}

void SpectrumCmdproc::_OnShowSpectrumModuleCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    _DoShowModuleInfo();
}