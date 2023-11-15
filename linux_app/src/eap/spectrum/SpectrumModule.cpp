#include "json_pb_converter.h"
#include "SpectrumModule.h"
#include "SpectrumCmdproc.h"
#include "SpectrumThread.h"
#include "OrientationProc.h"
#include "AirBorneProc.h"
#include "AlinkCmdproc.h"
#include "DroneSnifferModule.h"
#include "AirBorneSnifferModule.h"
#include "HalMgr.h"
#include "VehicleMountedModule.h"
#include "VehicleMountedCmdproc.h"


extern "C"
{
#include "sfAirBorneSniffer.h"
}

/*for tracers shrd106l */
extern VehicleMountedModule* GetVehicleMountedModuler();

static SpectrumModule* GetSpectrumModule()
{
    return dynamic_cast<SpectrumModule*>(EapGetSys().GetModuleStrategy(EAP_MODULE_ID_SPECTRUM));
}

extern "C"
{

uint16_t Alink_EnterOrientate_Func(uint8_t *request, uint8_t *response)
{
    uint8_t retLen = 0;
    alink_msg_t *request_msg = (alink_msg_t *)request;
    alink_msg_t *response_msg = (alink_msg_t *)response;
    uint8_t switchState = request_msg->buffer[0];

    retLen = (request_msg->msg_head.len_hi << 8) | request_msg->msg_head.len_lo;
    if (retLen != 1)
    {
    	retLen = 1;
    	response_msg->buffer[0] = false;
    	return retLen;
    }

    if (switchState == 1)
    {
    	SpectrumModule* module = GetSpectrumModule();
        if (NULL != module) module->AdminOrientate();
    }
    else if (switchState == 0)
    {
        SendAsynCmdCode(EAP_CMD_CLEAR_ORIENTATE);
    }
    else
    {
    	retLen = 1;
    	response_msg->buffer[0] = 0;
    	return retLen;
    }

    EAP_LOG_DEBUG("set enter or exit ok ,state=%d(0:exit,1:enter)\r\n",switchState);

    response_msg->buffer[0] = 1;
    retLen = 1;
    return retLen;
}

}

SpectrumModule::SpectrumModule(uint8_t coreId) : ModuleReal(coreId)
{
    _hasOrientateDrone = false;
    _oriProc = nullptr;
    _arbnProc = nullptr;
    if (ProductSwCfgGet().issupportspectrumorient())
        _oriProc = new OrientationProc(*this);
    if (ProductSwCfgGet().issupportairborneorient())
        _arbnProc = new AirBorneProc(*this);
}

uint8_t SpectrumModule::GetModuleId()
{
    return EAP_MODULE_ID_SPECTRUM;
}

OrientationProc& SpectrumModule::OriProc()
{
    assert(nullptr != _oriProc);
    return *_oriProc;
}

AirBorneProc& SpectrumModule::ArbnProc()
{
    assert(nullptr != _arbnProc);
    return *_arbnProc;
}

SpectrumModule::~SpectrumModule()
{
    if (ProductSwCfgGet().issupportspectrumorient())
        EAP_DELETE(_oriProc);
    if (ProductSwCfgGet().issupportairborneorient())
        EAP_DELETE(_arbnProc);
}

void SpectrumModule::Init()
{
    register_alink_cmd(EAP_ALINK_CMD_ENTER_ORIENTATE, Alink_EnterOrientate_Func);
    ModuleReal::Init();
}

CmdprocBase* SpectrumModule::_CreateCmdproc()
{
    SpectrumCmdproc* cmdproc = new SpectrumCmdproc();
    if (ProductSwCfgGet().issupportspectrumorient())
        cmdproc->SetSendBuf(DRONEID_SNIFFER_PARAM_LEN + sizeof(Thread_Msg_Head));
    else if (ProductSwCfgGet().issupportairborneorient())
    {
        cmdproc->SetSendBuf(AIRBORNE_PARAM_LEN + sizeof(Thread_Msg_Head));
        EAP_LOG_DEBUG("SetSendBuf=%d\n", AIRBORNE_PARAM_LEN + sizeof(Thread_Msg_Head));
    }

    return cmdproc;
}

ThreadBase* SpectrumModule::_CreateThread()
{
    return ADD_THREAD(SpectrumThread, _coreId);
}

int32_t SpectrumModule::get_angle_info()
{
    int32_t angle = 0;
    HalMgr* halMgr = EapGetSys().GetHalMgr();
    Drv_AngleInfo angleInfo = {0};
    Hal_Data inData = {0, nullptr};
    Hal_Data outData = {sizeof(Drv_AngleInfo), (uint8_t*)&angleInfo};
    if (nullptr != halMgr && EAP_SUCCESS == halMgr->OnGet(EAP_DRVCODE_GET_ANGLE_INFO, inData, outData))
    {
        angle = (int32_t)angleInfo.yaw * 100;
    }
    return angle;
}

int16_t SpectrumModule::FillSpectrumInfo(uint8_t* buf, uint16_t bufLen)
{
    int8_t count = _spectrumInfos.size();
    if (!_rptCtrl.Report(count)) return -1;
    int16_t dataLen = EAP_SN_LENGTH + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(int32_t) + 1 + count * sizeof(Rpt_SpectrumDroneInfo);
    if (dataLen > bufLen) return -1;

    uint8_t *data = buf;
    memcpy(buf, EapGetSys().GetSnName(), EAP_SN_LENGTH);
    data += EAP_SN_LENGTH;

    // 全向侦测模式临时上报"全向天线功率,定向天线功率,定向天线水平角"3个参数
    uint16_t QxPower = 0x00;
    uint16_t DxPower = 0x00;
    int32_t DxHorizon = 0x00;
    memcpy(data,&QxPower,sizeof(uint16_t));
    data += sizeof(uint16_t);
    memcpy(data,&DxPower,sizeof(uint16_t));
    data += sizeof(uint16_t);
    DxHorizon = get_angle_info();
    memcpy(data,&DxHorizon,sizeof(int32_t));
    data += sizeof(int32_t);
    // EAP_LOG_DEBUG("QxPower=%d, DxPower=%d, DxHorizon=%d\n", QxPower, DxPower, DxHorizon);

    // 上报无人机数量及参数信息
    *data++ = count;
    int32_t levelAngleCen = OriProc().GetLevelAngleCen();
    levelAngleCen = (int32_t)CONVERT_32_REV(levelAngleCen);
    int32_t pitchAngleCen = OriProc().GetPitchAngleCen();
    pitchAngleCen = (int32_t)CONVERT_32_REV(pitchAngleCen);
    for (auto& entry : _spectrumInfos)
    {
        Rpt_SpectrumDroneInfo* info = (Rpt_SpectrumDroneInfo *)data;
        memcpy(info->droneName, entry.first.c_str(), min(entry.first.size(), sizeof(info->droneName)));
        info->uavNumber  = 0xFF;
        info->droneHorizon = levelAngleCen;
        uint32_t freq = (uint32_t)(entry.second.actualFreq);
        info->uFreq        = CONVERT_32_REV(freq);
        info->dangerLevels = 2;

        data += sizeof(Rpt_SpectrumDroneInfo);
    }
    _rptCtrl.Clear();
    return dataLen;
}


void SpectrumModule::FiredDroneAuto(SpectrumAlgResult* result, uint16_t num)
{
	VehicleMountedModule *pvehMod = NULL;
	VehicleMountedCmdproc *pvehCmd = NULL;

	pvehMod = GetVehicleMountedModuler();
	if(!pvehMod){
		EAP_LOG_ERROR("err,  pvehMod is NULL!\n");
	}
	
	pvehCmd = (VehicleMountedCmdproc *)pvehMod->getCmdProc();
	if(!pvehCmd){
		EAP_LOG_ERROR("err,  pvehCmd is NULL!\n");
	}
	
	pvehCmd->VehicleMountedAutoFire(result, num);

	return;
}

int SpectrumModule::FiredDroneGetFireMode(void)
{
	VehicleMountedModule *pvehMod = NULL;
	VehicleMountedCmdproc *pvehCmd = NULL;
	uint8_t firemode = 0;

	pvehMod = GetVehicleMountedModuler();
	if(!pvehMod){
		EAP_LOG_ERROR("err,  pvehMod is NULL!\n");
	}
	
	pvehCmd = (VehicleMountedCmdproc *)pvehMod->getCmdProc();
	if(!pvehCmd){
		EAP_LOG_ERROR("err,  pvehCmd is NULL!\n");
	}
	
	pvehCmd->VehicleMountedGetFiredMode(&firemode);

	return firemode;
}


void SpectrumModule::Sensing(float curDetectFreq, SpectrumAlgResult* result, uint16_t num)
{
    static uint8_t last_workMode;
    uint8_t workMode = EapGetSys().GetWorkMode();
    uint16_t oldTargetNum = _spectrumInfos.size();
    if (WORKMODE_SPECTRUM_OMNI == workMode || WORKMODE_SPECTRUM_OMNI_ORIE == workMode)
    {
        _SaveAlgResult(curDetectFreq, result, num);

        if (WORKMODE_SPECTRUM_OMNI_ORIE == workMode)
        {
            OriProc().Proc(curDetectFreq);
        }
        // 从定向切回全向模式时重置切频程序
        else if (workMode != last_workMode)
        {
            OriProc().Clear();
        }
        last_workMode = workMode;
    }
    else
    {
        _spectrumInfos.clear();
        OriProc().Clear();
    }
    _TargetWarningProc(oldTargetNum);

	if(DEV_TRACER_SEH100 == ProductSwCfgGet().devicetype()){
		EAP_LOG_DEBUG("fire the drone auto, num = %d\n", num);
		if(num > 0){
			if(AUTO_FIRED == FiredDroneGetFireMode()){
				EAP_LOG_DEBUG("system status = %d\n", EapGetSys().GetSysStatus());
				if(SYS_DETECTION_TARGET_STATUS == EapGetSys().GetSysStatus()){
					EAP_LOG_DEBUG("fire the drone auto!\n");
					FiredDroneAuto(result,num);
				}
			}
		}
	}
	
    _rptCtrl.Refresh(oldTargetNum, _spectrumInfos.size());
}

void SpectrumModule::_TargetWarningProc(uint16_t oldTargetNum)
{
    uint16_t newTargetNum = _spectrumInfos.size();
    bool isTargetAppear = (0 == oldTargetNum && newTargetNum > 0);
    bool isTargetDisAppear = (0 < oldTargetNum && 0 == newTargetNum);
    if (isTargetAppear || isTargetDisAppear)
    {
        TargetWarning warnings = {EAP_TARGET_SCENE_SPECTRUM, (uint8_t)(isTargetAppear ? 1 : 0)};
        SendAsynData(EAP_CMD_TARGET_WARNING, (uint8_t*)&warnings, sizeof(TargetWarning));
    }
}

void SpectrumModule::_SaveAlgResult(float curDetectFreq, SpectrumAlgResult* result, uint16_t num)
{
    for (int i = 0; i < num; ++i)
    {
        string key;
        key.assign(result[i].name, sizeof(result[i].name));
        auto iter = _spectrumInfos.find(key);
        if (iter != _spectrumInfos.end())
        {
            iter->second.detectFreq     = curDetectFreq;
            iter->second.actualFreq     = result[i].freq[0];
            iter->second.amp            = result[i].amp;
            iter->second.remainingTimes = EAP_SPECTRUM_TARGET_REMAIN_TIMES;
        }
        else
        {
            SpectrumDrone_Info info;
            info.detectFreq     = curDetectFreq;
            info.actualFreq     = result[i].freq[0];
            info.remainingTimes = EAP_SPECTRUM_TARGET_REMAIN_TIMES;
            info.amp            = result[i].amp;
            _spectrumInfos.insert(make_pair(key, info));
        }
        EAP_LOG_DEBUG("num=%d, name=[%s], curDetectFreq=%0.1f, act_freq=%0.1f, amp=%0.3f\n", num, result[i].name, curDetectFreq, result[i].freq[0], result[i].amp);
    }
    _OldSpectrumDrone();
    if (OriProc().IsOpenOrientation())
    {
        _RefreshOrientateDrone();
    }
}

void SpectrumModule::_OldSpectrumDrone()
{
    for (auto iter = _spectrumInfos.begin(); iter != _spectrumInfos.end();)
    {
        if (iter->second.remainingTimes > 0) iter->second.remainingTimes--;
        if (0 == iter->second.remainingTimes)
        {
            _spectrumInfos.erase(iter++);
        }
        else
        {
            ++iter;
        }
    }
}

void SpectrumModule::_RefreshOrientateDrone()
{
    if (0 == _spectrumInfos.size())
    {
        _hasOrientateDrone = false;
        return;
    }
    _hasOrientateDrone = true;
    _orientateDroneIter = _spectrumInfos.begin();
    float maxAmp = _orientateDroneIter->second.amp;
    auto iter = _orientateDroneIter;
    ++iter;
    for (; iter != _spectrumInfos.end(); ++iter)
    {
        if (iter->second.amp > maxAmp)
        {
            maxAmp = iter->second.amp;
            _orientateDroneIter = iter;
        }
    }
}

const SpectrumDrone_Info* SpectrumModule::GetOrientateDrone()
{
    if (!_hasOrientateDrone) return nullptr;
    return &(_orientateDroneIter->second);
}

void SpectrumModule::AirBorneCtrl(void *data)
{
    assert(nullptr != data);
    ArbnProc().SetCtrlState(data);
}

int32_t SpectrumModule::AirBorneGetParam(void *data)
{
    assert(nullptr != data);
    return ArbnProc().GetParam(data);
}

void SpectrumModule::ClearOrientate()
{
    OriProc().Clear();
}

void SpectrumModule::AdminOrientate()
{
    OriProc().EnterAdminOri();
}

void SpectrumModule::RequestShowInfo()
{
    SendAsynCmdCode(EAP_CMD_SHOW_SPECTRUM_MODULE);
}

void SpectrumModule::ShowInfo(uint8_t level)
{
    ModuleReal::ShowInfo(level);
    cout << EAP_BLANK_PREFIX(level) << "_hasOrientateDrone: " << _hasOrientateDrone;
    if (_hasOrientateDrone)
    {
        cout << EAP_BLANK_PREFIX(level) << "_orientateDrone: ";
        cout << EAP_BLANK_PREFIX(level + 1) << "name: " << _orientateDroneIter->first;
        cout << EAP_BLANK_PREFIX(level + 1) << "detectFreq: " << _orientateDroneIter->second.detectFreq;
        cout << EAP_BLANK_PREFIX(level + 1) << "actualFreq: " << _orientateDroneIter->second.actualFreq;
        cout << EAP_BLANK_PREFIX(level + 1) << "amp: " << _orientateDroneIter->second.amp;
        cout << EAP_BLANK_PREFIX(level + 1) << "remainingTimes: " << (uint16_t)_orientateDroneIter->second.remainingTimes;
    }
    cout << EAP_BLANK_PREFIX(level) << "SpectrumNum: " << _spectrumInfos.size();
    cout << EAP_BLANK_PREFIX(level) << "RptIntervalTimes: " << (uint16_t)_rptCtrl.GetInterval();
    cout << EAP_BLANK_PREFIX(level) << "_spectrumInfos: ";
    int i = 0;
    for (auto spectrum : _spectrumInfos)
    {
        cout << EAP_BLANK_PREFIX(level + 1) << "_spectrumInfos[" << i << "]: ";
        cout << EAP_BLANK_PREFIX(level + 2) << "name: " << spectrum.first;
        cout << EAP_BLANK_PREFIX(level + 2) << "detectFreq: " << spectrum.second.detectFreq;
        cout << EAP_BLANK_PREFIX(level + 2) << "actualFreq: " << spectrum.second.actualFreq;
        cout << EAP_BLANK_PREFIX(level + 2) << "amp: " << spectrum.second.amp;
        cout << EAP_BLANK_PREFIX(level + 2) << "remainingTimes: " << (uint16_t)spectrum.second.remainingTimes;
        ++i;
    }
    if (nullptr != _oriProc)
    {
        cout << EAP_BLANK_PREFIX(level) << "_oriProc: ";
        _oriProc->ShowInfo(level + 1);
    }
    if (nullptr != _arbnProc)
    {
        cout << EAP_BLANK_PREFIX(level) << "_arbnProc: ";
        _arbnProc->ShowInfo(level + 1);
    }
}
