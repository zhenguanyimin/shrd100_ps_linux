#include "CmdprocBase.h"

extern "C"
{
#include "sfAirBorneSniffer.h"
}

#if 0 // 功能移入配置文件product_debug_cfg.json, 定义见ProductDebugCfg.proto
/* 机载Tracer调试采集数据 */
#define LOG_FILE_PATH "/run/media/mmcblk0p3/log"
// 无人机下发指令连续采集30次时频图数据(需在DataPath里保存DMA传输的原始数据)
#define UAV_DEBUG_DATA_FILE_PATH "/run/media/mmcblk0p3/log/debug"
#define UAV_DEBUG_DATA_FILE_PREFIX "spectrogram"
// 开机自动收集1000帧算法缓存数据(算法处理回填的alg_debug_data[30*1024])
#define ALG_CACHE_DATA_FILE_PATH "/run/media/mmcblk0p3/log/cache"
#define ALG_CACHE_DATA_FILE_PREFIX "alg-cache"
#define ALG_CACHE_DATA_MAX_NUM   10     // 最多保留10份日志文件
#define ALG_CACHE_DATA_MAX_FRAME 30000  // 每份日志最多存储30000帧数据
#define ALG_CACHE_DATA_FRAME_SIZE 15*1024   // 每帧15K*2数据=30KB字节
// 算法发现异常自动触发保存当前时频图数据(已经过SpectrumCmdproc模块高低16位分离)
#define ALG_EXCEPT_DATA_FILE_PATH "/run/media/mmcblk0p3/log/except"
#define ALG_EXCEPT_DATA_FILE_PREFIX "alg-except"
#define ALG_EXCEPT_DATA_MAX_NUM  100    // 最多保留100份异常数据文件
#define CURRENT_LOG_INDEX "current_log_index"
#else
#define ALG_CACHE_DATA_FRAME_SIZE 15*1024   // 每帧15K*2数据=30KB字节
#endif

class AirBorneSnifferCmdproc : public CmdprocBase
{
public:
    AirBorneSnifferCmdproc();
    ~AirBorneSnifferCmdproc();
    void SetSendBuf(uint32_t bufLen) override;
protected:
    void _RegisterCmd() override;
    void _OnAirBorneSnifferCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _RecordAlgCacheData();
    void _RecordAlgExceptData(void *data);
private:
    // 机载测向输出参数
    ioMeasureInfo_t _ioMeasureInfo;
    // 算法调试数据30KB字节
    short alg_debug_data[ALG_CACHE_DATA_FRAME_SIZE];
    int flag_debug_airborne;
};
