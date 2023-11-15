#include "DroneIdModule.h"
#include "DroneIdCmdproc.h"
#include "DroneIdThread.h"
#include "SysBase.h"
#include "HalMgr.h"
#include "MavAlink.pb.h"
#include "hal.h"
#include "json_pb_converter.h"
#include "WhitelistManager.h"

extern "C"
{
#include "droneID_utils.h"
#include "alg_init.h"
}

void DroneIdModule::Init()
{
    ModuleReal::Init();
    algorithm_init();
}

DroneIdModule::DroneIdModule(uint8_t coreId) : ModuleReal(coreId)
{
    _curFreq = 0;
    memset(_parseAllInfo, 0, sizeof(_parseAllInfo));
    _droneIdNum = 0;

    if (ProductSwCfgGet().issupportwarningwhitelist())
    {
        _enemyDroneIdNum = 0;
    }
}

uint8_t DroneIdModule::GetModuleId()
{
    return EAP_MODULE_ID_DRONEID;
}

CmdprocBase* DroneIdModule::_CreateCmdproc()
{
    return new DroneIdCmdproc();
}

ThreadBase* DroneIdModule::_CreateThread()
{
    EAP_LOG_INFO("DroneIdModule::_CreateThread() success\n");
    return ADD_THREAD(DroneIdThread, _coreId);
}

void DroneIdModule::RunDroneAlg(eDATA_PATH_INTR_FLAG flag,int channel)
{
    HalMgr* halMgr = EapGetSys().GetHalMgr();
    Drv_Drone_Freq freq = {0, 0};
    uint8_t workMode = EapGetSys().GetWorkMode();
    Hal_Data inData = {sizeof(uint8_t), &workMode};
    Hal_Data outData = {sizeof(Drv_Drone_Freq), (uint8_t*)&freq};
    if (nullptr != halMgr && EAP_SUCCESS == halMgr->OnGet(EAP_DRVCODE_GET_DRONE_FREQ, inData, outData))
    {
        _SetAlgFreq(freq.sigFreq, freq.freqOffset);
        _RunAlg(flag,channel);
    }
}

void DroneIdModule::RunDroneAlgNew(eDATA_PATH_INTR_FLAG flag,int channel)
{
    HalMgr* halMgr = EapGetSys().GetHalMgr();
    Drv_Drone_Freq freq = {0, 0};
    uint8_t workMode = EapGetSys().GetWorkMode();
    Hal_Data inData = {sizeof(uint8_t), &workMode};
    Hal_Data outData = {sizeof(Drv_Drone_Freq), (uint8_t*)&freq};
    if (nullptr != halMgr && EAP_SUCCESS == halMgr->OnGet(EAP_DRVCODE_GET_DRONE_FREQ, inData, outData))
    {
        _SetAlgFreq(freq.sigFreq, freq.freqOffset);
        _RunAlgNew(flag,channel);
    }
}

void DroneIdModule::_SetAlgFreq(uint64_t sigFreq, uint32_t freqOffset)
{
    SetFreqOffsetHz(freqOffset);
    SetDroneIDSigFreqHz(sigFreq);
}

void DroneIdModule::_RunAlg(eDATA_PATH_INTR_FLAG flag,int channel)
{
    uint16_t oldTargetNum = _droneIdNum;
    uint32_t newTargetNum = _droneIdNum;
    if (eDATA_PATH_TIMER == flag)
    {
        newTargetNum = algorithm_Run(flag, nullptr, 0, _parseAllInfo);
    }
    else
    {
        HalMgr* halMgr = EapGetSys().GetHalMgr();
		burst_data_event event;
		memset(&event,0,sizeof(event));
		event.flag = flag;
		event.channel = channel;
        Hal_Data inData = {sizeof(event), (uint8_t*)&event};
        Hal_Data outData = {EAP_DRONEID_VALID_ADC_LEN, (uint8_t*)_bufferAdc};
        if (nullptr != halMgr && EAP_SUCCESS == halMgr->OnGet(EAP_DRVCODE_GET_DRONE_ADC, inData, outData))
        {
            newTargetNum = algorithm_Run(flag, _bufferAdc, EAP_DRONEID_VALID_ADC_LEN, _parseAllInfo);
        }
    }
    if (newTargetNum > EAP_DRONEID_MAX_NUM)
    {
        EAP_LOG_ERROR("_droneIdNum(%d) is error!", _droneIdNum);
        _droneIdNum = EAP_DRONEID_MAX_NUM;
    }
    else
    {
        _droneIdNum = newTargetNum;
    }
    if (ProductSwCfgGet().issupportwarningwhitelist())
    {
        _TargetWarningProcWhitelist();
    }
    else
    {
        _TargetWarningProc(oldTargetNum);
    }
    _rptCtrl.Refresh(oldTargetNum, newTargetNum);
}

void DroneIdModule::_RunAlgNew(eDATA_PATH_INTR_FLAG flag,int channel)
{
    uint16_t oldTargetNum = _droneIdNum;
    uint32_t newTargetNum = _droneIdNum;
    if (eDATA_PATH_TIMER == flag)
    {
       newTargetNum = algorithm_RunNew(flag, nullptr, 0, _parseAllInfo);
    }
    else
    {
        HalMgr* halMgr = EapGetSys().GetHalMgr();

		burst_data_event event;
		memset(&event,0,sizeof(event));
		event.flag = flag;
		event.channel = channel;
        Hal_Data inData = {sizeof(event), (uint8_t*)&event};
        Hal_Data outData = {EAP_DRONEID_VALID_ADC_LEN_WITH_HEAD, (uint8_t*)_bufferAdc};
        if (nullptr != halMgr && EAP_SUCCESS == halMgr->OnGet(EAP_DRVCODE_GET_DRONE_ADC, inData, outData))
        {
           newTargetNum = algorithm_RunNew(flag, _bufferAdc, EAP_DRONEID_VALID_ADC_LEN_WITH_HEAD, _parseAllInfo);
        }
    }
    if (newTargetNum > EAP_DRONEID_MAX_NUM)
    {
        EAP_LOG_ERROR("_droneIdNum(%d) is error!", _droneIdNum);
        _droneIdNum = EAP_DRONEID_MAX_NUM;
    }
    else
    {
        _droneIdNum = newTargetNum;
    }
    if (ProductSwCfgGet().issupportwarningwhitelist())
    {
        _TargetWarningProcWhitelist();
    }
    else
    {
        _TargetWarningProc(oldTargetNum);
    }
    _rptCtrl.Refresh(oldTargetNum, newTargetNum);
}

void DroneIdModule::_TargetWarningProc(uint16_t oldTargetNum)
{
    uint16_t newTargetNum = _droneIdNum;
    bool isTargetAppear = (0 == oldTargetNum && newTargetNum > 0);
    bool isTargetDisAppear = (0 < oldTargetNum && 0 == newTargetNum);
    if (isTargetAppear || isTargetDisAppear)
    {
        TargetWarning warning = {EAP_TARGET_SCENE_DRONEID, (uint8_t)(isTargetAppear ? 1 : 0)};
        SendAsynData(EAP_CMD_TARGET_WARNING, (uint8_t*)&warning, sizeof(TargetWarning));
    }
}

int16_t DroneIdModule::_FillDroneIdInfoPB(uint8_t* buf, uint16_t bufLen)
{
    ReportDroneIdPB pb;
    pb.set_tracername(EapGetSys().GetSnName());
    pb.set_udronenum(_droneIdNum);
    for (int i = 0; i < _droneIdNum; ++i)
    {
        ReportDroneIdPB_DroneInfo *info = pb.add_drones();
        info->set_producttype(_parseAllInfo[i].droneID.type_num);
        info->set_dronename((char *)_parseAllInfo[i].droneID.type_name);
        info->set_serialnum((char *)_parseAllInfo[i].droneID.serial_num);
        info->set_dronelongitude(_parseAllInfo[i].droneID.drone_longtitude);
        info->set_dronelatitude(_parseAllInfo[i].droneID.drone_latitude);
        info->set_droneheight(_parseAllInfo[i].droneID.height);
        info->set_droneyawangle(_parseAllInfo[i].droneID.yaw_angle);
        info->set_dronespeed(_parseAllInfo[i].droneID.speed);
        info->set_droneverticalspeed(_parseAllInfo[i].droneID.z_speed);
        info->set_pilotlongitude(_parseAllInfo[i].droneID.pilot_longitude);
        info->set_ufreq((uint32_t)(_parseAllInfo[i].droneID.drone_Freq / 1e6));
        info->set_dangerlevels(_parseAllInfo[i].droneID.danger_levels);
    }

    // 序列化消息
    int16_t dataLen = pb.ByteSizeLong();
    if (dataLen > bufLen) return -1;
    pb.SerializeToArray(buf, dataLen);

    return dataLen;
}

int16_t DroneIdModule::FillDroneIdInfo(uint8_t* buf, uint16_t bufLen)
{
    if (!_rptCtrl.Report(_droneIdNum))
    {
        return -1;
    }
    int16_t dataLen = 0;
    if (EapGetSys().IsAlinkProtoBuf()) dataLen = _FillDroneIdInfoPB(buf, bufLen);
    else dataLen = _FillDroneIdInfo(buf, bufLen);
    if (-1 != dataLen) _rptCtrl.Clear();
    return dataLen;
}

int16_t DroneIdModule::_FillDroneIdInfo(uint8_t* buf, uint16_t bufLen)
{
    int16_t dataLen = EAP_SN_LENGTH + 1 + _droneIdNum * sizeof(Rpt_DroneInfo);
    if (dataLen > bufLen) return -1;

    uint8_t *data = buf;
    memcpy(buf, EapGetSys().GetSnName(), EAP_SN_LENGTH);
    data += EAP_SN_LENGTH;
    *data++ = _droneIdNum;
    for (int i = 0; i < _droneIdNum; ++i)
    {
        Rpt_DroneInfo* info = (Rpt_DroneInfo *)data;
        info->productType = _parseAllInfo[i].droneID.type_num;
        memcpy(info->droneName, _parseAllInfo[i].droneID.type_name, sizeof(info->droneName));
        memcpy(info->serialNum, _parseAllInfo[i].droneID.serial_num, sizeof(info->serialNum));
        info->droneLongitude     = (int32_t)CONVERT_32_REV(_parseAllInfo[i].droneID.drone_longtitude);
        info->droneLatitude      = (int32_t)CONVERT_32_REV(_parseAllInfo[i].droneID.drone_latitude);
        info->droneHeight        = (int16_t)CONVERT_16_REV(_parseAllInfo[i].droneID.height);
        info->droneYawAngle      = (int16_t)CONVERT_16_REV(_parseAllInfo[i].droneID.yaw_angle);
        info->droneSpeed         = (int16_t)CONVERT_16_REV(_parseAllInfo[i].droneID.speed);
        info->droneVerticalSpeed = (int16_t)CONVERT_16_REV(_parseAllInfo[i].droneID.z_speed);
        info->pilotLongitude     = (int32_t)CONVERT_32_REV(_parseAllInfo[i].droneID.pilot_longitude);
        info->pilotLatitude      = (int32_t)CONVERT_32_REV(_parseAllInfo[i].droneID.pilot_latitude);
        info->uFreq              = CONVERT_32_REV((uint32_t)(_parseAllInfo[i].droneID.drone_Freq / 1e6));
        info->dangerLevels       = CONVERT_16_REV(_parseAllInfo[i].droneID.danger_levels);
        data += sizeof(Rpt_DroneInfo);
    }
    return dataLen;
}

uint16_t DroneIdModule::GetDroneIdNum()
{
    return _droneIdNum;
}

void DroneIdModule::Clear()
{
    _droneIdNum = 0;
}

int32_t algorithm_Timer()
{
	Thread_Msg msg;
	burst_data_event *event;
	event = (burst_data_event *)msg.buf;
	memset(event,0,sizeof(*event));

	// printf("%s: send event: eDATA_PATH_TIMER=%d\r\n", __func__, eDATA_PATH_TIMER);
	event->flag = eDATA_PATH_TIMER;
	SendAsynMsg(EAP_CMD_DRONEID, (uint8_t*)&msg, sizeof(*event));
    return 0;
}

void DroneIdModule::OnTimer()
{
    algorithm_Timer();
}

void DroneIdModule::RequestShowInfo()
{
    SendAsynCmdCode(EAP_CMD_SHOW_DRONEID_MODULE);
}

void DroneIdModule::ShowInfo(uint8_t level)
{
    ModuleReal::ShowInfo(level);
    cout << EAP_BLANK_PREFIX(level) << "_curFreq: " << _curFreq;
    cout << EAP_BLANK_PREFIX(level) << "_droneIdNum: " << (uint16_t)_droneIdNum;
    cout << EAP_BLANK_PREFIX(level) << "RptIntervalTimes: " << (uint16_t)_rptCtrl.GetInterval();
    cout << EAP_BLANK_PREFIX(level) << "_parseAllInfo: ";
    for (int i = 0; i < _droneIdNum; ++i)
    {
        cout << EAP_BLANK_PREFIX(level + 1) << "_parseAllInfo[" << i << "]: ";
        cout << EAP_BLANK_PREFIX(level + 2) << "type_num: " << (uint16_t)_parseAllInfo[i].droneID.type_num;
        cout << EAP_BLANK_PREFIX(level + 2) << "type_name: " << (char *)_parseAllInfo[i].droneID.type_name;
        cout << EAP_BLANK_PREFIX(level + 2) << "serial_num: " << (char *)_parseAllInfo[i].droneID.serial_num;
        cout << EAP_BLANK_PREFIX(level + 2) << "drone_longtitude: " << _parseAllInfo[i].droneID.drone_longtitude;
        cout << EAP_BLANK_PREFIX(level + 2) << "drone_latitude: " << _parseAllInfo[i].droneID.drone_latitude;
        cout << EAP_BLANK_PREFIX(level + 2) << "height: " << _parseAllInfo[i].droneID.height;
        cout << EAP_BLANK_PREFIX(level + 2) << "yaw_angle: " << _parseAllInfo[i].droneID.yaw_angle;
        cout << EAP_BLANK_PREFIX(level + 2) << "speed: " << _parseAllInfo[i].droneID.speed;
        cout << EAP_BLANK_PREFIX(level + 2) << "x_speed: " << _parseAllInfo[i].droneID.x_speed;
        cout << EAP_BLANK_PREFIX(level + 2) << "y_speed: " << _parseAllInfo[i].droneID.y_speed;
        cout << EAP_BLANK_PREFIX(level + 2) << "z_speed: " << _parseAllInfo[i].droneID.z_speed;
        cout << EAP_BLANK_PREFIX(level + 2) << "pilot_longitude: " << _parseAllInfo[i].droneID.pilot_longitude;
        cout << EAP_BLANK_PREFIX(level + 2) << "pilot_latitude: " << _parseAllInfo[i].droneID.pilot_latitude;
        cout << EAP_BLANK_PREFIX(level + 2) << "danger_levels: " << _parseAllInfo[i].droneID.danger_levels;
        cout << EAP_BLANK_PREFIX(level + 2) << "drone_Freq: " << _parseAllInfo[i].droneID.drone_Freq;
    }
    cout << EAP_BLANK_PREFIX(level) << "_bufferAdc: " << (uint64_t)_bufferAdc;
}

void DroneIdModule::_TargetWarningProcWhitelist()
{
    uint32_t oldEnemyNum = _enemyDroneIdNum;
    vector<string> list = EapGetSys().GetWhitelistManager()->GetWhitelist();
    bool inWhitelist = false;

    EAP_LOG_DEBUG("oldEnemyNum=%d, whitelist size=%d", oldEnemyNum, list.size());
    _enemyDroneIdNum = _droneIdNum;
    for (uint32_t i = 0; i < _droneIdNum; i++)
    {
        inWhitelist = false;
        for (vector<string>::iterator it = list.begin(); it != list.end(); it++)
        {
            if (0 == it->compare((const char*)(_parseAllInfo[i].droneID.serial_num)))
            {
                inWhitelist = true;
                EAP_LOG_DEBUG("droneID in Whitelist: serial_num=%s, _enemyDroneIdNum=%d", _parseAllInfo[i].droneID.serial_num, _enemyDroneIdNum);
                break;
            }
        }
        if (inWhitelist)
        {
            _enemyDroneIdNum--;
        }
    }

    bool isTargetAppear = (0 == oldEnemyNum && 0 < _enemyDroneIdNum);
    bool isTargetDisAppear = (0 < oldEnemyNum && 0 == _enemyDroneIdNum);
    if (isTargetAppear || isTargetDisAppear)
    {
        TargetWarning warning = {EAP_TARGET_SCENE_DRONEID, (uint8_t)(isTargetAppear ? 1 : 0)};
        SendAsynData(EAP_CMD_TARGET_WARNING, (uint8_t*)&warning, sizeof(TargetWarning));
    }
}

