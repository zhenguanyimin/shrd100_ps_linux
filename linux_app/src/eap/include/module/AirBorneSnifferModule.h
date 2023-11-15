#pragma once

#include "ModuleReal.h"
extern "C"
{
#include "../../../app/alg/det_alg/sfAirBorneSniffer.h"
}

#define FLAG_DEBUG_AIRBORNE_FILE "/run/media/mmcblk0p1/flag_debug_airborne"

#pragma pack(1)
// 机载测向输入参数定义
typedef struct AirBorne_param
{
	//IN: same to the function parameter
	unsigned short am1Mat[EAP_SPECTRUM_N][EAP_SPECTRUM_LINE_NUM][EAP_SPECTRUM_COLUMN_NUM];
	unsigned short am2Mat[EAP_SPECTRUM_N][EAP_SPECTRUM_LINE_NUM][EAP_SPECTRUM_COLUMN_NUM];
	unsigned short ph1Mat[EAP_SPECTRUM_N][EAP_SPECTRUM_LINE_NUM][EAP_SPECTRUM_COLUMN_NUM];
	unsigned short ph2Mat[EAP_SPECTRUM_N][EAP_SPECTRUM_LINE_NUM][EAP_SPECTRUM_COLUMN_NUM];

	float cenFreq;
	int antStatus;
	unsigned int mCnt;
	ioController_t inC;
	ioFlyStatus_t inF;
	CompensateData_t bcData;
} AirBorne_param_t;
#pragma pack()

#define AIRBORNE_PARAM_LEN            (sizeof(AirBorne_param_t))
#define AIRBORNE_RESULT_LEN           (sizeof(ioMeasureInfo_t))

class AirBorneSnifferModule : public ModuleReal
{
public:
    AirBorneSnifferModule(uint8_t coreId);
    uint8_t GetModuleId() override;
protected:
    ThreadBase* _CreateThread() override;
    CmdprocBase* _CreateCmdproc() override;
private:
};

