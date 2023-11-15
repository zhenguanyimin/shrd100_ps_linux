#include "json_pb_converter.h"
#include "AirBorneSnifferCmdproc.h"
#include "AirBorneSnifferModule.h"
#include <unistd.h>

extern "C"
{
#include <sys/stat.h>
#include "sfAirBorneSniffer.h"
}

AirBorneSnifferCmdproc::AirBorneSnifferCmdproc()
{
    if (access(FLAG_DEBUG_AIRBORNE_FILE, F_OK) == 0)
        flag_debug_airborne = 1;
    else
        flag_debug_airborne = 0;
    EAP_LOG_DEBUG("%sAirBorne调试功能\n", flag_debug_airborne?"开启":"关闭");
}

AirBorneSnifferCmdproc::~AirBorneSnifferCmdproc()
{
}

void AirBorneSnifferCmdproc::SetSendBuf(uint32_t bufLen)
{
    CmdprocBase::SetSendBuf(bufLen);
}

void AirBorneSnifferCmdproc::_RegisterCmd()
{
    Register(EAP_CMD_AIRBORNE_SNIFFER, (Cmdproc_Func)&AirBorneSnifferCmdproc::_OnAirBorneSnifferCmd);
}

void AirBorneSnifferCmdproc::_OnAirBorneSnifferCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    AirBorne_param_t *param = (AirBorne_param_t *)(inMsg.data);

    // EAP_LOG_DEBUG("调用算法sfAirborneProcessing()处理机载测向数据.\n");
    sfArbnProcessing(param->bcData, param->am1Mat, param->am2Mat, param->ph1Mat, param->ph2Mat, param->cenFreq, param->antStatus, param->mCnt, param->inC, param->inF, &_ioMeasureInfo, alg_debug_data);
    // 将结果返回给飞机
    ioMeasureInfo_t* result = (ioMeasureInfo_t* )outMsg.data;
    memcpy(result, &_ioMeasureInfo, AIRBORNE_RESULT_LEN);
    outMsg.dataLen = AIRBORNE_RESULT_LEN;

    // 调试信息: 记录算法导出数据
    if (ProductDebugCfgGet().issupportalgcache() && param->inC.workMode > 0)
        _RecordAlgCacheData();

    if (flag_debug_airborne)
    {
        // 调试信息: 打印算法输出结果
        if (_ioMeasureInfo.tarNum > 0)
        {
            printf("[%s:%d] 侦测到无人机: _measureInfo->tarNum=%d, workStatus=%d.\n", __func__, __LINE__, _ioMeasureInfo.tarNum, _ioMeasureInfo.workStatus);
            if (_ioMeasureInfo.tarNum >= 1)
            {
                printf("\tt1Num=%u, t1Freq=%u, t1Amp=%u, t1mFw=%d, t1mFy=%d, t1fFw=%d, t1fFy=%d\n",
                    _ioMeasureInfo.t1Num, _ioMeasureInfo.t1Freq, _ioMeasureInfo.t1Amp,
                    _ioMeasureInfo.t1mFw, _ioMeasureInfo.t1mFy, _ioMeasureInfo.t1fFw, _ioMeasureInfo.t1fFy);
            }
            if (_ioMeasureInfo.tarNum >= 2)
            {
                printf("\tt2Num=%u, t2Freq=%u, t2Amp=%u, t2mFw=%d, t2mFy=%d, t2fFw=%d, t2fFy=%d\n",
                    _ioMeasureInfo.t2Num, _ioMeasureInfo.t2Freq, _ioMeasureInfo.t2Amp,
                    _ioMeasureInfo.t2mFw, _ioMeasureInfo.t2mFy, _ioMeasureInfo.t2fFw, _ioMeasureInfo.t2fFy);
            }
            if (_ioMeasureInfo.tarNum >= 3)
            {
                printf("\tt3Num=%u, t3Freq=%u, t3Amp=%u, t3mFw=%d, t3mFy=%d, t3fFw=%d, t3fFy=%d\n",
                    _ioMeasureInfo.t3Num, _ioMeasureInfo.t3Freq, _ioMeasureInfo.t3Amp,
                    _ioMeasureInfo.t3mFw, _ioMeasureInfo.t3mFy, _ioMeasureInfo.t3fFw, _ioMeasureInfo.t3fFy);
            }
            if (_ioMeasureInfo.tarNum >= 4)
            {
                printf("\tt4Num=%u, t4Freq=%u, t4Amp=%u, t4mFw=%d, t4mFy=%d, t4fFw=%d, t4fFy=%d\n",
                    _ioMeasureInfo.t4Num, _ioMeasureInfo.t4Freq, _ioMeasureInfo.t4Amp,
                    _ioMeasureInfo.t4mFw, _ioMeasureInfo.t4mFy, _ioMeasureInfo.t4fFw, _ioMeasureInfo.t4fFy);
            }
            printf("\n");
        }
    }
}

void AirBorneSnifferCmdproc::_RecordAlgCacheData()
{
    static int count = 0;
    static int index;
    char index_path[256];
    char log_path[256];
    FILE* fp;
    int ret;

    // 每次开机只记录一次算法缓存数据,满3万帧后停止记录
    if (count == 0)
    {
        // 获取最新算法缓存日志的索引
        snprintf(index_path, 256, "%s/%s", ProductDebugCfgGet().algcacheattr().file_path().c_str(), ProductDebugCfgGet().algcacheattr().file_index().c_str());
        fp = fopen(index_path, "r");
        if (fp)
        {
            ret = fscanf(fp, "%d", &index);
            fclose(fp);
            // 最多保留10份缓存数据文件,超过就覆盖前面的
            if (ret <= 0 || index >= ProductDebugCfgGet().algcacheattr().file_max_num())
                index = 1;
            else
                index++;
        }
        else
            index = 1;
        EAP_LOG_DEBUG("新的索引号为%d.\n", index);
        // 写入新索引号
        fp = fopen(index_path, "w");
        if (fp)
        {
            fprintf(fp, "%d", index);
            fclose(fp);
        }
    }

    // 最多存储30000帧数据
    if (count < ProductDebugCfgGet().algcacheattr().frame_max_num())
    {
        snprintf(log_path, 256, "%s/%s%d.dat", ProductDebugCfgGet().algcacheattr().file_path().c_str(), ProductDebugCfgGet().algcacheattr().file_prefix().c_str(), index);
        if (count == 0)
        {
            if (remove(log_path) == 0)
                EAP_LOG_DEBUG("文件%s已存在,将删除并创建新文件.\n", log_path);
        }
        fp = fopen(log_path, "a");
        if (fp)
        {
            fwrite(alg_debug_data, sizeof(short), ALG_CACHE_DATA_FRAME_SIZE, fp);
            fclose(fp);
            // EAP_LOG_DEBUG("记录alg_debug_data到文件%s.\n", log_path);
        }
        else
            EAP_LOG_DEBUG("记录日志文件%s失败.\n", log_path);
        count++;
    }
}

void AirBorneSnifferCmdproc::_RecordAlgExceptData(void *data)
{
    static int index = 0;
    AirBorne_param_t *param = (AirBorne_param_t *)data;
    char index_path[256];
    char log_path[256];
    FILE* fp;
    int ret;

    snprintf(index_path, 256, "%s/%s", ProductDebugCfgGet().algexceptattr().file_path().c_str(), ProductDebugCfgGet().algexceptattr().file_index().c_str());
    // 先获取最新异常数据日志的索引
    fp = fopen(index_path, "r");
    if (fp)
    {
        ret = fscanf(fp, "%d", &index);
        fclose(fp);
        // 最多保留100份异常数据文件
        if (ret <= 0 || index >= ProductDebugCfgGet().algexceptattr().file_max_num())
            index = 1;
        else
            index++;
    }
    else
        index = 1;
    EAP_LOG_DEBUG("新的索引号为%d.\n", index);
    // 写入新索引号
    fp = fopen(index_path, "w");
    if (fp)
    {
        fprintf(fp, "%d", index);
        fclose(fp);
    }

    // 存储异常数据文件
    snprintf(log_path, 256, "%s/%s%d.dat", ProductDebugCfgGet().algexceptattr().file_path().c_str(), ProductDebugCfgGet().algexceptattr().file_prefix().c_str(), index);
    fp = fopen(log_path, "wb");
    if (fp)
    {
        fwrite(&param->cenFreq, sizeof(float), 1, fp);
        fwrite(param->am1Mat, sizeof(unsigned short), EAP_SPECTRUM_LINE_NUM*EAP_SPECTRUM_COLUMN_NUM, fp);
        fwrite(param->am2Mat, sizeof(unsigned short), EAP_SPECTRUM_LINE_NUM*EAP_SPECTRUM_COLUMN_NUM, fp);
        fwrite(param->ph1Mat, sizeof(unsigned short), EAP_SPECTRUM_LINE_NUM*EAP_SPECTRUM_COLUMN_NUM, fp);
        fwrite(param->ph2Mat, sizeof(unsigned short), EAP_SPECTRUM_LINE_NUM*EAP_SPECTRUM_COLUMN_NUM, fp);
        EAP_LOG_DEBUG("算法检查到异常后存储当前数据到文件%s.\n", log_path);
    }
    fclose(fp);
}
