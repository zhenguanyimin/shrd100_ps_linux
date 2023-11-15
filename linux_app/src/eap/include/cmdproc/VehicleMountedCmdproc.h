#include "CmdprocBase.h"

typedef enum {
	VEHICLE_MOUNTED_FIRING = 1,
	VEHICLE_MOUNTED_FIRED  = 2,
}FIRE_STATE_E;

enum {
	AUTO_FIRED = 1,
	CMD_FIRED  = 2,
	INVALID_CMD =3
};

class VehicleMountedCmdproc : public CmdprocBase
{
public:
	VehicleMountedCmdproc();
	~VehicleMountedCmdproc();

	void _VehicleMountedStartFiredAll(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
	void _VehicleMountedStopFiredAll(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
	void VehicleMountedStartFiredAll(void);

	void VehicleMountedSetFiredTime(uint8_t *firetime);
	void VehicleMountedGetFiredTime(uint8_t *firetime);

	void VehicleMountedSetFiredMode(uint8_t *fireMode);
	void VehicleMountedGetFiredMode(uint8_t *fireMode);

	void _VehicleMountedFiredAll(bool enable);

	void _VehicleMountedSetFiredirection(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
	void VehicleMountedAutoFire(SpectrumAlgResult *spect, int num);
	
	void _VehicleMountedNoticeFireStatus(FIRE_STATE_E state);

	void _VehicleMountedCmdFire(const Cmdproc_Data& inMsg);
	uint8_t _VehicleMountedGetSectorBydroneHorizon(uint32_t uHorizon);
	void _VehicleMountedEnableFired(uint8_t sectors);
	void _VehicleMountedDisableFired(uint8_t sectors);
	int _VehicleMountedOfflineRF(void);
	int _VehicleMountedOnlineRF(void);
protected:
	void _RegisterCmd() override;
private:
	uint8_t _uModeSelect;              // 1: auto fire;      2: cmd fire
	bool _isAllStrike;                 // 0: one direction ; 1: all direction
	uint32_t _uFiredTime;
};

