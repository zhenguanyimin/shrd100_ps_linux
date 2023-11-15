#ifndef _DRONEID_MAIN_H_
#define _DRONEID_MAIN_H_

#include "droneID_system_param.h"
#include "eap_pub.h"

/*==================================================================================================
*                                      DEFINES AND MACROS
==================================================================================================*/
/***** Droneid algorithm version  *****
DRONEID_1.100000
        |--------- main droneid algorithm version number
          |------- version number of Data preprocess
           |------- version number of Sto, Cfo compensation
            |------- version number of Channel Estimation
             |------- version number of Mimo
              |------- version number of SoftDemod, Descrambler, Quantilization, TurboDecode
               |------- version number of Deframe
*/
#define DRONEID_ALG_VERSION 		"DRONEID_1.100000"

/*=================================================================================================
*                                STRUCTURES AND OTHER TYPEDEFS
=================================================================================================*/


/*==================================================================================================
*                                GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/


/*==================================================================================================
*                                    FUNCTION PROTOTYPES
==================================================================================================*/
const char *GetDroneidAlgVersion();
int32_t DroneID_main(eDATA_PATH_INTR_FLAG flag, uint64_t *inputData, uint32_t dataLength, DroneID_ParseAllInfo_t *outList);
int32_t DroneID_mainNew(eDATA_PATH_INTR_FLAG flag, uint64_t *inputData, uint32_t dataLength, DroneID_ParseAllInfo_t *outList);
void ParseAdcData(uint64_t *inputData, Cplxf_t *fAdcData, Cplxint32_t *iAdcData, uint32_t dataLength);
int32_t GetFirstDroneID(DroneID_ParseInfo_t *pCurDroneID, DroneID_ParseAllInfo_t *pMultiDroneIDList);
void UpdateLessThanAllDroneID(DroneID_ParseInfo_t *pCurDroneID,
		DroneID_ParseAllInfo_t *pMultiDroneIDList, DroneID_ParseAllInfo_t *pMultiDroneIDListLast);
void UpdateEqualToAllDroneID(DroneID_ParseInfo_t *pCurDroneID,
		DroneID_ParseAllInfo_t *pMultiDroneIDList, DroneID_ParseAllInfo_t *pMultiDroneIDListLast);
void CurFrameNoDroneIDProc(eDATA_PATH_INTR_FLAG flag, DroneID_ParseAllInfo_t *pMultiDroneIDList, DroneID_ParseAllInfo_t *pMultiDroneIDListLast);
void CurFrameHaveDroneIDProc(DroneID_ParseInfo_t *pCurDroneID,
		DroneID_ParseAllInfo_t *pMultiDroneIDList, DroneID_ParseAllInfo_t *pMultiDroneIDListLast);
int32_t ManageMultiDroneIDList(DroneID_ParseAllInfo_t *pMultiDroneIDList, DroneID_ParseAllInfo_t *pFinalOutputList);
int32_t OutputCurFrameDroneID(DroneID_Signal_t *droneID, DroneID_ParseInfo_t *outList, int32_t numDroneID);
int32_t OutputMultiFrameDroneID(eDATA_PATH_INTR_FLAG flag, uint32_t numCurDroneID, DroneID_ParseInfo_t *pCurDroneID,
		DroneID_ParseAllInfo_t *pMultiDroneIDList, DroneID_ParseAllInfo_t *pMultiDroneIDListLast,
		DroneID_ParseAllInfo_t *pFinalOutputList);
DroneID_OutputInfoList_t* DroneID_GetOutputListInfo(void);
DroneID_OutputInfoList_t* DroneID_GetOutputListInfoPredict(void);
int32_t GetCurFrameManageStartTime(void);

void DroneidSeamphoreTake(void);
void DroneidSeamphoreGive(void);

#endif /* _DRONEID_MAIN_H_ */
