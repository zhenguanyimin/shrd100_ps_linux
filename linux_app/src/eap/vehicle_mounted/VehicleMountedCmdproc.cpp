#include "AlinkCmdproc.h"
#include "HalMgr.h"
#include "SysBase.h"
#include "VehicleMountedCmdproc.h"
#include "../../tracer/hal/gpio/gpioapp.h"
#include "eap_pub.h"

#if 0
#define DEBUG_VEHICLEMOUNTED_PRINTF(arg...) printf(arg)
#else
#define DEBUG_VEHICLEMOUNTED_PRINTF(arg...) 
#endif

#define VEHICLE_MOUNTED_PROTO_DATA_LENG 12

#define ANGLE_30   30
#define ANGLE_90   90
#define ANGLE_150  150
#define ANGLE_210  210
#define ANGLE_270  270
#define ANGLE_330  330
#define ANGLE_360  360

#define DEVIATION_ANGLE 5

#define NOTE_TO_C2_M 2

typedef struct gpio_info{
	uint16_t name;
	uint8_t direction;
	uint8_t value;
}GPIO_S;

VehicleMountedCmdproc::VehicleMountedCmdproc()
{
	_uModeSelect = AUTO_FIRED;
	_isAllStrike = 0;
	_uFiredTime = 60;
}

VehicleMountedCmdproc::~VehicleMountedCmdproc()
{
}

void VehicleMountedCmdproc::_RegisterCmd()
{
	Register(EAP_CMD_SET_VEHICLE_MOUNTED_START_FIRED_ALL, (Cmdproc_Func)&VehicleMountedCmdproc::_VehicleMountedStartFiredAll);
	Register(EAP_CMD_SET_VEHICLE_MOUNTED_STOP_FIRED_ALL, (Cmdproc_Func)&VehicleMountedCmdproc::_VehicleMountedStopFiredAll);
}

void VehicleMountedCmdproc::_VehicleMountedNoticeFireStatus(FIRE_STATE_E state)
{
	//EAP_LOG_DEBUG("stat = %d\n", state);
	alink_msg_t msg = {0};
	msg.channelType = EAP_CHANNEL_TYPE_TCP | EAP_CHANNEL_TYPE_TYPEC;
	msg.channelId   = UINT16_ALL;
	memcpy(msg.buffer, EapGetSys().GetSnName(), EAP_SN_LENGTH);
	msg.buffer[EAP_SN_LENGTH] = state;
	AlinkCmdproc::SendAlinkCmd(msg, EAP_SN_LENGTH + 1, EAP_ALINK_RPT_TRACER_FIRE_STATE);
}


void VehicleMountedCmdproc::VehicleMountedAutoFire(SpectrumAlgResult *spect, int num)
{
	int ret = 0;
	int i;
	uint8_t sectors = 0;

	SpectrumAlgResult *pspect = spect;
	
	if(AUTO_FIRED != _uModeSelect){
		/* the message is err, this is cmd fired */
	    EAP_LOG_ERROR("err, _uModeSelect = 0x%x, is not auto fired!\n", _uModeSelect);
		return ;
	}

	if(!pspect){
		EAP_LOG_ERROR("err, pspect is NULL!\n");
		return ;
	}

#if 0
	/* fire the first drone */
	pspect = (SpectrumAlgResult *)pmsg->buffer;
	if(!pspect){
		printf("[%s, %d]err, pspect is NULL!\n", __func__, __LINE__);
		return ;
	}

	pspect->range = 40;
	printf("[%s, %d]pspect->freq[0] = %f\n", __func__, __LINE__, pspect->freq[0]);
	printf("[%s, %d]pspect->name = %s\n", __func__, __LINE__, pspect->name);

	sectors = _VehicleMountedGetSectorBydroneHorizon(pspect->range);

	/* close rf */
	ret = _VehicleMountedOfflineRF();
	if(ret < 0){
		printf("[%s, %d] err, close rf failed!\n", __func__, __LINE__);
		return ;
	}

	/* fired */
	printf("[%s, %d]sectors = %d\n", __func__, __LINE__, sectors);
	_VehicleMountedEnableFired(sectors);
	
	/* send fired mesg to c2 */
	for(i=0; i<40; i++){
		_VehicleMountedNoticeFireStatus(VEHICLE_MOUNTED_FIRING);
		sleep(1);
	}

	/*disable fired */
	_VehicleMountedDisableFired(sectors);

	/* send fired finish mesg to c2 */
	_VehicleMountedNoticeFireStatus(VEHICLE_MOUNTED_FIRED);
	
	/* restart rf */
	ret = _VehicleMountedOnlineRF();
	if(ret < 0){
		printf("[%s, %d] err, close rf failed!\n", __func__, __LINE__);
		return ;
	}
#else
	VehicleMountedStartFiredAll();


#endif

	return ;
}

uint8_t VehicleMountedCmdproc::_VehicleMountedGetSectorBydroneHorizon(uint32_t uHorizon)
{
	int sector = 0;

	//EAP_LOG_DEBUG("%s, %d, uHorizon = %d\n", uHorizon);
	
	if(uHorizon > ANGLE_360){
		EAP_LOG_ERROR("uHorizon = %d, big than 360, err!\n", uHorizon);
		sector = 0 ;
		return sector;
	}

	/* less 25, big than 335, setctor set the 5 bit */
	if((uHorizon < (ANGLE_30-DEVIATION_ANGLE)) || (uHorizon > (ANGLE_330+DEVIATION_ANGLE))){
		sector |= 1 << 5; 
		return sector;
	}

	/* big than 25, less 35 */
	if((uHorizon >= (ANGLE_30-DEVIATION_ANGLE)) && (uHorizon <= (ANGLE_30+DEVIATION_ANGLE))){
		sector |= 1 << 5;
		sector |= 1 << 6;
		return sector;
	}

	/* */
	if((uHorizon > (ANGLE_30+DEVIATION_ANGLE)) && (uHorizon < (ANGLE_90-DEVIATION_ANGLE))){
		sector |= 1 << 6;
		return sector;
	}
	
	if((uHorizon >= (ANGLE_90-DEVIATION_ANGLE)) && (uHorizon <= (ANGLE_90+DEVIATION_ANGLE))){
		sector |= 1 << 1;
		sector |= 1 << 6;
		return sector;
	}

	if((uHorizon > (ANGLE_90+DEVIATION_ANGLE)) && (uHorizon < (ANGLE_150-DEVIATION_ANGLE))){
		sector |= 1 << 1;
		return sector;
	}

	if((uHorizon >= (ANGLE_150-DEVIATION_ANGLE)) && (uHorizon <= (ANGLE_150+DEVIATION_ANGLE))){
		sector |= 1 << 1;
		sector |= 1 << 2;
	    return sector;
	}

	if((uHorizon > (ANGLE_150+DEVIATION_ANGLE)) && (uHorizon < (ANGLE_210-DEVIATION_ANGLE))){
		sector |= 1 << 2;
		return sector;
	}
	
	if((uHorizon >= (ANGLE_210-DEVIATION_ANGLE)) && (uHorizon <= (ANGLE_210+DEVIATION_ANGLE))){
		sector |= 1 << 2;
		sector |= 1 << 3;
		return sector;
	}

	if((uHorizon > (ANGLE_210+DEVIATION_ANGLE)) && (uHorizon < (ANGLE_270-DEVIATION_ANGLE))){
		sector |= 1 << 3;
		return sector;
	}

	if((uHorizon >= (ANGLE_270-DEVIATION_ANGLE)) && (uHorizon <= (ANGLE_270+DEVIATION_ANGLE))){
		sector |= 1 << 3;
		sector |= 1 << 4;
		return sector;
	}

	if((uHorizon > (ANGLE_270+DEVIATION_ANGLE)) && (uHorizon < (ANGLE_330-DEVIATION_ANGLE))){
		sector |= 1 << 4;
		return sector;
	}

	if((uHorizon >= (ANGLE_330-DEVIATION_ANGLE)) && (uHorizon <= (ANGLE_330+DEVIATION_ANGLE))){
		sector |= 1 << 4;
		sector |= 1 << 5;
		return sector;
	}
	
}

int VehicleMountedCmdproc::_VehicleMountedOfflineRF(void)
{
	HalMgr* halMgr = NULL;
	Hal_Data inData = {0, 0};
	Hal_Data outData = {0, 0};

	/* rf */
	uint8_t disable_rf = 0;
	uint8_t rf_status = 0;

	halMgr = EapGetSys().GetHalMgr();
	if(!halMgr){
		EAP_LOG_ERROR("err, get halMgr failed!\n");
		return -1;
	}
	
	/* disable rf */
	inData.length = 1;
	inData.buf = &disable_rf;
	halMgr->OnSet(EAP_DRVCODE_SET_RF_ENABLE, inData);
	
	/* get rf status, rf closed */
	outData.length = 1;
	outData.buf = &rf_status;

	/* return 1, closed ok*/
	halMgr->OnGet(EAP_DRVCODE_GET_RF_ENABLE, inData, outData);
	if(0 != rf_status){
		/* close rf failed */
		return -1;
	}
	return 0;
}

int VehicleMountedCmdproc::_VehicleMountedOnlineRF(void)
{
	HalMgr* halMgr = NULL;
	Hal_Data inData = {0, 0};
	Hal_Data outData = {0, 0};

	/* rf */
	uint8_t enable_rf = 1;
	uint8_t rf_status = 0;

	halMgr = EapGetSys().GetHalMgr();
	if(!halMgr){
		printf("[%s, %d]err, get halMgr failed!\n", __func__, __LINE__);
		return -1;
	}
	
	/* eable rf */
	inData.length = 1;
	inData.buf = &enable_rf;
	halMgr->OnSet(EAP_DRVCODE_SET_RF_ENABLE, inData);
	
	/* get rf status, rf closed */
	outData.length = 1;
	outData.buf = &rf_status;

	
	halMgr->OnGet(EAP_DRVCODE_GET_RF_ENABLE, inData, outData);
	if(!rf_status){
		/* enable rf failed */
		EAP_LOG_ERROR("restart rf failed\n");
		return -1;
	}
	return 0;
}

void VehicleMountedCmdproc::_VehicleMountedEnableFired(uint8_t sectors)
{
	int i=0;
	GPIO_S gpio_ctrl;
	HalMgr* halMgr = NULL;
	Hal_Data inData = {0, 0};
	Hal_Data outData = {0, 0};
	
	EAP_LOG_DEBUG("sectors = %d\n", sectors);

	if(!sectors){
		EAP_LOG_ERROR("err, sectors is 0!\n");
		return ;
	}
	
	/* set fire gpio */
	for(i=1; i<7; i++){
		if(sectors & (1 << i)){
			GPIO_S gpio_ctrl;
			EAP_LOG_DEBUG("[%s, %d] sector = %d\n", __func__, __LINE__, i);
			switch(i)
			{
				case 1:
					gpio_ctrl.name = MIO_SWITCH_CHANNEL_0;
					break;
				case 2:
					gpio_ctrl.name = MIO_SWITCH_CHANNEL_1;
					break;
				case 3:
					gpio_ctrl.name = MIO_SWITCH_CHANNEL_2;
					break;
				case 4:
					gpio_ctrl.name = MIO_SWITCH_CHANNEL_3;
					break;
				case 5:
					gpio_ctrl.name = EMIO_SWITCH_CHANNEL_4;
					break;
				case 6:
					gpio_ctrl.name = MIO_SWITCH_CHANNEL_5;
					break;
				default:
					EAP_LOG_ERROR("err!\n");
					break;
			};
			gpio_ctrl.direction = OUTPUT; 
			gpio_ctrl.value = 1;
			inData.length = sizeof(GPIO_S);
			inData.buf = (uint8_t *)&gpio_ctrl;

			halMgr = EapGetSys().GetHalMgr();
			if(!halMgr){
				EAP_LOG_ERROR("err, get halMgr failed!\n");
				return ;
			}
			halMgr->OnSet(EAP_DRVCODE_SET_GPIO_VALUE, inData);
		}
	}
	return ;
}

void VehicleMountedCmdproc::_VehicleMountedFiredAll(bool enable)
{
	int i=0;
	GPIO_S gpio_ctrl[6] = {0};
	HalMgr* halMgr = NULL;
	Hal_Data inData = {0, 0};
	
	EAP_LOG_DEBUG("enable = %d\n", enable);
	
	/* set fired all gpio */
	for(i=0; i<6; i++){
		gpio_ctrl[i].direction = OUTPUT; 
		gpio_ctrl[i].value = enable;
	}
	
	gpio_ctrl[0].name = MIO_SWITCH_CHANNEL_0;
	gpio_ctrl[1].name = MIO_SWITCH_CHANNEL_1;
	gpio_ctrl[2].name = MIO_SWITCH_CHANNEL_2;
	gpio_ctrl[3].name = MIO_SWITCH_CHANNEL_3;
	gpio_ctrl[4].name = EMIO_SWITCH_CHANNEL_4;
	gpio_ctrl[5].name = MIO_SWITCH_CHANNEL_5;

	inData.length = sizeof(gpio_ctrl);
	inData.buf = (uint8_t *)gpio_ctrl;

	halMgr = EapGetSys().GetHalMgr();
	if(!halMgr){
		EAP_LOG_DEBUG("err, get halMgr failed!\n");
		return ;
	}
	halMgr->OnSet(EAP_DRVCODE_SET_GPIO_VALUE, inData);
	return ;
}


void VehicleMountedCmdproc::_VehicleMountedDisableFired(uint8_t sectors)
{
	int i=0;
	GPIO_S gpio_ctrl;
	HalMgr* halMgr = NULL;
	Hal_Data inData = {0, 0};
	Hal_Data outData = {0, 0};
	
	EAP_LOG_DEBUG("sectors = %d\n", sectors);

	if(!sectors){
		EAP_LOG_ERROR("err, sectors is 0!\n");
		return ;
	}
	
	/* set fire gpio */
	for(i=1; i<7; i++){
		if(sectors & (1 << i)){
			GPIO_S gpio_ctrl;
			switch(i)
			{
				case 1:
					gpio_ctrl.name = MIO_SWITCH_CHANNEL_0;
					break;
				case 2:
					gpio_ctrl.name = MIO_SWITCH_CHANNEL_1;
					break;
				case 3:
					gpio_ctrl.name = MIO_SWITCH_CHANNEL_2;
					break;
				case 4:
					gpio_ctrl.name = MIO_SWITCH_CHANNEL_3;
					break;
				case 5:
					gpio_ctrl.name = EMIO_SWITCH_CHANNEL_4;
					break;
				case 6:
					gpio_ctrl.name = MIO_SWITCH_CHANNEL_5;
					break;
				default:
					EAP_LOG_ERROR("err!\n");
					break;
			};
			gpio_ctrl.direction = OUTPUT; 
			gpio_ctrl.value = 0;
			inData.length = sizeof(GPIO_S);
			inData.buf = (uint8_t *)&gpio_ctrl;

			halMgr = EapGetSys().GetHalMgr();
			if(!halMgr){
				EAP_LOG_ERROR("err, get halMgr failed!\n");
				return ;
			}
			halMgr->OnSet(EAP_DRVCODE_SET_GPIO_VALUE, inData);
		}
	}
	return ;
}

void VehicleMountedCmdproc::VehicleMountedStartFiredAll(void)
{
	int i;
	int ret = 0;
	/* stop rf app */

	if(SYS_HIT_TARGET_STATUS == EapGetSys().GetSysStatus()){
		EAP_LOG_DEBUG("is hitting now!\n");
		return;
	}
	
	/* close rf */
	ret = _VehicleMountedOfflineRF();
	if(ret < 0){
		EAP_LOG_ERROR("err, close rf failed!\n");
		return ;
	}

	EAP_LOG_DEBUG("start to fire all drone\n");
	/* fired */
	_VehicleMountedFiredAll(true);
	
	/* send fired mesg to c2 */
	for(i=0; i<_uFiredTime/NOTE_TO_C2_M; i++){
		_VehicleMountedNoticeFireStatus(VEHICLE_MOUNTED_FIRING);
		sleep(NOTE_TO_C2_M);
	}

	/*disable fired */
	_VehicleMountedFiredAll(false);

	EAP_LOG_DEBUG("stop fire all drone\n");
	
	/* send fired finish mesg to c2 */
	_VehicleMountedNoticeFireStatus(VEHICLE_MOUNTED_FIRED);
	
	/* restart rf */
	ret = _VehicleMountedOnlineRF();
	if(ret < 0){
		EAP_LOG_ERROR("err, close rf failed!\n");
		return ;
	}

	/* start rf app */
	
	return ;
	
}


void VehicleMountedCmdproc::_VehicleMountedStartFiredAll(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
	int i;
	int ret = 0;

	/* stop rf app */

	if(SYS_HIT_TARGET_STATUS == EapGetSys().GetSysStatus()){
		EAP_LOG_DEBUG("is hitting now!\n");
		return;
	}
	
	/* close rf */
	ret = _VehicleMountedOfflineRF();
	if(ret < 0){
		EAP_LOG_ERROR("err, close rf failed!\n");
		return ;
	}

	EAP_LOG_DEBUG("start to fire all drone\n");
	/* fired */
	_VehicleMountedFiredAll(true);
	
	/* send fired mesg to c2 */
	for(i=0; i<_uFiredTime/NOTE_TO_C2_M; i++){
		_VehicleMountedNoticeFireStatus(VEHICLE_MOUNTED_FIRING);
		sleep(NOTE_TO_C2_M);
	}

	/*disable fired */
	_VehicleMountedFiredAll(false);

	EAP_LOG_DEBUG("stop fire all drone\n");
	
	/* send fired finish mesg to c2 */
	_VehicleMountedNoticeFireStatus(VEHICLE_MOUNTED_FIRED);
	
	/* restart rf */
	ret = _VehicleMountedOnlineRF();
	if(ret < 0){
		EAP_LOG_ERROR("err, close rf failed!\n");
		return ;
	}

	/* start rf app */
	
	return ;
	
}

void VehicleMountedCmdproc::_VehicleMountedStopFiredAll(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
	int ret = 0;
	
	/*disable fired */
	_VehicleMountedFiredAll(false);

	/* restart rf */
	ret = _VehicleMountedOnlineRF();
	if(ret < 0){
		EAP_LOG_ERROR("err, close rf failed!\n");
		return ;
	}

	return ;
	
}

void VehicleMountedCmdproc::VehicleMountedSetFiredTime(uint8_t *firetime)
{
	if(!firetime){
		EAP_LOG_ERROR("para err!\n");
		return ;
	}
	
	_uFiredTime = *firetime;
	
	EAP_LOG_DEBUG("_uFiredTime = %d\n", _uFiredTime);
	
	return ;

}

void VehicleMountedCmdproc::VehicleMountedGetFiredTime(uint8_t *firetime)
{
	if(!firetime){
		EAP_LOG_ERROR("para err!\n");
		return ;
	}
	
	EAP_LOG_DEBUG("---------\n");
	
	*firetime = _uFiredTime;
	
	EAP_LOG_DEBUG("[%s, %d]_uFiredTime = %d\n", __func__, __LINE__, _uFiredTime);
	
	return ;

}


void VehicleMountedCmdproc::VehicleMountedSetFiredMode(uint8_t *fireMode)
{
	if(fireMode){
		_uModeSelect = *fireMode;
		printf("[%s, %d]_uModeSelect = %d\n", __func__, __LINE__, _uModeSelect);
	}
}

void VehicleMountedCmdproc::VehicleMountedGetFiredMode(uint8_t *fireMode)
{
	if(fireMode){
		*fireMode = _uModeSelect;
		printf("[%s, %d]*fireMode = %d\n", __func__, __LINE__, *fireMode);
	}
}

void VehicleMountedCmdproc::_VehicleMountedSetFiredirection(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
	int length = 0;
	alink_msg_t *pmsg = (alink_msg_t *)inMsg.data;
	alink_msg_t *prsp = (alink_msg_t *)outMsg.data;

	if(!pmsg || !prsp){
		EAP_LOG_ERROR("para err!\n");
	}
	
	EAP_LOG_DEBUG("pmsg->msg_head.msgid = 0x%x\n", pmsg->msg_head.msgid);
	
	length = (pmsg->msg_head.len_hi << 8) | pmsg->msg_head.len_lo;
	
	EAP_LOG_DEBUG("length = %d\n", length);
	
	if(length){
		_isAllStrike = pmsg->buffer[0];
	}
	
	EAP_LOG_DEBUG("_isAllStrike = %d\n", _isAllStrike);
	
	prsp->buffer[0] = 1;
	return ;

}




