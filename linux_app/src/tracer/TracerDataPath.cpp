#include "json_pb_converter.h"
#include "TracerDataPath.h"
#include "TracerAdcHal.h"
#include "TracerMainHalMgr.h"
#include "../srv/log/log.h"
#include "hal/dma/util.h"
#include "SysBase.h"
#include "HalMgr.h"

extern "C"
{
uint32_t buffer_aeag_[POINT_PACK_NUM][LINE_NUM][COLUMN_NUM];
uint32_t buffer_aeag1_[NET_MAX_NUM * POINT_PACK_NUM][LINE_NUM][COLUMN_NUM];
static Spectrum_DataPingPong sRdmBuf = {0};
uint8_t uOutputSpecMat = 0;
uint32_t RecordCenFreq[NET_MAX_NUM];
char CollectDataSuffixName[MAX_NAME_LEN] = {'\0'};
tracer_dma_t g_dma = {0};
}

int TracerDataPath::_DmaGetFftData(const Hal_Data& inData, Hal_Data& outData)
{
	float *g_x_sp = (float *)(inData.buf);
	float *g_y_sp = (float *)(outData.buf);
	uint32_t tx_len = inData.length;
	uint32_t rx_len = outData.length;
	int ret;

	// 校验数据长度
	if (tx_len != FFT_DMA_PACKET_LEN || rx_len != FFT_DMA_PACKET_LEN)
	{
		printf("Error: tx_len=%d, rx_len=%d, should be same as FFT_DMA_PACKET_LEN=%d\n", tx_len, rx_len, FFT_DMA_PACKET_LEN);
		return EAP_FAIL;
	}
	// 拷贝到fft_tx_buf
	if (g_dma.fft_tx_buf)
		memcpy(g_dma.fft_tx_buf, g_x_sp, tx_len);
	else
	{
		printf("Error: fft_tx_buf not init.\n");
		return EAP_SUCCESS;
	}
	// 开始传输数据
	ret = axidma_twoway_transfer(g_dma.axidma_dev, g_dma.fft_tx_chan, g_dma.fft_tx_buf,
		tx_len, NULL, g_dma.fft_rx_chan, g_dma.fft_rx_buf, rx_len, NULL, true);
	if (ret < 0)
	{
		memset(g_y_sp, 0, rx_len);
		printf("DMA transfer failed, error=%d!\n", ret);
	}
	else
	{
		memcpy(g_y_sp, g_dma.fft_rx_buf, rx_len);
		// printf("DMA transfer success!\n");
	}

	return EAP_SUCCESS;
}

int TracerDataPath::_DmaSpectrumDetData(const Hal_Data& inData, Hal_Data& outData)
{
	uint8_t startDMA = INDATA_U8(inData);
	int ret;

	printf("start dma: startDMA=%d\n", startDMA);
	if (startDMA == 0)
	{
		// 停止传输数据
		printf("%s,%d: Stop the DMA of spectrum.\n", __func__, __LINE__);
		axidma_stop_transfer(g_dma.axidma_dev, g_dma.det_rx_chan);
		RF_StopPLAlg();
	}
	else
	{
		// 开始传输数据
		printf("%s,%d: Start the DMA of spectrum.\n", __func__, __LINE__);
		RF_StopPLAlg();
		ret = axidma_oneway_transfer(g_dma.axidma_dev, g_dma.det_rx_chan, g_dma.det_rx_buf, DET_DMA_PACKET_LEN, false);
		if (ret < 0)
			printf("DMA transfer failed, error=%d!\n", ret);
		RF_StartPLAlgByTimer();
	}

	return EAP_SUCCESS;
}

int TracerDataPath::_CollectData(const Hal_Data& inData, Hal_Data& outData)
{
	uint8_t startDMA = inData.length;
	int ret;

	if (GetOutputSpecMat() > 0)
	{
		printf("%s,%d: 正在采集数据中...\n", __func__, __LINE__);
		return EAP_FAIL;
	}
	if (inData.length > 0)
	{
		// 设置采集数据后缀名
		snprintf(CollectDataSuffixName, MAX_NAME_LEN, "-%s", inData.buf);
		printf("%s,%d: CollectDataSuffixName=%s.\n", __func__, __LINE__, CollectDataSuffixName);
	}
	SetOutputSpecMat(1);

	return EAP_SUCCESS;
}

void TracerDataPath::Init()
{
    printf("\nInit DMA channels and buffer!\r\n");

	g_dma.axidma_dev = axidma_init();
	g_dma.tx_chans = axidma_get_dma_tx(g_dma.axidma_dev);
	g_dma.rx_chans = axidma_get_dma_rx(g_dma.axidma_dev);
	// Enable DMA GPIO
	axi_write(XPAR_AXI_GPIO_DMA_BASEADDR, 0x01);

	if( EapGetSys().GetWorkMode() == WORKMODE_DRONE)
	{
		//初始化FFT的BUFF
		g_dma.fft_tx_buf = (char *)axidma_malloc(g_dma.axidma_dev, FFT_DMA_PACKET_LEN);
		g_dma.fft_rx_buf = (char *)axidma_malloc(g_dma.axidma_dev, FFT_DMA_PACKET_LEN);
		// TODO: 应从配置文件读取参数
		g_dma.fft_rx_chan = FFT_RX_CHANNEL;
		g_dma.fft_tx_chan = FFT_TX_CHANNEL;
	}

	if( EapGetSys().GetWorkMode() != WORKMODE_DRONE)
	{
		//初始化频谱侦测的BUFF
		g_dma.det_rx_buf = (char *)axidma_malloc(g_dma.axidma_dev, REV_DET_PACK_LEN);
		// TODO: 应从配置文件读取参数
		if (ProductSwCfgGet().issupportspectrum())
			g_dma.det_rx_chan = DET_RX_CHANNEL;
		else if (ProductSwCfgGet().issupportairborne())
			g_dma.det_rx_chan = 1;	// 机载Tracer没有FFT,所以设备树里det_rx_chan变成了1
		data_path_buff_init(DET_DATA);
		// 设置DMA回调函数
		axidma_set_callback(g_dma.axidma_dev, g_dma.det_rx_chan, DmaSpectrumDetCb, g_dma.det_rx_buf);
	}
}

TracerDataPath::TracerDataPath(HalMgr* halMgr) : HalBase(halMgr)
{
	if( EapGetSys().GetWorkMode() == WORKMODE_DRONE)
	{
		_Register(EAP_DRVCODE_DMA_FFT_DATA, (Hal_Func)&TracerDataPath::_DmaGetFftData);
	}
	else
	{
		printf("\n_Register(EAP_DRVCODE_DMA_DET_DATA)!\r\n");
		_Register(EAP_DRVCODE_DMA_DET_DATA, (Hal_Func)&TracerDataPath::_DmaSpectrumDetData);
		_Register(EAP_DRVCODE_COLLECT_DATA, (Hal_Func)&TracerDataPath::_CollectData);
	}
}

TracerDataPath::~TracerDataPath()
{
	axidma_free(g_dma.axidma_dev, g_dma.det_rx_buf, REV_DET_PACK_LEN);
	axidma_free(g_dma.axidma_dev, g_dma.fft_tx_buf, FFT_DMA_PACKET_LEN);
	axidma_free(g_dma.axidma_dev, g_dma.fft_rx_buf, FFT_DMA_PACKET_LEN);
	axidma_destroy(g_dma.axidma_dev);
}

extern "C"
{

void data_path_buff_init(int type)
{
	if (DIR_DATA == type)
	{
	}
	else if (DET_DATA == type)
	{
		sRdmBuf.revLen = 0;
		sRdmBuf.buff1.flag = BUFF_EMPTY;
		sRdmBuf.buff1.length = REV_DET_PACK_LEN;
		sRdmBuf.buff1.type = DET_DATA;
		sRdmBuf.buff1.ts = 0;
		sRdmBuf.buff1.pData = (uint8_t *)buffer_aeag_;
	}
	else if (FFT_DATA == type)
	{
	}
}

void ClearDetDmaBuf(void)
{
	sRdmBuf.revLen = 0;
	sRdmBuf.buff1.flag = BUFF_EMPTY;
}

void SetOutputSpecMat(uint8_t value)
{
	// 支持连续抓包0-100次
	if (value < 0)
		value = 0;
	else if (value > 100)
		value = 100;
	uOutputSpecMat = value;
}

uint8_t GetOutputSpecMat(void)
{
	return uOutputSpecMat;
}

void ReduceOutputSpecMat(void)
{
	uOutputSpecMat--;
}

void DmaSpectrumDetCb(int channel_id, void *data)
{
	Spectrum_DataBuffer *pBuf = &sRdmBuf.buff1;
	Spectrum_DataPingPong *pDataPingPong = &sRdmBuf;
	uint8_t *pDetalgData = (uint8_t *)g_dma.det_rx_buf;
	uint32_t rx_len = DET_DMA_PACKET_LEN;
	static struct timespec StartTime, EndTime;
	uint32_t trans_time, process_time;
	uint32_t framenum = 0;
	float Freq;

	if (pBuf->flag == BUFF_FULL)
	{
		// printf("数据已满60包, 等待算法处理完毕!\n");
		sRdmBuf.revLen = 0;
	}
	else
	{
		if (sRdmBuf.revLen == 0)
		{
			// 计算等待算法处理时间
			clock_gettime(CLOCK_MONOTONIC, &StartTime);
			process_time = (int)((StartTime.tv_sec - EndTime.tv_sec)*1000 + (StartTime.tv_nsec - EndTime.tv_nsec)/1000000);
			// printf("等待算法处理时间: process_time=%d(ms)!\n", process_time);
		}
		sRdmBuf.revLen += rx_len;
		// printf("receive a DMA data! sRdmBuf.revLen=%d\n", sRdmBuf.revLen);
		if (REV_DET_PACK_LEN <= sRdmBuf.revLen)
		{
			sRdmBuf.revLen = 0;
			pBuf->flag = BUFF_FULL;
			for (uint8_t i = 0; i < DMA_PACK_NUM; i++)
			{
				memcpy(pBuf->pData + i * MAT_PACKET_LEN, pDetalgData + i * DET_DMA_PACKET_LEN, MAT_PACKET_LEN);
				memcpy(pBuf->pData + REV_DET_PACK_LEN / 2 + i * MAT_PACKET_LEN, pDetalgData + i * DET_DMA_PACKET_LEN + MAT_PACKET_LEN, MAT_PACKET_LEN);
			}

			// 2023-10-26 大卫反馈时频图顺序不对，需要相邻索引两两交换，需要找FPGA确认是否输出顺序有问题
			uint32_t *array = (uint32_t *)pBuf->pData;
			uint32_t temp;
			for (int j = 0; j < COLUMN_NUM * LINE_NUM; j=j+2)
			{
				temp = array[j];
				array[j] = array[j+1];
				array[j+1] = temp;
			}

			// 导出频谱侦测数据
			if (ProductDebugCfgGet().issupportspectrogram() && GetOutputSpecMat() > 0)
			{
				TracerMainHalMgr *halMgr = dynamic_cast<TracerMainHalMgr *>(EapGetSys().GetHalMgr());
				TracerAdcHal *adcHal = halMgr->GetAdcHal();
				if (nullptr != adcHal)
				{
					adcHal->GetCurFreq(&Freq, NULL);
					DectDataExport(framenum, (uint32_t)Freq);
				}
			}

			// 发送DMA数据给频谱模块
			SendAsynData(EAP_CMD_SPECTRUM, (uint8_t *)pDataPingPong, sizeof(Spectrum_DataPingPong));

			// 计算DMA传输时间
			clock_gettime(CLOCK_MONOTONIC, &EndTime);
			trans_time = (int)((EndTime.tv_sec - StartTime.tv_sec)*1000 + (EndTime.tv_nsec - StartTime.tv_nsec)/1000000);
			// printf("等待DMA传输时间: trans_time=%d(ms)!\n", trans_time);

			framenum++;
		}
	};
	axidma_oneway_transfer(g_dma.axidma_dev, g_dma.det_rx_chan, g_dma.det_rx_buf + sRdmBuf.revLen, DET_DMA_PACKET_LEN, false);

	return;
}

void DectDataExport(uint32_t framenum, uint32_t Freq)
{
	static uint8_t cntNet = 0;
	static int index = 0;
	char index_path[128];
	char log_path[128];
	FILE *fp = NULL;
	int ret;

	if (GetOutputSpecMat() > 0)
	{
		memcpy(&buffer_aeag1_[cntNet * 2][0][0], &buffer_aeag_[0][0][0], REV_DET_PACK_LEN / 2);
		memcpy(&buffer_aeag1_[cntNet * 2 + 1][0][0], &buffer_aeag_[1][0][0], REV_DET_PACK_LEN / 2);
		LOG_DEBUG("-------------- cnt: %d-------\r\n", cntNet);
		RecordCenFreq[cntNet] = Freq;
		cntNet++;
		if (cntNet >= NET_MAX_NUM)
		{
			if (GetOutputSpecMat() > 0)
			{
				// 更新索引
				snprintf(index_path, 128, "%s/%s", ProductDebugCfgGet().spectrogramattr().file_path().c_str(), ProductDebugCfgGet().spectrogramattr().file_index().c_str());
				fp = fopen(index_path, "r");
				if (fp)
				{
					ret = fscanf(fp, "%d", &index);
					fclose(fp);
					// 最多保留100份时频图文件
					if (ret <= 0 || index >= ProductDebugCfgGet().spectrogramattr().file_max_num())
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

				// 记录时频图
				snprintf(log_path, 128, "%s/%s%d%s.dat", ProductDebugCfgGet().spectrogramattr().file_path().c_str(), ProductDebugCfgGet().spectrogramattr().file_prefix().c_str(), index, CollectDataSuffixName);
				fp = fopen(log_path, "wb");
				if (fp)
				{
					fwrite("----------------", sizeof(unsigned char), 16, fp);
					fwrite(RecordCenFreq, sizeof(RecordCenFreq), 1, fp);
					fwrite("################", sizeof(unsigned char), 16, fp);
					for (int i=0; i<DMA_PACK_NUM; i++)
					{
						fwrite(&buffer_aeag1_[i][0][0], sizeof(uint32_t), COLUMN_NUM * LINE_NUM, fp);
					}
					fwrite("&&&&&&&&&&&&&&&&", sizeof(unsigned char), 16, fp);
					fclose(fp);
					EAP_LOG_DEBUG("记录频谱侦测数据%s成功!\n", log_path);
				}
				memset(CollectDataSuffixName, 0, sizeof(CollectDataSuffixName));
				ReduceOutputSpecMat();
			}
			cntNet = 0;
			memset(buffer_aeag1_, 0, sizeof(buffer_aeag1_));
		}
	}
}

void FFTDmaTest()
{
	TracerMainHalMgr *halMgr = dynamic_cast<TracerMainHalMgr *>(EapGetSys().GetHalMgr());
	float g_Fft_in_sp[FFT_DMA_SIZE] = { 0.0f };
	float g_Fft_out_sp[FFT_DMA_SIZE] = { 0.0f };
	Hal_Data inData = {FFT_DMA_PACKET_LEN, (uint8_t*)&g_Fft_in_sp};
	Hal_Data outData = {FFT_DMA_PACKET_LEN, (uint8_t*)&g_Fft_out_sp};
	int ret;

	if (nullptr != halMgr)
	{
		TracerDataPath *dataHal = halMgr->GetDataHal();
		if (nullptr != dataHal)
		{
			int fd = open(FFT_DMA_TEST_FILE, O_RDONLY);
			if (fd < 0)
			{
				printf("[%s:%d]Error opening input file %s\n",__FUNCTION__,__LINE__,FFT_DMA_TEST_FILE);
				return;
			}
			ret = robust_read(fd, (char *)g_Fft_in_sp, FFT_DMA_PACKET_LEN);
			if (ret < 0) {
				printf("[%s:%d]Unable to read in input buffer.\n",__FUNCTION__,__LINE__);
				return;
			}
			close(fd);
			ret = halMgr->OnGet(EAP_DRVCODE_DMA_FFT_DATA, inData, outData);
			printf("[%s:%d]FFT DMA test result: %d.\n",__FUNCTION__,__LINE__,ret);
		}
	}
}

}
