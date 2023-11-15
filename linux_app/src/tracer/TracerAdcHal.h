#pragma once


#include <stdint.h>
#include <unistd.h>
#include "HalBase.h"
#include "eap_pub.h"
#include "TracerCommonHal.h"


#ifdef __cplusplus
extern "C" {
#endif
#include "hal.h"
#include "ad9361/ad9361_config.h"
#include "gpio/gpioapp.h"
#include "output/output.h"
#include "input/input.h"
#include "switch_filter/switch_filter.h"
#ifdef __cplusplus
}
#endif

/*** 开启AD9361初始化，当前版本bit流不匹配会导致配置失败 ***/
//#define EXTRACT_ADC_MODULE_EN		1 // 0:disable, 1:enable
//如果没有射频模块，请在编译的时候加-DNO_ADC_MODULE=1

#define BURST_IRQ_DEV_NAME "/dev/PL-burst_irq_event"

enum IRD_RATE
{
	IRD_RATE_5_12us,
	IRD_RATE_2_56us,
	IRD_RATE_10_24us,
};

enum RF_EVENT
{
	eRF_AD9361_CFG_F,
	eRF_PL_START_COLLECT,
	eRF_PL_STOP_COLLECT,
	eRF_AD9361_CFG_HIT_915,
	eRF_AD9361_CFG_HIT_GPS,
	eRF_AD9361_CFG_HIT_GPS_915,
	eRF_AD9361_CFG_CLOSE_HIT,
};

enum PL_REG_ADDR
{
	eCFG_PWR_GATE_L = XPAR_M08_AXI_BASEADDR + 0x00,
	eCFG_PWR_GATE_H = XPAR_M08_AXI_BASEADDR + 0x04,
	eCFG_RF_MOD 	= XPAR_M08_AXI_BASEADDR + 0x08,
	eCFG_PINC_CH0 	= XPAR_M08_AXI_BASEADDR + 0x0c,
	eCFG_PINC_CH1 	= XPAR_M08_AXI_BASEADDR + 0x10,
	eIDX_FREQOFFSET = XPAR_M08_AXI_BASEADDR + 0x20,
	eRF_MOD 		= XPAR_M08_AXI_BASEADDR + 0x24,
	eWR_DDR_ENABLE 	= XPAR_M09_AXI_BASEADDR + 0x14,
};

enum
{
    ANTENNA_SWITCH_MODE0 = 0,   // A3+B2天线
    ANTENNA_SWITCH_MODE1,       // B2+A3天线
    ANTENNA_SWITCH_MODE2,       // A3+B1天线
    ANTENNA_SWITCH_MODE3,       // B1+A3天线
    ANTENNA_SWITCH_MODE4,       // A3+A2天线
    ANTENNA_SWITCH_MODE5,       // A2+A3天线
    ANTENNA_SWITCH_MODE6,       // B2+B1天线
    ANTENNA_SWITCH_MODE7,       // B1+B2天线
    ANTENNA_SWITCH_MODE8,       // B2+A2天线
    ANTENNA_SWITCH_MODE9,       // A2+B2天线
    ANTENNA_SWITCH_MODE10,      // B1+A2天线
    ANTENNA_SWITCH_MODE11,      // A2+B1天线
};

typedef struct freq_cfg
{
    uint64_t uFreq;      //hz
    int32_t uFreqOffsetL; //Frequency left Offset
    int32_t uFreqOffsetR; //Frequency right Offset
} freq_cfg_t;


/*


*/
#define BURST_DATA_MAGIC_HEAD 0x55aa55aa
#define BURST_DATA_MAGIC_TAIL 0xaa55aa55

#pragma pack(1)
/*
add by huaguoqing,2023.09.01
报文是小序的. PL and Ps 侧都是little endian. 所以不用转字节序
校验规则：
1. 简单校验
    （0）magic_head,magic_tail
	（1）cnt1 - cnt0 =  frame_size; frame_size是包含帧头和帧尾的。
	（2）payload长度是4字节对齐的, < buffer接收区长度。
2. 完整校验（在简单校验基本上增加）
	（0）crc校验

此消息结构定义，存在明显不合理地方：
	（1）crc校验没有对头尾进行校验。
	（2）分为帧头和帧尾两部分，处理不方便
	（3）数据crc放在帧尾最好


| burst_data_front_header_t | payload data | burst_data_tail_header_t |
	
*/
typedef struct  burst_data_font_header
{
	uint64_t cnt0;
	
	uint32_t payload_uint64_items_count; //payload_uint64_items_count unit is not bytes. but 8bytes as an item.()
	uint32_t magic_head;//0x55aa55aa

	uint32_t head_rsv_0;
	uint32_t frame_seq; // squence number of frame, auto increase.
	
	char     payload[0];
}burst_data_front_header_t;

typedef struct  burst_data_tail_header
{
	uint32_t tail_rsv_0; 
	uint32_t magic_tail;//0xaa55aa55
	
	uint64_t sto;
	uint64_t tail_rsv[13];
	uint64_t cnt1;
	uint64_t crc;
}burst_data_tail_header_t;

#pragma pack()

//对burst_data接收统计信息
typedef struct burst_data_rcv_stat
{
	int      is_check_crc;
	uint64_t rcv_count;
	uint64_t seq_start;
	uint64_t seq_now;
	uint64_t last_rcv_tick;
		
	//异常情况统计
	uint64_t frame_err_count; //如果运行中，不改变is_check_crc，则frame_err_count = header_parse_fail_count + data_crc_fail_count
	uint64_t header_parse_fail_count;
	uint64_t data_crc_fail_count;
	
	uint64_t frame_lost_count;
	uint64_t frame_rcv_min_tick;  // if 0, min tick is invalid.
	uint32_t frame_lost_max_step; // if 0, lost_max_step invalid.
	
}burst_data_rcv_stat_t;



class TracerAdcHal : public HalBase
{
public:
    TracerAdcHal(HalMgr* halMgr);
    virtual ~TracerAdcHal();
    void Init() override;
    int GetCurFreq(float *freq, int *index);

protected:
    int _AdcGetFreqRegValue(const Hal_Data& inData, Hal_Data& outData);
    int _AdcGetBurstData(const Hal_Data& inData, Hal_Data& outData);
    int _AdcGetDetectionParam(const Hal_Data& inData, Hal_Data& outData);
    int _AdcSetDetectFreq(const Hal_Data& inData, Hal_Data& outData);
    int _AdcSetGain(const Hal_Data& inData, Hal_Data& outData);
    int _AdcSetGainAgc(const Hal_Data& inData, Hal_Data& outData);

    void _InitBurstDataIrqInfoTable();

private:
    float _curFreq;
    int _curFreqIndex;
};

extern "C" {
void RF_StopPLAlg(void);
void RF_StartPLAlgByTimer(void);
}
