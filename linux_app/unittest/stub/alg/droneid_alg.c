#include <stdint.h>
#include <stdbool.h>
#include <memory.h>
#include "droneID_system_param.h"
#include "alg_init.h"

// extern DroneID_ParseAllInfo_t g_stubOutputList;
extern int g_stubOutputListNum;

void SetFreqOffsetHz(int32_t freqOffset)
{

}

void SetDroneIDSigFreqHz(uint64_t freq)
{
    
}

int32_t algorithm_Run(eDATA_PATH_INTR_FLAG flag, uint64_t *inputData, uint32_t dataLength, DroneID_ParseAllInfo_t *outList)
{
    // memcpy(outList, &g_stubOutputList, sizeof(DroneID_ParseAllInfo_t));
    return g_stubOutputListNum;
}

int32_t algorithm_RunNew(eDATA_PATH_INTR_FLAG flag, uint64_t *inputData, uint32_t dataLength, DroneID_ParseAllInfo_t *outList)
{
    // memcpy(outList, &g_stubOutputList, sizeof(DroneID_ParseAllInfo_t));
    return g_stubOutputListNum;
}

int32_t algorithm_Timer()
{
    return 0;
}

void SetAlgDebugPrintMode(int32_t printMode)
{
    
}

void SetDroneIDRemoveThre(int32_t threValue)
{
}

void SetDroneIDRemoveThreLostDur(int32_t threValue)
{
    
}

void SetDroneIDOutputSnMode(int32_t snMode)
{
}

void SetDroneIDTurboDecIterNum(int32_t iterNum)
{
    
}

void register_alg_dma_fft_cb(alg_dma_fft_cb_t func)
{

}

void alg_fft_dma_init()
{

}

void algorithm_init()
{

}
