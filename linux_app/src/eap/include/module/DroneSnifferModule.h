#pragma once

#include "ModuleReal.h"
extern "C"
{
#include "../../../app/alg/det_alg/sfDroneSniffer.h"
}

#pragma pack(1)
typedef struct DroneSniffer_param
{
	//IN: same to the function parameter
	unsigned short specMat[EAP_SPECTRUM_N][EAP_SPECTRUM_LINE_NUM][EAP_SPECTRUM_COLUMN_NUM];
	unsigned short upMat[EAP_SPECTRUM_N][EAP_SPECTRUM_LINE_NUM][EAP_SPECTRUM_COLUMN_NUM];
	float cenFreq;
	float fs;
	float bw;
	float gain;
	int useRow;
	float SNR;
} DroneSniffer_param_t;
#pragma pack()

#define DRONEID_SNIFFER_PARAM_LEN     (sizeof(DroneSniffer_param_t))
#define DRONEID_SNIFFER_RESULT_LEN    (sizeof(uint16_t) + sizeof(SpectrumAlgResult)*EAP_DRONE_SNIFFER_NUM)

class DroneSnifferModule : public ModuleReal
{
public:
    DroneSnifferModule(uint8_t coreId);
    uint8_t GetModuleId() override;
protected:
    ThreadBase* _CreateThread() override;
    CmdprocBase* _CreateCmdproc() override;
private:
};

