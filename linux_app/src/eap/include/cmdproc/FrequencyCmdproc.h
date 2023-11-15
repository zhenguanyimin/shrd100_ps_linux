#include "CmdprocBase.h"

typedef struct SYS_SampleParam
{
	/* Unit: KHz */
	uint32_t SampleFreq;
	/* Unit: KHz */
	uint32_t Band;
	/* Unit: dB */
	uint32_t Gain;
} SYS_SampleParam_t;

typedef struct SYS_AutoSweepParam
{
	/* Unit: MHz */
	uint32_t CurCenterFreq;

	/* Unit: MHz */
	uint32_t StartCenterFreq;
	/* Unit: MHz */
	uint32_t EndCenterFreq;
	/* Unit: MHz */
	uint32_t CenterFreqInc;
} SYS_AutoSweepParam_t;

typedef struct SYS_DefinedSweepParam
{
	uint32_t DefinedFreqValideNo;
	uint32_t DefinedFreqTotalNo;

	uint32_t CurFreqIndex;
	/* Unit: MHz */
	float DefinedFreq[40];
} SYS_DefinedSweepParam_t;

typedef struct SYS_DefinedFreqParam
{
	uint8_t DefinedFreqIndex;
	/* Unit: MHz */
	uint32_t DefinedFreq;
} SYS_DefinedFreqParam_t;

typedef struct SYS_Frequency_Param
{
	uint8_t DetectionMode;

	/* Unit: MHz */
	uint32_t CenterFreq;
	/* CenterFreq index */
	uint8_t CenterFreqItem;

	/* parameter for different Detection Mode */
	SYS_DefinedFreqParam_t DefinedFreqParam;
	SYS_AutoSweepParam_t AutoSweepParam;
	SYS_DefinedSweepParam_t DefinedSweepParam;

	SYS_SampleParam_t SampleParam;
}SYS_FrequencyParam_t ;

class FrequencyCmdproc : public CmdprocBase
{
public:
protected:
    void _RegisterCmd() override;
    void _OnFrequencyCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnSetFrequencyCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnSetFrequencyModeCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnChgFrequencyParamCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnShowFrequencyModuleCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
};
