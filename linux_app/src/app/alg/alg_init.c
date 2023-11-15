#include "alg_init.h"
#include <unistd.h>
/*
 *initialize algorithm
 */
void algorithm_init()
{
	alg_fft_dma_init();
}

int32_t algorithm_Run(eDATA_PATH_INTR_FLAG flag, uint64_t *inputData, uint32_t dataLength, DroneID_ParseAllInfo_t *outList)
{
	return DroneID_main(flag, inputData, dataLength, outList);;
}

int32_t algorithm_RunNew(eDATA_PATH_INTR_FLAG flag, uint64_t *inputData, uint32_t dataLength, DroneID_ParseAllInfo_t *outList)
{
	return DroneID_mainNew(flag, inputData, dataLength, outList);;
}

uint8_t AlgorithmStopFlag = 0;
void SetAlgorithmStopFlag(uint8_t value)
{
	AlgorithmStopFlag = value;
}

uint8_t GetAlgorithmStopFlag()
{
	return AlgorithmStopFlag;
}

/* 注册PL硬件加速计算的回调函数供算法使用 */
alg_dma_fft_cb_t m_dma_fft_cb = NULL;
void register_alg_dma_fft_cb(alg_dma_fft_cb_t func)
{
	m_dma_fft_cb = func;
}

int flag_debug_fft_dma = 0;
void alg_fft_dma_init()
{
	// 临时修改关闭FFT加速，兼容新老版本
	if (access(FLAG_DEBUG_FFT_DMA, F_OK) == 0)
		flag_debug_fft_dma = 1;
	if (access(FLAG_DEBUG_NO_FFT_DMA, F_OK) == 0)
		flag_debug_fft_dma = 0;
}
