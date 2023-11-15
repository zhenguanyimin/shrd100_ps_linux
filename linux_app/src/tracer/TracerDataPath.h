#pragma once

#ifndef __DATA_PATH_H__
#define __DATA_PATH_H__

/*========================================================================================
*                                     INCLUDE FILES
=========================================================================================*/
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include "HalBase.h"
#include "eap_pub.h"
#include "TracerCommonHal.h"
#include "hal/dma/libaxidma.h"
#include "hal/dma/axidma_ioctl.h"

/*========================================================================================
*                                      LOCAL MACROS
=========================================================================================*/

// 必须与设备树里的xlnx,device-id保持一致
#define FFT_TX_CHANNEL 0x3
#define FFT_RX_CHANNEL 0x4

#define FFT_DMA_PACKET_LEN (8 * 2048)
#define FFT_DMA_SIZE (2 * 2048)

#define FFT_DMA_TEST_FILE "/run/media/mmcblk0p1/fft_dma_test.data"

// 频谱侦测
#define DECT_DMA_TEST_FILE "/home/root/dect_dma_test.data"
#define POINT_PACK_NUM 2
#define DMA_PACK_NUM 60
#define NET_MAX_NUM 30
#define LINE_NUM 3000
#define COLUMN_NUM 128
#define REV_DET_PACK_LEN (COLUMN_NUM * LINE_NUM * 4 * POINT_PACK_NUM)

#define DET_RX_CHANNEL 0x2	// &axi_dma_others, device-id = <0x2>;
#define DET_DMA_PACKET_LEN (50 * COLUMN_NUM * 4 * POINT_PACK_NUM)
#define MAT_PACKET_LEN DET_DMA_PACKET_LEN / 2
#define MAX_NAME_LEN 32


#define NO_DATA 1
#define DIR_DATA 1
#define RFFT_DATA 2
#define WIFI_DATA 3
#define DET_DATA 4
#define FFT_DATA 5
#define DATA_TYPE_MAX 6

/*========================================================================================
*                       LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
=========================================================================================*/
enum
{
	BUFF_STAT_START,
	BUFF_EMPTY,
	BUFF_IN_USED,
	BUFF_FULL,
	BUFF_STAT_MAX
};

/*========================================================================================
*                                GLOBAL VARIABLE DECLARATIONS
=========================================================================================*/
class TracerDataPath : public HalBase
{
public:
    TracerDataPath(HalMgr* halMgr);
    ~TracerDataPath();
    void Init() override;

protected:
    int _DmaGetFftData(const Hal_Data& inData, Hal_Data& outData);
    int _DmaSpectrumDetData(const Hal_Data& inData, Hal_Data& outData);
    int _CollectData(const Hal_Data& inData, Hal_Data& outData);

private:

};

/*========================================================================================
*                                    FUNCTIONS
=========================================================================================*/

extern "C" {
typedef struct tracer_dma
{
	axidma_dev_t axidma_dev;
	const array_t *tx_chans;
	const array_t *rx_chans;
	char *fft_tx_buf;
	char *fft_rx_buf;
	char *det_rx_buf;
	int fft_tx_chan;
	int fft_rx_chan;
	int det_rx_chan;
} tracer_dma_t;

void FFTDmaTest();
void ClearDetDmaBuf(void);
void DmaSpectrumDetCb(int channel_id, void *data);
void DectDataExport(uint32_t framenum, uint32_t Freq);
void SetOutputSpecMat(uint8_t value);
uint8_t GetOutputSpecMat(void);
void data_path_buff_init(int type);
}
#endif
