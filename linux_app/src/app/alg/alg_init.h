/*
 * alg_init.h
 *
 *  Created on: 2022-08-24
 *      Author: A19199
 */
#ifndef ALG_INIT_H
#define ALG_INIT_H
#include <stdio.h>
#include "../../inc/common_define.h"
#include "droneID/droneID_main.h"
#include "eap_pub.h"

#define FLAG_DEBUG_FFT_DMA "/run/media/mmcblk0p1/flag_debug_fft_dma"
#define FLAG_DEBUG_NO_FFT_DMA "/run/media/mmcblk0p1/flag_debug_no_fft_dma"
extern int flag_debug_fft_dma;

typedef int (*alg_dma_fft_cb_t)(float *g_x_sp, float *g_y_sp);
extern alg_dma_fft_cb_t m_dma_fft_cb;

void register_alg_dma_fft_cb(alg_dma_fft_cb_t func);
void alg_fft_dma_init();

void algorithm_init();
int32_t algorithm_Run(eDATA_PATH_INTR_FLAG flag, uint64_t *inputData, uint32_t dataLength, DroneID_ParseAllInfo_t *outList);
int32_t algorithm_RunNew(eDATA_PATH_INTR_FLAG flag, uint64_t *inputData, uint32_t dataLength, DroneID_ParseAllInfo_t *outList);

extern void SetAlgorithmStopFlag(uint8_t value);
extern uint8_t GetAlgorithmStopFlag();

#endif /* ALG_INIT_H */
