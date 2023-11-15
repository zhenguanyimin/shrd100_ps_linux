
#include "droneID_main.h"
#include "droneID_utils.h"
#include "extract_process_module.h"
#include "burst_process_module.h"
#include "testData/testData_get_process.h"
#include "../tracking/tracking.h"
#include "eap_pub.h"

/* global data declaration */
static const uint16_t g_packetType[] =
{
	0x1001,
	0x1002,
	0x1003,
};


extern float g_zcSnr[2];
extern float g_evm;
extern uint8_t g_decodeOut[LENGTH_DECODE_OUT_BYTE];
static DroneID_ParseAllInfo_t g_multiDroneIDList[MAX_NUM_DRONE_ID] = { 0 };
static DroneID_ParseAllInfo_t g_multiDroneIDListLast[MAX_NUM_DRONE_ID] = { 0 };
static DroneID_ParseAllInfo_t g_finalOutputList[MAX_NUM_DRONE_ID] = { 0 };
static DroneID_OutputInfoList_t g_DroneIdOutput;
static DroneID_OutputInfoList_t g_DroneIdOutputPredict;
static uint32_t g_multiDroneIDCnt = 0;

static struct timespec gCurFrameManageStartTic;	// unit = 1 tic
static int32_t gCurFrameManageStartTime = 0;	// unit = 1 ms
static struct timespec gCurFrameLogTic;	// unit = 1 tic
static int32_t gCurFrameLogTime = 0;	// unit = 1 ms

static uint64_t g_bufferAdcLastFrame[LENGTH_BURST_DATA] = { 0 };

#if EXTRACT_BURST_MODULE_EN
Cplxf_t g_fAdcBuff[LENGTH_ADC_DATA] = { { 0.0f } };
Cplxint32_t g_iAdcBuff[LENGTH_ADC_DATA] = { { 0 } };
//int32_t g_scoreBuff[LENGTH_ADC_DATA] = { 0 };
#endif	//EXTRACT_BURST_MODULE_EN

// static SemaphoreHandle_t  xDroneidSeamphore;
// void DroneidSeamphoreTake(void)
// {
// 	if (xDroneidSeamphore != NULL)
// 	{
// 		xSemaphoreTake(xDroneidSeamphore, 10000);
// 	}
// }

// void DroneidSeamphoreGive(void)
// {
// 	if (xDroneidSeamphore != NULL)
// 	{
// 		xSemaphoreGive(xDroneidSeamphore);
// 	}
// }

/* 请把要保存Burst数据的路径写入 /run/media/mmcblk0p1/flag_debug_droneid */
//eg: echo -n "/home/root/burst_30dB.log" > /run/media/mmcblk0p1/flag_debug_droneid
FILE *alg_open()
{
    static int record_droneid_log = -1;
    static char path[128];
    FILE *fp = NULL;

    if (record_droneid_log == -1)
    {
        fp =  fopen("/run/media/mmcblk0p1/flag_debug_droneid", "r");
        if (fp == NULL)
            record_droneid_log = 0;
        else if(fgets(path, 127, fp) != NULL)
        {
            int len = strlen(path);
            if (len > 0 && path[len-1] == '\n')
                path[len-1] = '\0';
            printf("Save the DroneID log to %s\n", path);
            record_droneid_log = 1;
        }
    }
    if (record_droneid_log == 1)
    {
        fp = fopen(path, "ab+");
        if (fp == NULL)
            printf("Failed to open %s\n", path);
    }

    return fp;
}

int alg_data(void *data, uint32_t dataLength)
{
    FILE *file = alg_open();

    if (file == NULL)
        return 1;

    fwrite("################", sizeof(unsigned char), 16, file);
    fwrite(data, sizeof(uint64_t), dataLength, file);
    fwrite("&&&&&&&&&&&&&&&&", sizeof(unsigned char), 16, file);
    fwrite("\n", sizeof(unsigned char), 1, file);
    fclose(file);

    return 0;
}

int alg_log(void *log, uint32_t length)
{
    FILE *file = alg_open();

    printf("%s", (char *)log);
    if (file == NULL)
        return 1;

    fwrite(log, sizeof(char), length, file);
    fclose(file);

    return 0;
}

/* get version of droneid algorithm */
const char *GetDroneidAlgVersion()
{
	return DRONEID_ALG_VERSION;
}


/* droneID process main */
int32_t DroneID_main(eDATA_PATH_INTR_FLAG flag, uint64_t *inputData, uint32_t dataLength, DroneID_ParseAllInfo_t *outList)
{
	struct timespec tCur1, tEnd1;
	int tUsed1 = 0;
	int32_t i = 0;
    int32_t numDroneID = 0;
    int32_t numCurDroneIDOutput = 0, numMultiDroneIDOutput = 0;
    DroneID_Signal_t droneID[MAX_NUM_DRONE_ID] = { 0 };
    DroneID_ParseInfo_t droneIDList[MAX_NUM_DRONE_ID] = { 0 };

    if (eDATA_PATH_DDR_BURST == flag)
    {
		/* Do burst processing for drone ID information */
		numDroneID = PadBurstProcessing(inputData, dataLength, droneID);
    }

	clock_gettime(CLOCK_MONOTONIC, &tCur1);

	clock_gettime(CLOCK_MONOTONIC, &gCurFrameLogTic);
	gCurFrameLogTime = (int32_t)((gCurFrameLogTic.tv_sec * 1000000000LL + gCurFrameLogTic.tv_nsec) / 1000000);

	/* Populate results into output list */
	if (numDroneID > 0)
	{
		numCurDroneIDOutput = OutputCurFrameDroneID(droneID, droneIDList, numDroneID);
		if (eDATA_PATH_DDR_BURST == flag)
			alg_data(inputData, dataLength);
	}

	numMultiDroneIDOutput = OutputMultiFrameDroneID(flag, numCurDroneIDOutput, droneIDList,
			g_multiDroneIDList, g_multiDroneIDListLast, g_finalOutputList);

	memcpy(&g_multiDroneIDListLast[0], &g_multiDroneIDList[0], sizeof(DroneID_ParseAllInfo_t) * MAX_NUM_DRONE_ID);

	// DroneidSeamphoreTake();

	for (i = 0; i < numMultiDroneIDOutput; i++)
	{
		memcpy(&outList[i], &g_finalOutputList[i], sizeof(DroneID_ParseAllInfo_t));
		memcpy(&g_DroneIdOutput.DroneIdOutputInfoList[i], &g_finalOutputList[i], sizeof(DroneID_ParseAllInfo_t));
		memcpy(&g_DroneIdOutputPredict.DroneIdOutputInfoList[i], &g_finalOutputList[i], sizeof(DroneID_ParseAllInfo_t));
	}
	g_DroneIdOutput.DroneIdNum = numMultiDroneIDOutput;
	g_DroneIdOutputPredict.DroneIdNum = numMultiDroneIDOutput;

	if (DRONEID_OUTPUT_LIST_PREDICT == GetDroneIDOutputListMode())
	{
		TrackingPredict_DroneID(DroneID_GetOutputListInfo(), DroneID_GetOutputListInfoPredict());
		for (i = 0; i < numMultiDroneIDOutput; i++)
		{
			memcpy(&outList[i], &g_DroneIdOutputPredict.DroneIdOutputInfoList[i], sizeof(DroneID_ParseAllInfo_t));
		}
	}

	// DroneidSeamphoreGive();

	clock_gettime(CLOCK_MONOTONIC, &tEnd1);
	tUsed1 = (tEnd1.tv_sec - tCur1.tv_sec) * 1000000 + (tEnd1.tv_nsec - tCur1.tv_nsec) / 1000;
	if (PRINT_BURST_PROCESS_TIME_COST == GetAlgDebugPrintMode())
	{
		printf("output process timeCost: %dus\r\n\n", tUsed1);
	}

	// printf("numMultiDroneIDOutput:%d, gCurFrameLogTime:%d, tv_sec:%d, tv_nsec:%d\r\n", 
	// numMultiDroneIDOutput, gCurFrameLogTime, gCurFrameLogTic.tv_sec, gCurFrameLogTic.tv_nsec);

	return numMultiDroneIDOutput;
}

/* droneID process main new */
int32_t DroneID_mainNew(eDATA_PATH_INTR_FLAG flag, uint64_t *inputData, uint32_t dataLength, DroneID_ParseAllInfo_t *outList)
{
	struct timespec tCur1, tEnd1;
	int tUsed1 = 0;
	int32_t i = 0;
    int32_t numDroneID = 0;
    int32_t numCurDroneIDOutput = 0, numMultiDroneIDOutput = 0;
    uint64_t adcCnt0Curr = 0, adcCnt0Last = 0;
    DroneID_Signal_t droneID[MAX_NUM_DRONE_ID] = { 0 };
    DroneID_ParseInfo_t droneIDList[MAX_NUM_DRONE_ID] = { 0 };
	char buffer[512];

    if (eDATA_PATH_DDR_BURST == flag)
    {
		/* Do burst processing for drone ID information */
    	adcCnt0Curr = inputData[0];
    	adcCnt0Last = g_bufferAdcLastFrame[0];
    	if (llabs(adcCnt0Curr - adcCnt0Last) > LENGTH_BURST_DATA)
    	{
    		numDroneID = PadBurstProcessingNew(inputData, dataLength, droneID);
    		memcpy(&g_bufferAdcLastFrame[0], inputData, sizeof(uint64_t) * dataLength);
			sprintf(buffer, "\r\nframe:%d, adcCh:%d\r\n", GetDroneIDProcFrameCnt(), ((g_bufferAdcLastFrame[2] & 0xFF000000) >> 24));
			alg_log(buffer, strlen(buffer));
    	}
    	else
    	{
    		memcpy(&g_bufferAdcLastFrame[0], inputData, sizeof(uint64_t) * dataLength);
    		goto out;
    	}
    }

	clock_gettime(CLOCK_MONOTONIC, &tCur1);

	clock_gettime(CLOCK_MONOTONIC, &gCurFrameLogTic);
	gCurFrameLogTime = (int32_t)((gCurFrameLogTic.tv_sec * 1000000000LL + gCurFrameLogTic.tv_nsec) / 1000000);

	/* Populate results into output list */
	if (numDroneID > 0)
	{
		numCurDroneIDOutput = OutputCurFrameDroneID(droneID, droneIDList, numDroneID);
		if (eDATA_PATH_DDR_BURST == flag)
			alg_data(inputData, dataLength);
	}

	numMultiDroneIDOutput = OutputMultiFrameDroneID(flag, numCurDroneIDOutput, droneIDList,
			g_multiDroneIDList, g_multiDroneIDListLast, g_finalOutputList);

	memcpy(&g_multiDroneIDListLast[0], &g_multiDroneIDList[0], sizeof(DroneID_ParseAllInfo_t) * MAX_NUM_DRONE_ID);

	// DroneidSeamphoreTake();

	for (i = 0; i < numMultiDroneIDOutput; i++)
	{
		memcpy(&outList[i], &g_finalOutputList[i], sizeof(DroneID_ParseAllInfo_t));
		memcpy(&g_DroneIdOutput.DroneIdOutputInfoList[i], &g_finalOutputList[i], sizeof(DroneID_ParseAllInfo_t));
		memcpy(&g_DroneIdOutputPredict.DroneIdOutputInfoList[i], &g_finalOutputList[i], sizeof(DroneID_ParseAllInfo_t));
	}
	g_DroneIdOutput.DroneIdNum = numMultiDroneIDOutput;
	g_DroneIdOutputPredict.DroneIdNum = numMultiDroneIDOutput;

	if (DRONEID_OUTPUT_LIST_PREDICT == GetDroneIDOutputListMode())
	{
		TrackingPredict_DroneID(DroneID_GetOutputListInfo(), DroneID_GetOutputListInfoPredict());
		for (i = 0; i < numMultiDroneIDOutput; i++)
		{
			memcpy(&outList[i], &g_DroneIdOutputPredict.DroneIdOutputInfoList[i], sizeof(DroneID_ParseAllInfo_t));
		}
	}

	// DroneidSeamphoreGive();

	clock_gettime(CLOCK_MONOTONIC, &tEnd1);
	tUsed1 = (tEnd1.tv_sec - tCur1.tv_sec) * 1000000 + (tEnd1.tv_nsec - tCur1.tv_nsec) / 1000;
	if (PRINT_BURST_PROCESS_TIME_COST == GetAlgDebugPrintMode())
	{
		printf("output process timeCost: %dus\r\n\n", tUsed1);
	}

out:
	return numMultiDroneIDOutput;
}



/* parse adc data */
void ParseAdcData(uint64_t *inputData, Cplxf_t *fAdcData, Cplxint32_t *iAdcData, uint32_t dataLength)
{
	int32_t i = 0, j = 0;
	int16_t real = 0, imag = 0;

	for (i = 0; i < dataLength; i++)
	{
		if (i % 2)
		{
			continue;
		}
		real = inputData[i] & ADC_REAL_MASK;
		imag = (inputData[i] >> ADC_IMAG_OFFSET) & ADC_IMAG_MASK;

		if (real > ADC_DATA_MAX)
		{
			real -= ADC_DATA_BIT_SCOPE;
		}
		if (real > 0)
		{
			iAdcData[j].re = 1;
		}
		else
		{
			iAdcData[j].re = -1;
		}
		fAdcData[j].re = (float)real;

		if (imag > ADC_DATA_MAX)
		{
			imag -= ADC_DATA_BIT_SCOPE;
		}
		if (imag > 0)
		{
			iAdcData[j].im = 1;
		}
		else
		{
			iAdcData[j].im = -1;
		}
		fAdcData[j].im = (float)imag;

		j++;
	}
}


/* transition drone type from number to string */
bool TransDroneType(uint8_t product_type_number, char *droneTypeOut)
{
	bool ret = true;
	char *drone_type;

	switch (product_type_number)
	{
		case 1:
			drone_type = "DJI Inspire 1";
			break;
		case 2:
			drone_type = "DJI Phantom 3";
			break;
		case 3:
			drone_type = "DJI Phantom 3 pro";
			break;
		case 4:
			drone_type = "DJI Phantom 3 Std";
			break;
		case 11:
			drone_type = "DJI Phantom 4";
			break;
		case 15:
			drone_type = "DJI Phantom 3 4k";
			break;
		case 16:
			drone_type = "DJI Mavic Pro";
			break;
		case 17:
			drone_type = "DJI Inspire 2";
			break;
		case 18:
			drone_type = "DJI Phantom 4 Pro";
			break;
		case 21:
			drone_type = "DJI Spark";
			break;			
		case 24:
			drone_type = "DJI Mavic Air";
			break;
		case 26:
			drone_type = "DJI Phantom 4 Series";
			break;
		case 27:
			drone_type = "DJI Phantom 4 Adv";
			break;
		case 35:
			drone_type = "DJI Phantom 4 RTK";
			break;
		case 36:
			drone_type = "DJI Phantom 4 Pro V2.0";
			break;
		case 41:
			drone_type = "DJI Mavic 2";
			break;
		case 51:
			drone_type = "DJI Mavic 2 Ent";
			break;
		case 53:
			drone_type = "DJI Mavic Mini";
			break;
		case 58:
			drone_type = "DJI Mavic Air2";
			break;
		case 61:
			drone_type = "DJI FPV";
			break;
		case 63:
			drone_type = "DJI Mini 2";
			break;
		case 66:
			drone_type = "DJI Air 2S";
			break;
		case 67:
			drone_type = "DJI M30";
			break;
		case 68:
			drone_type = "DJI Mavic 3";
			break;
		case 69:
			drone_type = "DJI Mavic 2 Ent Adv";
			break;
		case 73:
			drone_type = "DJI Mini 3 Pro";
			break;
		case 77:
			drone_type = "DJI Mavic 3E";
			break;
		case 84:
			drone_type = "DJI Mavic 3 Pro";
			break;
		case 86:
			drone_type = "DJI Mavic 3 Classic";
			break;
		case 90:
			drone_type = "DJI AIR3";
			break;
		case 93:
			drone_type = "DJI Mini 4 Pro";
			break;	
		default:
			drone_type = "unknown";
			ret = false;
			break;
	}

	strcpy(droneTypeOut, drone_type);
	return ret;
}


/* filter parse failed droneID info */
bool FilterParseFailedDroneID(DroneID_ParseInfo_t *pCurDroneID)
{
	bool droneInfoValidFlag = true;
	float height = 0.f;
	float yaw = 0.f;
	float speed = 0.f;
	float vSpeed = 0.f;
	float droneLong = 0.f, droneLat = 0.f;
	float pilotLong = 0.f, pilotLat = 0.f;

	height = pCurDroneID->height/1.e1,
	yaw = pCurDroneID->yaw_angle/1.e2,
	speed = pCurDroneID->speed/1.e2,
	vSpeed = pCurDroneID->z_speed/1.e2,
	droneLong = pCurDroneID->drone_longtitude/1.e7/PI_VALUE*180;
	droneLat = pCurDroneID->drone_latitude/1.e7/PI_VALUE*180,
	pilotLong = pCurDroneID->pilot_longitude/1.e7/PI_VALUE*180,
	pilotLat = pCurDroneID->pilot_latitude/1.e7/PI_VALUE*180;

	if ((fabsf(height) > 5000.0f) ||
		(fabsf(yaw) > 360.0f) ||
		(fabsf(speed) > 100.0f) ||
		(fabsf(vSpeed) > 100.0f) ||
		(fabsf(droneLong) > 180.0f) ||
		(fabsf(droneLat) > 90.0f) ||
		(fabsf(pilotLong) > 180.0f) ||
		(fabsf(pilotLat) > 90.0f))
	{
		droneInfoValidFlag = false;
	}

	return droneInfoValidFlag;
}


/* get first droneID */
int32_t GetFirstDroneID(DroneID_ParseInfo_t *pCurDroneID, DroneID_ParseAllInfo_t *pMultiDroneIDList)
{
	int32_t numDroneIDOutput = 0;
	bool droneTypeValidFlag = TransDroneType(pCurDroneID->type_num, (char *)&pCurDroneID->type_name[0]);

	if (droneTypeValidFlag)
	{
		memcpy(&pMultiDroneIDList[0].droneID, pCurDroneID, sizeof(DroneID_ParseInfo_t));
		pMultiDroneIDList[0].lostCnt = 0;
		pMultiDroneIDList[0].aliveCnt = 1;
		pMultiDroneIDList[0].lostDuration = 0;
		pMultiDroneIDList[0].lastUpdateTime = GetCurFrameManageStartTime();
		pMultiDroneIDList[0].removeThreLostDur = GetDroneIDRemoveThreLostDur();
		numDroneIDOutput = 1;
	}

	return numDroneIDOutput;
}

/* update less than all droneID */
void UpdateLessThanAllDroneID(DroneID_ParseInfo_t *pCurDroneID,
		DroneID_ParseAllInfo_t *pMultiDroneIDList, DroneID_ParseAllInfo_t *pMultiDroneIDListLast)
{
	int32_t i = 0;
	int32_t aliveIdx = g_multiDroneIDCnt;

	for (i = 0; i < g_multiDroneIDCnt; i++)
	{
		if (0 != strcmp((char *)pCurDroneID->serial_num, (char *)pMultiDroneIDList[i].droneID.serial_num))
		{
			memcpy(&pMultiDroneIDList[i], &pMultiDroneIDListLast[i], sizeof(DroneID_ParseAllInfo_t));
			pMultiDroneIDList[i].lostCnt++;
			pMultiDroneIDList[i].lostDuration = GetCurFrameManageStartTime() - pMultiDroneIDList[i].lastUpdateTime;
			pMultiDroneIDList[i].removeThreLostDur = GetDroneIDRemoveThreLostDur();
		}
		else
		{
			aliveIdx = i;
		}
	}

	memcpy(&pMultiDroneIDList[aliveIdx].droneID, pCurDroneID, sizeof(DroneID_ParseInfo_t));
	if (aliveIdx < g_multiDroneIDCnt)
	{
		pMultiDroneIDList[aliveIdx].lostCnt = 0;
		pMultiDroneIDList[aliveIdx].aliveCnt++;
	}
	else
	{
		pMultiDroneIDList[aliveIdx].lostCnt = 0;
		pMultiDroneIDList[aliveIdx].aliveCnt = 1;
		g_multiDroneIDCnt++;
	}
	pMultiDroneIDList[aliveIdx].lostDuration = 0;
	pMultiDroneIDList[aliveIdx].lastUpdateTime = GetCurFrameManageStartTime();
	pMultiDroneIDList[aliveIdx].removeThreLostDur = GetDroneIDRemoveThreLostDur();
}

/* update equal to all droneID */
void UpdateEqualToAllDroneID(DroneID_ParseInfo_t *pCurDroneID,
		DroneID_ParseAllInfo_t *pMultiDroneIDList, DroneID_ParseAllInfo_t *pMultiDroneIDListLast)
{
	int32_t i = 0;

	for (i = 0; i < g_multiDroneIDCnt; i++)
	{
		if (0 != strcmp((char *)pCurDroneID->serial_num, (char *)pMultiDroneIDList[i].droneID.serial_num))
		{
			memcpy(&pMultiDroneIDList[i], &pMultiDroneIDListLast[i], sizeof(DroneID_ParseAllInfo_t));
			pMultiDroneIDList[i].lostCnt++;
			pMultiDroneIDList[i].lostDuration = GetCurFrameManageStartTime() - pMultiDroneIDList[i].lastUpdateTime;
			pMultiDroneIDList[i].removeThreLostDur = GetDroneIDRemoveThreLostDur();
		}
		else
		{
			memcpy(&pMultiDroneIDList[i].droneID, pCurDroneID, sizeof(DroneID_ParseInfo_t));
			pMultiDroneIDList[i].lostCnt = 0;
			pMultiDroneIDList[i].aliveCnt++;
			pMultiDroneIDList[i].lostDuration = 0;
			pMultiDroneIDList[i].lastUpdateTime = GetCurFrameManageStartTime();
			pMultiDroneIDList[i].removeThreLostDur = GetDroneIDRemoveThreLostDur();
		}
	}
}

/* current frame no droneID process */
void CurFrameNoDroneIDProc(eDATA_PATH_INTR_FLAG flag, DroneID_ParseAllInfo_t *pMultiDroneIDList, DroneID_ParseAllInfo_t *pMultiDroneIDListLast)
{
	int32_t i = 0;
	for (i = 0; i < g_multiDroneIDCnt; i++)
	{
		memcpy(&pMultiDroneIDList[i], &pMultiDroneIDListLast[i], sizeof(DroneID_ParseAllInfo_t));
		pMultiDroneIDList[i].lostDuration = GetCurFrameManageStartTime() - pMultiDroneIDList[i].lastUpdateTime;
		if (eDATA_PATH_DDR_BURST == flag)
		{
			pMultiDroneIDList[i].lostCnt++;
			pMultiDroneIDList[i].removeThreLostDur = GetDroneIDRemoveThreLostDur();
		}
	}
}

/* current frame have droneID process */
void CurFrameHaveDroneIDProc(DroneID_ParseInfo_t *pCurDroneID,
		DroneID_ParseAllInfo_t *pMultiDroneIDList, DroneID_ParseAllInfo_t *pMultiDroneIDListLast)
{
	bool droneTypeValidFlag = true;
	bool droneInfoValidFlag = true;

	droneTypeValidFlag = TransDroneType(pCurDroneID->type_num, (char *)&pCurDroneID->type_name[0]);
	droneInfoValidFlag = FilterParseFailedDroneID(pCurDroneID);

	if (droneTypeValidFlag && droneInfoValidFlag)
	{
		if (g_multiDroneIDCnt == 0)
		{/* get first droneID */
			g_multiDroneIDCnt = GetFirstDroneID(pCurDroneID, pMultiDroneIDList);
		}
		else
		{
			if (g_multiDroneIDCnt < MAX_NUM_DRONE_ID)
			{/* update less than all droneID */
				UpdateLessThanAllDroneID(pCurDroneID, pMultiDroneIDList, pMultiDroneIDListLast);
			}
			else
			{/* update equal to all droneID */
				g_multiDroneIDCnt = MAX_NUM_DRONE_ID;
				UpdateEqualToAllDroneID(pCurDroneID, pMultiDroneIDList, pMultiDroneIDListLast);
			}
		}
	}
}

/* manage multiple droneID list */
int32_t ManageMultiDroneIDList(DroneID_ParseAllInfo_t *pMultiDroneIDList, DroneID_ParseAllInfo_t *pFinalOutputList)
{
	int32_t i = 0, finalOutputCnt = 0;
	int32_t multiDroneIDListCnt = 0;
	DroneID_ParseAllInfo_t multiDroneIDList[MAX_NUM_DRONE_ID] = { 0 };

	memset(&pFinalOutputList[0], 0, sizeof(DroneID_ParseAllInfo_t) * MAX_NUM_DRONE_ID);
	for (i = 0; i < g_multiDroneIDCnt; i++)
	{
		if (pMultiDroneIDList[i].lostDuration < pMultiDroneIDList[i].removeThreLostDur)
		{
			memcpy(&multiDroneIDList[multiDroneIDListCnt], &pMultiDroneIDList[i], sizeof(DroneID_ParseAllInfo_t));
			multiDroneIDListCnt++;

			memcpy(&pFinalOutputList[finalOutputCnt], &pMultiDroneIDList[i], sizeof(DroneID_ParseAllInfo_t));
			finalOutputCnt++;
		}
	}

	memset(&pMultiDroneIDList[0], 0, sizeof(DroneID_ParseAllInfo_t) * MAX_NUM_DRONE_ID);
	memcpy(&pMultiDroneIDList[0], &multiDroneIDList[0], sizeof(DroneID_ParseAllInfo_t) * multiDroneIDListCnt);
	g_multiDroneIDCnt = multiDroneIDListCnt;

	return finalOutputCnt;
}


/* output current frame droneID */
int32_t OutputCurFrameDroneID(DroneID_Signal_t *droneID, DroneID_ParseInfo_t *outlist, int32_t numDroneID)
{
	int32_t ii = 0, jj = 0;
	int32_t numDroneIDOutput = 1;
	bool droneTypeValidFlag = false;
	bool packetTypeValidFlag = false;
	bool DroneIdCrcCheckFlag = true;
	float speed = 0.0f;
	float droneLon = 0.0, droneLat = 0.0, droneH = 0.0;
	float pilotLon = 0.0, pilotLat = 0.0, pilotH = 0.0;
	float distance = 0.0;
	char buffer[512];

	for (ii = 0; ii < numDroneID; ii++)
    {
		outlist[ii].type_num = droneID[ii].product_type;
		droneTypeValidFlag = TransDroneType(outlist[ii].type_num, (char *)&outlist[ii].type_name[0]);

		for (jj = 0; jj < (sizeof(g_packetType) / sizeof(g_packetType[0])); jj++)
		{
			if (droneID[ii].packet_type == g_packetType[jj])
			{
				packetTypeValidFlag = true;
			}
		}

		memcpy((void *)&outlist[ii].serial_num[0],
			(void *)&droneID[ii].serial_num[0], 16*sizeof(uint8_t));

        outlist[ii].drone_longtitude   = droneID[ii].drone_longtitude;
        outlist[ii].drone_latitude     = droneID[ii].drone_latitude;
        outlist[ii].height             = droneID[ii].height;
        outlist[ii].yaw_angle          = droneID[ii].yaw_angle;

		speed = sqrtf(powf((float)droneID[ii].x_speed/1.e2, 2) +
			powf((float)droneID[ii].y_speed/1.e2, 2) + powf((float)droneID[ii].z_speed/1.e2, 2));

		outlist[ii].speed              = (int16_t) (speed * 1.e2);
		outlist[ii].x_speed            = droneID[ii].x_speed;
		outlist[ii].y_speed            = droneID[ii].y_speed;
        outlist[ii].z_speed            = droneID[ii].z_speed;
        outlist[ii].pilot_longitude    = droneID[ii].pilot_longitude;
        outlist[ii].pilot_latitude     = droneID[ii].pilot_latitude;
        outlist[ii].drone_Freq         = GetDroneIDSigFreqHz();

		if (DroneIdCrc24aTableCheck(g_decodeOut) == false)
		{
			numDroneIDOutput = 0;
			DroneIdCrcCheckFlag = false;
			g_droneIDCrcErrorCnt++;
		}

		if ((PRINT_DRONEID_RIGHT_ONLY == GetAlgDebugPrintMode()) ||
			(PRINT_DRONEID_RIGHT_AND_WRONG == GetAlgDebugPrintMode()) ||
			(PRINT_MULTI_DRONEID == GetAlgDebugPrintMode()))
		{
			droneLon = outlist[ii].drone_longtitude/1.e7/PI*180;
			droneLat = outlist[ii].drone_latitude/1.e7/PI*180;
			droneH = outlist[ii].height/1.e1;
			pilotLon = outlist[ii].pilot_longitude/1.e7/PI*180;
			pilotLat = outlist[ii].pilot_latitude/1.e7/PI*180;
			distance = GetDistance(droneLat, droneLon, droneH, pilotLat, pilotLon, pilotH);

			g_droneIDProcFrameCnt++;
			sprintf(buffer, "droneid info : type:%d(%s), sn:%s, dLng:%.7f, dLat:%.7f, h:%.2f, "
					"yaw:%.2f, spd:%.2f, spdx:%.2f, spdy:%.2f, spdz:%.2f, pLng:%.7f, pLat:%.7f, t:%d\r\n",
					outlist[ii].type_num,
					outlist[ii].type_name,
					outlist[ii].serial_num,
					droneLon,
					droneLat,
					droneH,
					outlist[ii].yaw_angle/1.e2,
					outlist[ii].speed/1.e2,
					outlist[ii].x_speed/1.e2,
					outlist[ii].y_speed/1.e2,
					outlist[ii].z_speed/1.e2,
					pilotLon,
					pilotLat,
					gCurFrameLogTime);
			alg_log(buffer, strlen(buffer));
			sprintf(buffer, "postproc info : dis:%.2f, zcSnr1:%.2f, zcSnr2:%.2f, evm:%.4f, freq:%.1f, crc:%d, errCnt:%d, errRate:%.3f\r\n",
					distance,
					g_zcSnr[0],
					g_zcSnr[1],
					g_evm,
					outlist[ii].drone_Freq/1.e6,
					DroneIdCrcCheckFlag,
					g_droneIDCrcErrorCnt,
					((GetDroneIDCrcErrorCnt() * 1.0f) / (GetDroneIDProcFrameCnt() * 1.0f)));
			alg_log(buffer, strlen(buffer));
		}
    }

	return numDroneIDOutput;
}


/* output multiple frame droneID */
int32_t OutputMultiFrameDroneID(eDATA_PATH_INTR_FLAG flag, uint32_t numCurDroneID, DroneID_ParseInfo_t *pCurDroneID,
		DroneID_ParseAllInfo_t *pMultiDroneIDList, DroneID_ParseAllInfo_t *pMultiDroneIDListLast,
		DroneID_ParseAllInfo_t *pFinalOutputList)
{
	int32_t i = 0;
	int32_t numDroneIDOutput = 0;
	float droneLon = 0.0, droneLat = 0.0, droneH = 0.0;
	float pilotLon = 0.0, pilotLat = 0.0, pilotH = 0.0;
	float distance = 0.0;

	clock_gettime(CLOCK_MONOTONIC, &gCurFrameManageStartTic);
	gCurFrameManageStartTime = (int32_t)((gCurFrameManageStartTic.tv_sec * 1000000000LL + gCurFrameManageStartTic.tv_nsec) / 1000000);

	if (0 == numCurDroneID)
	{/* current frame no droneID process */
		CurFrameNoDroneIDProc(flag, pMultiDroneIDList, pMultiDroneIDListLast);
	}
	else
	{/* current frame have droneID process */
		CurFrameHaveDroneIDProc(pCurDroneID, pMultiDroneIDList, pMultiDroneIDListLast);
	}

	/* manage multiple droneID list */
	numDroneIDOutput = ManageMultiDroneIDList(pMultiDroneIDList, pFinalOutputList);

	if ((PRINT_MULTI_DRONEID == GetAlgDebugPrintMode())||
		(PRINT_HEART_BEAT_DATA == GetAlgDebugPrintMode()))
	{
		for (i = 0; i < numDroneIDOutput; i++)
		{
			droneLon = pFinalOutputList[i].droneID.drone_longtitude/1.e7/PI*180;
			droneLat = pFinalOutputList[i].droneID.drone_latitude/1.e7/PI*180;
			droneH = pFinalOutputList[i].droneID.height/1.e1;
			pilotLon = pFinalOutputList[i].droneID.pilot_longitude/1.e7/PI*180;
			pilotLat = pFinalOutputList[i].droneID.pilot_latitude/1.e7/PI*180;
			distance = GetDistance(droneLat, droneLon, droneH, pilotLat, pilotLon, pilotH);

			printf("[%d] type:%d(%s), sn:%s, dLng:%.7f, dLat:%.7f, h:%.2f, "
					"yaw:%.2f, spd:%.2f, spdx:%.2f, spdy:%.2f, spdz:%.2f, pLng:%.7f, pLat:%.7f, t:%d"
					"dis:%.2f, zcSnr1:%.2f, zcSnr2:%.2f, evm:%.4f, freq:%.1f\r\n", i,
					pFinalOutputList[i].droneID.type_num,
					pFinalOutputList[i].droneID.type_name,
					pFinalOutputList[i].droneID.serial_num,
					droneLon,
					droneLat,
					droneH,
					pFinalOutputList[i].droneID.yaw_angle/1.e2,
					pFinalOutputList[i].droneID.speed/1.e2,
					pFinalOutputList[i].droneID.x_speed/1.e2,
					pFinalOutputList[i].droneID.y_speed/1.e2,
					pFinalOutputList[i].droneID.z_speed/1.e2,
					pilotLon,
					pilotLat,
					gCurFrameLogTime,
					distance,
					g_zcSnr[0],
					g_zcSnr[1],
					g_evm,
					pFinalOutputList[i].droneID.drone_Freq/1.e6);
		}
	}

	return numDroneIDOutput;
}

DroneID_OutputInfoList_t* DroneID_GetOutputListInfo(void)
{
	return &g_DroneIdOutput;
}

DroneID_OutputInfoList_t* DroneID_GetOutputListInfoPredict(void)
{
	return &g_DroneIdOutputPredict;
}

int32_t GetCurFrameManageStartTime(void)
{
	return gCurFrameManageStartTime;
}

