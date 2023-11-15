#include "json_pb_converter.h"
#include "TracerAdcHal.h"
#include "TracerDataPath.h"
#include "TracerMainHalMgr.h"
#include "TracerUtils_c.h"
#include "FrequencyCmdproc.h"
#include "../srv/log/log.h"

#define DEBUG_PRINTF_BURST_DATA_STAT  0

#define FLAG_DEBUG_DRONEID_BURST_LOG "/run/media/mmcblk0p1/flag_debug_droneid"
#define FLAG_DEBUG_DRONEID_CHECK_CRC "/run/media/mmcblk0p1/flag_debug_droneid_check_crc"

#define debug_print(fmt, ...) \
	if (flag_debug_droneid) { \
		printf("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__); \
	}

#define CFG_DEBUG_BURST_DATA
#ifdef CFG_DEBUG_BURST_DATA
#define DEBUG_BURST_DATA_PRINTF(arg...) printf(arg)
#else
#define DEBUG_BURST_DATA_PRINTF(arg...) 
#endif



static int flag_debug_droneid = 0;
static uint64_t bufferAdc_64bit[VALID_ADC_BUF_LEN];
extern VirualAddr_t DDR_VirtualAddr;
static void _DDRBurst_intr_Handler(void *psArg);

static int BURST_DATA_IRQ_INFO_TABLE_SIZE = 0; // It's initialized in TracerAdcHal::_InitBurstDataIrqInfoTable
static burst_data_irq_info_t* _sg_burst_data_irq_info_table = nullptr;

static burst_data_rcv_stat_t _sg_stat = {0};

static uint64_t droneID_sig_freq_cfg_buf[] =
{
		(uint64_t)2414500e3,
		(uint64_t)2429500e3,
		(uint64_t)2444500e3,
		(uint64_t)2459500e3,
		(uint64_t)5756500e3,
		(uint64_t)5776500e3,
		(uint64_t)5796500e3,
		(uint64_t)5816500e3,
};

static freq_cfg_t freq_cfg_buf[] =
{
		{(uint64_t)2422000e3, (int32_t)-7500e3, (int32_t)7500e3},
		{(uint64_t)2452000e3, (int32_t)-7500e3, (int32_t)7500e3},
		{(uint64_t)5766500e3, (int32_t)-10000e3, (int32_t)10000e3},
		{(uint64_t)5806500e3, (int32_t)-10000e3, (int32_t)10000e3},
};

static SYS_FrequencyParam_t DetectionParam = {
		DEFINED_SWEEP_FREQ_MODE,
		842,
		0,
		{0, 842},
		{400, 400, 6000, 40},
		{12, 40, 0, {842, 915, 2420, 2460, 5160, 5200, 5240, 5745, 5765, 5785, 5805, 5825, 0, 0}},
		{40, 34, 0}
};

static SYS_FrequencyParam_t DronidDetectionParam = {
		DEFINED_SWEEP_FREQ_MODE,
		2422,
		0,
		{0, 2422},
		{400, 2422, 6000, 40},
		{4, 40, 0, {2422, 2452, 5766.5, 5806.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{40, 34, 0}
};

static SYS_FrequencyParam_t SpectrumDetectionParam = {
		DEFINED_SWEEP_FREQ_MODE,
		842,
		0,
		{0, 842},
		{400, 400, 6000, 40},
		{30, 40, 0, {842,915,1080,1120,1160,1200,1240,1280,1320,1360,1437,2422,2437,2462,5160,5180,5200,5220,5240,5655,5695,5725,5745,5765,5785,5805,5825,5850,5890,5930}},
		{40, 34, 0}
};

static SYS_FrequencyParam_t AirBorneDetectionParam = {
		DEFINED_SWEEP_FREQ_MODE,
		2420,
		0,
		{0, 5785},		// 机载Tracer缺省定频5785
		{400, 400, 6000, 40},
		{5, 40, 0, {2420,2460,5745,5785,5825}},
		{40, 34, 0}
};

float SYS_GetCenterFreq(uint32_t CurFreqIndex)
{
	int workMode = EapGetSys().GetWorkMode();
	if (workMode == WORKMODE_DRONE)
		return DronidDetectionParam.DefinedSweepParam.DefinedFreq[CurFreqIndex];
	else if (workMode == WORKMODE_SPECTRUM_OMNI || workMode == WORKMODE_SPECTRUM_OMNI_ORIE)
		return SpectrumDetectionParam.DefinedSweepParam.DefinedFreq[CurFreqIndex];
	else if (workMode == WORKMODE_AIRBORNE_OMNI || workMode == WORKMODE_AIRBORNE_OMNI_ORIE)
		return AirBorneDetectionParam.DefinedSweepParam.DefinedFreq[CurFreqIndex];
}

void RfConfig_Init()
{
	uint32_t PL_Version_date;
	uint32_t PL_Version_time;
	uint8_t uDetectFreqItem = 0;
	float uLocalOscFreq;


	PL_Version_date = axi_read_data(PL_VOERSION_VP0); /*set threshold*/
	PL_Version_time = axi_read_data(PL_VOERSION_VP1); /*set threshold*/

	axi_write_data(PL_FAN_PWM_RATE, 0x0001);
	/* 屏蔽RTOS继承过来的无效代码 */
	// axi_write_data(PL_DLY_EN_VTC, 0x0000);
	// axi_write_data(PL_DLY_D, 150);
	// axi_write_data(PL_DLY_LOAD, 0x1FFF);
	// axi_write_data(PL_DLY_LOAD, 0x0000);
	// axi_write_data(PL_DLY_EN_VTC, 0x1FFF);
	axi_write_data(PL_SL_SWITCH, 2);

	InitAd9361_Rx();

	if( EapGetSys().GetWorkMode() == WORKMODE_DRONE)
	{
		// 销售需求SHRD103L抓飞手版本支持Mini 4 Pro机型，得提高阈值
		if (!strcmp(ProductSwCfgGet().vertype().c_str(), "SHRD103L"))
			axi_write(eCFG_PWR_GATE_L, 0x00300000);
		else
			axi_write(eCFG_PWR_GATE_L, 0x00200000);
		axi_write(eCFG_PWR_GATE_H, 0x00000000);
		axi_write(eWR_DDR_ENABLE, 1);
	}

#if 1	// 提高代码可读性
	axi_write_data(PL_DLY_EN_VTC, 0x00);
	axi_write_data(PL_DLY_LOAD, 0x00);
	// delay设为0x135时波形有毛刺,改为0x100
	axi_write_data(PL_DLY_D, 0x100);
	axi_write_data(PL_O_DLY_D_FRAME, 0x100);
	axi_write_data(PL_DLY_LOAD, 0x7F);
	axi_write_data(PL_DLY_EN_VTC, 0x7F);
#else
	axi_write(0x80000024, 0x00);	// PL_DLY_EN_VTC
	axi_write(0x80000028, 0x00);	// PL_DLY_LOAD
	axi_write(0x8000002C, 0x100);	// PL_DLY_D
	axi_write(0x80000030, 0x100);	// PL_O_DLY_D_FRAME
	axi_write(0x80000028, 0x7F);	// PL_DLY_LOAD
	axi_write(0x80000024, 0x7F);	// PL_DLY_EN_VTC
#endif

	axi_write_data(PL_AD_START, 1);

	// sPL_Cfg = aeagCfg_GetPlSignalHandlingCfg();
	uLocalOscFreq = SetRxFreq(SYS_GetCenterFreq(uDetectFreqItem));

	if ((uLocalOscFreq >= 2200) && (uLocalOscFreq <= 2500))
	{
		Adrf5250Ctrl(eRF5250_CHANNEL3);
	}
	else if ((uLocalOscFreq >= 5000) && (uLocalOscFreq <= 5900))
	{
		Adrf5250Ctrl(eRF5250_CHANNEL5);
	}
	else
	{
		Adrf5250Ctrl(eRF5250_CHANNEL3);
	}

	printf("\r\n###PL_Version: %X %X###\r\n", PL_Version_date & 0xFFFF, PL_Version_time);

	axi_write_data( PL_IRDRATE, IRD_RATE_5_12us );

	printf("RfConfig_Init(uLocalOscFreq=%0.1f) success.\r\n", uLocalOscFreq);
}

void RF_StopPLAlg(void)
{
	axi_write_data(PL_AD_START, 0);
}

void RF_StartPLAlgByTimer(void)
{
	usleep(10);
	if (EapGetSys().GetWorkMode() != WORKMODE_DRONE)
		ClearDetDmaBuf();
	axi_write_data(PL_AD_START, 1);
}

int TracerAdcHal::_AdcGetFreqRegValue(const Hal_Data& inData, Hal_Data& outData)
{
	uint8_t workMode = INDATA_U8(inData);
	Drv_Drone_Freq *freq = (Drv_Drone_Freq *)(outData.buf);
	uint32_t uIdxFreqoffset = 0;
	uint32_t uRfMod = 0;

	if (workMode == WORKMODE_DRONE)
	{
		uIdxFreqoffset = axi_read(eIDX_FREQOFFSET);
		uRfMod = axi_read(eRF_MOD);
		if (uIdxFreqoffset == 0)
			freq->freqOffset = freq_cfg_buf[uRfMod].uFreqOffsetL;
		else
			freq->freqOffset = freq_cfg_buf[uRfMod].uFreqOffsetR;
		freq->sigFreq = droneID_sig_freq_cfg_buf[uRfMod * 2 + uIdxFreqoffset];
	}
	return EAP_SUCCESS;
}

int TracerAdcHal::_AdcGetBurstData(const Hal_Data& inData, Hal_Data& outData)
{
	uint64_t *bufferAdc = (uint64_t *)(outData.buf);
	burst_data_event *event = (burst_data_event *)inData.buf;
	burst_data_irq_info_t *info = (burst_data_irq_info_t *)&_sg_burst_data_irq_info_table[event->channel];
	debug_print("DroneID读取数据: channel=%d, length=%d(%d*8), phyAddress=%p, virtualAddr=%p\n",
                    info->channel, info->phy_data_len, info->phy_addr, info->vir_addr,
                    VALID_ADC_LEN_WITH_HEAD);
	for (uint32_t i = 0; i < VALID_ADC_LEN_WITH_HEAD; i++)
	{
		bufferAdc[i] = info->bufferAdc_64bit[i];
	}

	return EAP_SUCCESS;
}

int TracerAdcHal::_AdcGetDetectionParam(const Hal_Data& inData, Hal_Data& outData)
{
	uint8_t workMode = INDATA_U8(inData);
	SYS_FrequencyParam_t *param = (SYS_FrequencyParam_t *)(outData.buf);
	printf("%s: workMode: %d\r\n", __func__, workMode);
	switch( workMode )
	{
		case WORKMODE_DRONE:
		{
			memcpy(param, &DronidDetectionParam, sizeof(DronidDetectionParam));
			break;
		}
		case WORKMODE_SPECTRUM_OMNI:
		case WORKMODE_SPECTRUM_OMNI_ORIE:
		{
			memcpy(param, &SpectrumDetectionParam, sizeof(SpectrumDetectionParam));
			break;
		}
		case WORKMODE_AIRBORNE_OMNI:
		case WORKMODE_AIRBORNE_OMNI_ORIE:
		{
			memcpy(param, &AirBorneDetectionParam, sizeof(AirBorneDetectionParam));
			break;
		}
		default:
			break;
	}

	return EAP_SUCCESS;
}

int TracerAdcHal::_AdcSetDetectFreq(const Hal_Data& inData, Hal_Data& outData)
{
	Drv_SetDetectFreq *param = (Drv_SetDetectFreq *)(inData.buf);
	static uint8_t channel = eRF5250_CHANNEL3;

	RF_StopPLAlg();

	// 打击状态下进入保护模式，还有故障状态也不做切频操作
	uint8_t status = EapGetSys().GetSysStatus();
	if (status == SYS_HIT_TARGET_STATUS || status == SYS_MALFUNCTION_STATUS)
	{
		EAP_LOG_INFO("当前系统状态为%d, 不进行切频操作!\n", status);
		return EAP_SUCCESS;  // RF保护状态不更新频率
	}

	if (param->mode == WORKMODE_DRONE && param->freqIndex != UINT8_INVALID)
	{
		axi_write(eCFG_RF_MOD, param->freqIndex);
	}
	if (get_device_type() == BOARD_TYPE_T2)
	{
		if ((param->freq > 400) && (param->freq <= 1000))
		{
			if (channel != HMC345ALP3E_400M_1000M)
			{
				channel = HMC345ALP3E_400M_1000M;
				GPIO_OutputCtrl(EMIO_RF_CTLA_0, IO_VOL);
				GPIO_OutputCtrl(EMIO_RF_CTLB_0, IO_VOL);
				GPIO_OutputCtrl(EMIO_RF_CTLA_1, IO_VOH);
				GPIO_OutputCtrl(EMIO_RF_CTLB_1, IO_VOH);
				GPIO_OutputCtrl(EMIO_RF_CTLA_2, IO_VOL);
				GPIO_OutputCtrl(EMIO_RF_CTLB_2, IO_VOL);
				GPIO_OutputCtrl(EMIO_RF_CTLA_3, IO_VOH);
				GPIO_OutputCtrl(EMIO_RF_CTLB_3, IO_VOH);
			}
		}
		else if ((param->freq > 2400) && (param->freq <= 2483))
		{
			if (channel != HMC345ALP3E_2400M_2483M)
			{
				channel = HMC345ALP3E_2400M_2483M;
				GPIO_OutputCtrl(EMIO_RF_CTLA_0, IO_VOH);
				GPIO_OutputCtrl(EMIO_RF_CTLB_0, IO_VOL);
				GPIO_OutputCtrl(EMIO_RF_CTLA_1, IO_VOL);
				GPIO_OutputCtrl(EMIO_RF_CTLB_1, IO_VOH);
				GPIO_OutputCtrl(EMIO_RF_CTLA_2, IO_VOH);
				GPIO_OutputCtrl(EMIO_RF_CTLB_2, IO_VOL);
				GPIO_OutputCtrl(EMIO_RF_CTLA_3, IO_VOL);
				GPIO_OutputCtrl(EMIO_RF_CTLB_3, IO_VOH);
			}
		}
		else if ((param->freq >= 5100) && (param->freq <= 5300))
		{
			if (channel != HMC345ALP3E_5100M_5300M)
			{
				channel = HMC345ALP3E_5100M_5300M;
				GPIO_OutputCtrl(EMIO_RF_CTLA_0, IO_VOL);
				GPIO_OutputCtrl(EMIO_RF_CTLB_0, IO_VOL);
				GPIO_OutputCtrl(EMIO_RF_CTLA_1, IO_VOH);
				GPIO_OutputCtrl(EMIO_RF_CTLB_1, IO_VOH);
				GPIO_OutputCtrl(EMIO_RF_CTLA_2, IO_VOL);
				GPIO_OutputCtrl(EMIO_RF_CTLB_2, IO_VOL);
				GPIO_OutputCtrl(EMIO_RF_CTLA_3, IO_VOH);
				GPIO_OutputCtrl(EMIO_RF_CTLB_3, IO_VOH);
			}
		}
		else if ((param->freq > 5650) && (param->freq <= 5850))
		{
			if (channel != HMC345ALP3E_5650M_5850M)
			{
				channel = HMC345ALP3E_5650M_5850M;
				GPIO_OutputCtrl(EMIO_RF_CTLA_0, IO_VOL);
				GPIO_OutputCtrl(EMIO_RF_CTLB_0, IO_VOH);
				GPIO_OutputCtrl(EMIO_RF_CTLA_1, IO_VOH);
				GPIO_OutputCtrl(EMIO_RF_CTLB_1, IO_VOL);
				GPIO_OutputCtrl(EMIO_RF_CTLA_2, IO_VOL);
				GPIO_OutputCtrl(EMIO_RF_CTLB_2, IO_VOH);
				GPIO_OutputCtrl(EMIO_RF_CTLA_3, IO_VOH);
				GPIO_OutputCtrl(EMIO_RF_CTLB_3, IO_VOL);
			}
		}
	}
	// T3新板划分为五个频段
	else if (get_device_type() == BOARD_TYPE_T3 || get_device_type() == BOARD_TYPE_T3_1)
	{
		// 机载Tracer专用  - SHRD104L - 有问题找周大卫
		if (ProductSwCfgGet().issupportairborne())
		{
			// LFCN-6000+：0.4~6GHz
			if ((param->freq > 2400) && (param->freq <= 2483))
			{
				if (channel != LFCN_6000)
				{
					channel = LFCN_6000;
					GPIO_OutputCtrl(EMIO_RF_CTLA_0, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLB_0, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLA_1, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLB_1, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLA_2, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLB_2, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLA_3, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLB_3, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_GPIO_0, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_1, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_2, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_3, IO_VOL);
					// printf("配置射频板和滤波器: channel = LFCN_6000\n");
				}
			}
			// BFCN-5750+: 5.65~5.85GHz
			else if ((param->freq > 5650) && (param->freq <= 5850))
			{
				if (channel != BFCN_5750)
				{
					channel = BFCN_5750;
					GPIO_OutputCtrl(EMIO_RF_CTLA_0, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLB_0, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLA_1, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLB_1, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLA_2, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLB_2, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLA_3, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLB_3, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_0, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_1, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_GPIO_2, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_GPIO_3, IO_VOH);
					// printf("配置射频板和滤波器: channel = BFCN_5750\n");
				}
			}
		}
		else	// 其他产品通用
		{
			// BFCN-4800+: 5.15~5.25GHz
			if ((param->freq > 5150) && (param->freq <= 5250))
			{
				if (channel != BFCN_4800)
				{
					channel = BFCN_4800;
					GPIO_OutputCtrl(EMIO_RF_CTLA_0, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLB_0, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLA_1, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLB_1, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLA_2, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLB_2, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLA_3, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLB_3, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_0, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_GPIO_1, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_2, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_3, IO_VOL);
					// printf("配置射频板和滤波器: channel = BFCN_4800\n");
				}
			}
			// TA1038A:    2.4~2.46GHz
			else if ((param->freq > 2400) && (param->freq <= 2460))
			{
				if (channel != TA1038A)
				{
					channel = TA1038A;
					GPIO_OutputCtrl(EMIO_RF_CTLA_0, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLB_0, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLA_1, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLB_1, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLA_2, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLB_2, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLA_3, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLB_3, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_0, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_GPIO_1, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_2, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_3, IO_VOH);
					// printf("配置射频板和滤波器: channel = TA1038A\n");
				}
			}
			// TA1059A:    2.45~2.51GHz
			else if ((param->freq > 2450) && (param->freq <= 2510))
			{
				if (channel != TA1059A)
				{
					channel = TA1059A;
					GPIO_OutputCtrl(EMIO_RF_CTLA_0, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLB_0, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLA_1, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLB_1, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLA_2, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLB_2, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLA_3, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLB_3, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_0, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_1, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_GPIO_2, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_GPIO_3, IO_VOL);
					// printf("配置射频板和滤波器: channel = TA1059A\n");
				}
			}
			// BFCN-5750+: 5.65~5.85GHz
			else if ((param->freq > 5650) && (param->freq <= 5850))
			{
				if (channel != BFCN_5750)
				{
					channel = BFCN_5750;
					GPIO_OutputCtrl(EMIO_RF_CTLA_0, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLB_0, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLA_1, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLB_1, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLA_2, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLB_2, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLA_3, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLB_3, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_0, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_1, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_GPIO_2, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_GPIO_3, IO_VOH);
					// printf("配置射频板和滤波器: channel = BFCN_5750\n");
				}
			}
			// LFCN-6000+：0.4~6GHz
			else if ((param->freq > 400) && (param->freq <= 6000))
			{
				if (channel != LFCN_6000)
				{
					channel = LFCN_6000;
					GPIO_OutputCtrl(EMIO_RF_CTLA_0, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLB_0, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLA_1, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLB_1, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_CTLA_2, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLB_2, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLA_3, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_CTLB_3, IO_VOH);
					GPIO_OutputCtrl(EMIO_RF_GPIO_0, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_1, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_2, IO_VOL);
					GPIO_OutputCtrl(EMIO_RF_GPIO_3, IO_VOL);
					// printf("配置射频板和滤波器: channel = LFCN_6000\n");
				}
			}
		}
	}
	// 配置AD9361
	SetRxFreq(param->freq);

	// 机载Tracer还需要设置天线阵列给PL寄存器
	if (ProductSwCfgGet().issupportairborne())
	{
		// 设置增益为0x37
		SetAd9361Gain(0x37, 0x37);
		// 2420、2460都使用 4: A3+A2天线，5745、5785、5825都使用6: B2+B1天线
		if (param->freq < 4000)
		{
			axi_write_data(PL_ADC_CUT_MODE, ANTENNA_SWITCH_MODE4);
			axi_write_data(PL_ADC_24G_58G_RX1, 0);
			axi_write_data(PL_ADC_24G_58G_RX2, 0);
		}
		else
		{
			axi_write_data(PL_ADC_CUT_MODE, ANTENNA_SWITCH_MODE6);
			axi_write_data(PL_ADC_24G_58G_RX1, 1);
			axi_write_data(PL_ADC_24G_58G_RX2, 1);
		}
	}
	RF_StartPLAlgByTimer();

	// 记录到本地，方便频谱导出数据
	_curFreq = param->freq;
	_curFreqIndex = param->freqIndex;

	//debug_print("CurFreqIndex=%d, CfgFreq=%0.1f\n", param->freqIndex, param->freq);

	return EAP_SUCCESS;
}

int TracerAdcHal::GetCurFreq(float *freq, int *index)
{
	if (freq)
		*freq = _curFreq;
	if (index)
		*index = _curFreqIndex;
	return EAP_SUCCESS;
}

int TracerAdcHal::_AdcSetGain(const Hal_Data& inData, Hal_Data& outData)
{
	uint32_t value = INDATA_U32(inData);
	SetAd9361Gain(value, value);
	return EAP_SUCCESS;
}

int TracerAdcHal::_AdcSetGainAgc(const Hal_Data& inData, Hal_Data& outData)
{
	uint32_t value = INDATA_U32(inData);
	SetAd9361GainAGC(value);
	return EAP_SUCCESS;
}

#define BURST_DATA_PAYLOAD_LEN(h)  (h->payload_uint64_items_count << 3)
#define BURST_DATA_FRAME_LEN(h) (sizeof(burst_data_font_header) + BURST_DATA_PAYLOAD_LEN(h) + sizeof(burst_data_tail_header_t))
#define BURST_DATA_TAIL_HEAD_OFFSET(h)  (sizeof(burst_data_font_header) + BURST_DATA_PAYLOAD_LEN(h))

/** 
 *     @brief 获取帧尾指针
 *     
 *     @param frame  帧头指针  
 *     @return    帧尾结构的指针
 *                NULL  简单校验出错，无法获取到帧尾
 *     @see    
 *     @note    在获取帧尾时，需要进行简单校验 
 */ 
static burst_data_tail_header_t *_DDRBurstDataGetFrameTailHeader(void *frame,uint32_t buf_len)
{
	burst_data_font_header *h = (burst_data_font_header *)frame;
	burst_data_tail_header *t = NULL;
	uint32_t frame_header_len = sizeof(burst_data_font_header) + sizeof(burst_data_tail_header);
//	DEBUG_BURST_DATA_PRINTF("***[%s:%d] start check,seq:%u, frame_len:0x%x,buf_len:0x%x,magic_head:0x%x,payload_uint64_items_count:0x%x\n",__FUNCTION__,__LINE__, h->frame_seq, BURST_DATA_FRAME_LEN(h),buf_len,h->magic_head,h->payload_uint64_items_count);
	if((BURST_DATA_FRAME_LEN(h) > buf_len) || h->magic_head != BURST_DATA_MAGIC_HEAD){
		DEBUG_BURST_DATA_PRINTF("***[%s:%d] err check,seq:%u, frame_len:0x%x,buf_len:0x%x,magic_head:0x%x,payload_uint64_items_count:0x%x\n",__FUNCTION__,__LINE__, h->frame_seq, BURST_DATA_FRAME_LEN(h),buf_len,h->magic_head,h->payload_uint64_items_count);
		return NULL;
	}
	t = (burst_data_tail_header *)(frame + BURST_DATA_TAIL_HEAD_OFFSET(h));

	if(t->magic_tail != BURST_DATA_MAGIC_TAIL || ((t->cnt1 - h->cnt0) != (BURST_DATA_PAYLOAD_LEN(h)>>3))){
		DEBUG_BURST_DATA_PRINTF("***[%s:%d] err check,seq:%u,cnt1:0x%lx,cnt0:0x%lx,magic_tail:0x%x\n",__FUNCTION__,__LINE__,h->frame_seq,t->cnt1,h->cnt0,t->magic_tail);
		return NULL;
	}
//	DEBUG_BURST_DATA_PRINTF("***[%s:%d] end: check,seq:%u,cnt1:0x%lx,cnt0:0x%lx,magic_tail:0x%x\n",__FUNCTION__,__LINE__,h->frame_seq,t->cnt1,h->cnt0,t->magic_tail);
	return t;
}

/** 
 *     @brief CRC校验帧数据
 *     
 *     @param h  帧头指针 
 *     @param t  帧尾指针 
 *     @return    校验结果
 *                0 通过；-1出错
 *     @see    
 *     @note    在CRC校验前，需要先进行简单校验，即保证帧头和帧尾基本正确。 
 */ 
static int _DDRBurstDataCrcCheck(burst_data_font_header *h,burst_data_tail_header *t)
{
	uint64_t *p = &h->cnt0;
	uint64_t crc = 0;
	int i;

	for(i = 0; i < ((BURST_DATA_FRAME_LEN(h) >> 3) - 1); i++){
		crc += (uint64_t)(*p);
		p++;
	}

	return (crc == t->crc)?0:-1;
}


void DroneId_DDRBurstDumpStat()
{
	burst_data_rcv_stat_t *s = &_sg_stat;

	printf(
		"******dronid burst data stat\n"
		"is_check_crc=%d\n"
		"rcv_count=%lu\n"
		"seq_start=%lu\n"
		"seq_now=%lu\n"
		"last_rcv_tick=%lu\n"
		"\n"
		"frame_err_count=%lu\n"
		"header_parse_fail_count=%lu\n"
		"data_crc_fail_count=%lu\n"
		"\n"
		"frame_lost_coun=%lu\n"
		"frame_rcv_min_tick=%lu\n"
		"frame_lost_max_step=%u\n"
		"\n"
		,s->is_check_crc
		,s->rcv_count
		,s->seq_start
		,s->seq_now
		,s->last_rcv_tick

		,s->frame_err_count
		,s->header_parse_fail_count
		,s->data_crc_fail_count

		,s->frame_lost_count
		,s->frame_rcv_min_tick
		,s->frame_lost_max_step

	);

}

static void _DDRBurst_intr_Handler(void *psArg)
{
	int ret;
	burst_data_irq_info_t *info = (burst_data_irq_info_t *)psArg;
	burst_data_front_header_t *h = (burst_data_front_header_t*)info->vir_addr;
	burst_data_tail_header_t *t = NULL;
	uint64_t cur_tick = eap_get_cur_time_ms();
	uint64_t pre_tick;
	uint64_t delta_tick;
	uint32_t frame_len;
	uint32_t seq_pre,seq_now;
	uint32_t seq_diff;

	static uint64_t pre_print_tick = 0;

	
	// 通知DroneID数据已存DDR
	debug_print("收到eDATA_PATH_DDR_BURST中断, 通知DroneID处理数据\r\n");
	
	simple_dcache_invalid_range((unsigned long long )info->vir_addr,(unsigned long long )info->vir_data_len);
	
	_sg_stat.rcv_count++;
	pre_tick = _sg_stat.last_rcv_tick;
	if(pre_tick){
		delta_tick = cur_tick - pre_tick;
		//更新接收时间最小间隔
		if(!_sg_stat.frame_rcv_min_tick){
			_sg_stat.frame_rcv_min_tick = delta_tick;
		}else if(delta_tick && delta_tick < _sg_stat.frame_rcv_min_tick){
			_sg_stat.frame_rcv_min_tick = delta_tick;
		}
	}
	_sg_stat.last_rcv_tick = cur_tick;
	
	//作简单报文检查
	t = _DDRBurstDataGetFrameTailHeader(h,info->vir_data_len);
	if(!t){
		_sg_stat.header_parse_fail_count++;
		_sg_stat.frame_err_count++;
		DEBUG_BURST_DATA_PRINTF("***[%s:%d] err check,seq:%u\n",__FUNCTION__,__LINE__, h->frame_seq);
#ifdef CFG_DEBUG_BURST_DATA
		eap_print_memory(h,sizeof(*h));
		DroneId_DDRBurstDumpStat();
#endif
		return;
	}

	frame_len = BURST_DATA_FRAME_LEN(h);
	
	//先拷过来，以防中断产生过快，导致数据被冲掉
	memcpy(info->bufferAdc_64bit, (uint64_t *)info->vir_addr, frame_len);

	//对拷过来的数据，重新作校验。如果确信此处memcpy不受cpu dcache影响，则此处检验可以省略。
	h = (burst_data_front_header_t*)info->bufferAdc_64bit;
	t = _DDRBurstDataGetFrameTailHeader(info->bufferAdc_64bit,frame_len);
	if(!t){
		_sg_stat.header_parse_fail_count++;
		_sg_stat.frame_err_count++;
		DEBUG_BURST_DATA_PRINTF("***[%s:%d] err check after copy,seq:%u\n",__FUNCTION__,__LINE__, h->frame_seq);
#ifdef CFG_DEBUG_BURST_DATA
		eap_print_memory(h,sizeof(*h));
		DroneId_DDRBurstDumpStat();
#endif
		return;
	}
	
	if(_sg_stat.is_check_crc){
		ret = _DDRBurstDataCrcCheck(h,t);
		if(ret<0){
			_sg_stat.data_crc_fail_count++;
			_sg_stat.frame_err_count++;
			DEBUG_BURST_DATA_PRINTF("***[%s:%d] err crc check,seq:%u\n",__FUNCTION__,__LINE__, h->frame_seq);
#ifdef CFG_DEBUG_BURST_DATA
			eap_print_memory(h,sizeof(*h));
			DroneId_DDRBurstDumpStat();
#endif
			return;
		}		
	}

/*
	//对burst_data接收统计信息
	typedef struct burst_data_rcv_stat
	{
		int 	 is_check_crc;
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
*/
	if(!_sg_stat.seq_start){
		_sg_stat.seq_start = h->frame_seq;
	}
	if(!_sg_stat.seq_now){
		_sg_stat.seq_now = h->frame_seq;
	}	
	
	seq_pre = _sg_stat.seq_now;
	seq_now = h->frame_seq;
	_sg_stat.seq_now = seq_now;
	
	if((seq_pre != seq_now) && (seq_pre + 1) != seq_now){
		seq_diff = seq_now - seq_pre;
		if(seq_diff < 0){
			//序号回绕了
			seq_diff = ((uint32_t)0xffffffff - seq_pre + 1 + seq_now);
		}
		if((seq_diff - 1) > _sg_stat.frame_lost_max_step){
			_sg_stat.frame_lost_max_step = seq_diff - 1; //assert(seq_diff > 1) 
		}
	}

	#if(DEBUG_PRINTF_BURST_DATA_STAT)
	if(_sg_stat.is_check_crc){
		if((cur_tick - pre_print_tick) > 3000){
			DroneId_DDRBurstDumpStat();
		}
	}
	#endif
	
	//通知droneid模块处理此帧。
	//FIXME:显然，此处采用共享一个buffer,还是可能造成数据在处理完成前被踩。
	burst_data_event event;
	memset(&event,0,sizeof(event));
	event.flag = eDATA_PATH_DDR_BURST;
	event.channel = info->channel;
	SendAsynData(EAP_CMD_DRONEID, (uint8_t*)&event, sizeof(event));
}

static void _tracer_adc_hal_init()
{
#ifdef NO_ADC_MODULE
		printf("\nNo RF board, AD9361 not initialized!\r\n");
#else
		if (access(FLAG_DEBUG_DRONEID_BURST_LOG, F_OK) == 0)
			flag_debug_droneid = 1;
		else
			flag_debug_droneid = 0;
	
		if( EapGetSys().GetWorkMode() == WORKMODE_DRONE)
		{
			if (access(FLAG_DEBUG_DRONEID_CHECK_CRC, F_OK) == 0)
				_sg_stat.is_check_crc = 1;
	
			simple_dcache_init();
			data_path_irq_init();
		}
#endif
}

void TracerAdcHal::Init()
{
	// 先初始化AD9361
	RfConfig_Init();

	if (ProductSwCfgGet().issupportdroneid())
	{
		// 再初始化共享内存和中断
		_InitBurstDataIrqInfoTable();
		burst_data_task_init_param_t param;
		memset(&param,0,sizeof(param));
		param.init = _tracer_adc_hal_init;
		param.info_counts = BURST_DATA_IRQ_INFO_TABLE_SIZE;
		param.info = _sg_burst_data_irq_info_table;

		data_path_burst_task(&param);
	}
}

TracerAdcHal::TracerAdcHal(HalMgr* halMgr) : HalBase(halMgr)
{
    _Register(EAP_DRVCODE_GET_DRONE_FREQ, (Hal_Func)&TracerAdcHal::_AdcGetFreqRegValue);
    _Register(EAP_DRVCODE_GET_DRONE_ADC, (Hal_Func)&TracerAdcHal::_AdcGetBurstData);
    _Register(EAP_DRVCODE_GET_DETECTION_PARAM, (Hal_Func)&TracerAdcHal::_AdcGetDetectionParam);
    _Register(EAP_DRVCODE_SET_DETECT_FREQ, (Hal_Func)&TracerAdcHal::_AdcSetDetectFreq);
    _Register(EAP_DRVCODE_SET_GAIN, (Hal_Func)&TracerAdcHal::_AdcSetGain);
    _Register(EAP_DRVCODE_SET_GAIN_AGC, (Hal_Func)&TracerAdcHal::_AdcSetGainAgc);
}

void TracerAdcHal::_InitBurstDataIrqInfoTable()
{
    /*********************************
    static burst_data_irq_info_t _sg_burst_data_irq_info_table[BURST_DATA_IRQ_INFO_TABLE_SIZE] = {
        {
            .is_enable = 1,
            .irq_dev_file = BURST_IRQ_DEV_NAME"_0",
            .phy_addr = (void*)0x60000000,
            .phy_data_len = VALID_ADC_BUF_LEN,		
            .channel = 0,
            .rcv_handler = _DDRBurst_intr_Handler,
        },
        {
            .is_enable = 0, //enable according to product sw config.
            .irq_dev_file = BURST_IRQ_DEV_NAME"_1",
            .phy_addr = (void*)0x61000000,
            .phy_data_len = VALID_ADC_BUF_LEN,		
            .channel = 1,
            .rcv_handler = _DDRBurst_intr_Handler,
        }
    };
    ******************************************/

#ifndef __UNITTEST__
    auto& lSwCfg = ProductSwCfgGet();
    BURST_DATA_IRQ_INFO_TABLE_SIZE = lSwCfg.burstdatairqinfolist_size();
    auto burst_data_table = new burst_data_irq_info_t[BURST_DATA_IRQ_INFO_TABLE_SIZE];
    for(int index = 0; index < BURST_DATA_IRQ_INFO_TABLE_SIZE; ++index)
    {
        const auto& pb_data_item = lSwCfg.burstdatairqinfolist(index);
        const auto channel_num = pb_data_item.channel();
        assert(channel_num == index);

        auto& busrt_data_info = burst_data_table[index];
        busrt_data_info.channel = pb_data_item.channel();
        busrt_data_info.is_enable = pb_data_item.isenabled();
        busrt_data_info.irq_dev_file = const_cast<char*>(pb_data_item.irqdevfilename().c_str());
        busrt_data_info.phy_addr = (void*)std::stoul(pb_data_item.phymemaddr(), nullptr, 16);
        busrt_data_info.phy_data_len = VALID_ADC_BUF_LEN;
        busrt_data_info.rcv_handler = _DDRBurst_intr_Handler;
    }
    _sg_burst_data_irq_info_table = burst_data_table;
#endif
}

TracerAdcHal::~TracerAdcHal()
{
    delete[] _sg_burst_data_irq_info_table;
    _sg_burst_data_irq_info_table = nullptr;
}
