#pragma once

#include <cstdint>
#include <set>
#include <string>

#include "eap_pub.h"

class ThreadMgr;
class Module;
class HalMgr;
class ModuleStrategy;
class WhitelistManager;

class SysBase
{
public:
    SysBase(uint8_t deviceType, int16_t processId = -1,char *productCfgStdPath = NULL, char *productCfgDebugPath = NULL);
    virtual ~SysBase();
    ThreadMgr& GetThreadMgr();
    HalMgr* GetHalMgr() { return _halMgr; }
    virtual void Init();
    uint32_t GetModulesNum();
    int16_t GetProcessId() { return _processId; }
    char* GetSnName() { return _snName; }
    void SetSnName(char* snName, uint8_t snLen);
    uint8_t GetSnNameLen() { return _snLen; }
    char* GetRemoteAddr() { return _remoteAddr; }
    void SetRemoteAddr(char* remoteAddr);
    uint8_t GetDeviceType() { return _deviceType; }
    Module* GetModule(uint8_t moduleId);
    ModuleStrategy* GetModuleStrategy(uint8_t moduleId);
    void SetWorkMode(uint8_t workMode);
    uint8_t GetWorkMode() { return _workMode; }
    void SetSysStatus(uint8_t status);
    uint8_t GetSysStatus() { return _sysStatus; }
    void SetCurDetectFreq(float freq) { _curDetectFreq = freq; }
    float GetCurDetectFreq() { return _curDetectFreq; }
    virtual void _RegisterCliCmd();
    uint32_t GetPlVersionDate() { return _plVersionDate; }
    uint32_t GetPlVersionTime() { return _plVersionTime; }
    void RefreshBatteryCapacity();
    uint8_t GetBatteryCapacity() { return _batteryCapacity; }
    uint8_t GetBatAndAdapterState() { return _batAndAdapterState; }
    void SetTargetAppear(uint8_t isTargetAppear);
    void SendALinkMsg(uint8_t channelType, uint8_t msgType, uint32_t p1, uint32_t p2, uint32_t p3);
    void SetReceivedALinkMsg(const int* buff, const std::uint32_t length);
    void SetLedStatus(uint8_t ledNo, uint8_t status);
    void AirBorneCtrl(uint16_t workMode, uint16_t dxNum, uint16_t dxFreq);
    void AdcSetGain(uint32_t value);
    void ShowModule(uint8_t moduleId);
    void ShowModuleUnSafe(uint8_t moduleId);
    void SetModuleFilterLevel(const uint8_t moduleId, const log_level_t level);
    void SetOtherModulesFilterLevel(const uint8_t moduleId, const log_level_t level);
    void ShowInfo();
    void SetDupInfo(uint32_t protocol);
    void DupResponse();
    void ClearRemainDupTimes() { _remainDupTimes = 0; }
    bool IsAlinkProtoBuf(){ return EAP_ALINK_PROTOBUF == _dupProtocol; }
    void SetDupProtocol(uint32_t protocol) { _dupProtocol = protocol; }
    uint32_t GetDupProtocol() { return _dupProtocol; }
    virtual int PlGenDmaCalc(int drvCode, uint8_t *para_x, uint32_t leng_x, uint8_t *para_y, uint32_t leng_y) { return EAP_SUCCESS; }
    void Telnet_GetSN();
    void Telnet_SetSN(char *sn, int length);
    void Telnet_GetRemoteAddr();
    WhitelistManager *GetWhitelistManager() {return _whitelistManager;}
protected:
    void _InstallModule(Module* module);
    void _InstallHalMgr(HalMgr* halMgr);
    void _GetSnFromHal();
    void _GetPLVersionFromHal();
    void _GetRemoteAddrFromHal();
    void _RegisterAlinkCmd();
    virtual void _RegisterAlgCb() {};
    int16_t _FillDupBroadcastResponseInfo(uint8_t* buf);
		
private:
    ThreadMgr* _threadMgr;
    HalMgr* _halMgr;
    int16_t _processId;
    Module* _modules[EAP_MODULE_ID_MAX];
    char _snName[EAP_SN_LENGTH];
    uint8_t _snLen;
    char _remoteAddr[EAP_REMOTE_ADDR_LEN];
    uint8_t _deviceType;
    uint8_t _workMode; // WORKMODE_DRONE ....
    uint8_t _sysStatus; // SYS_DETECTION_TARGET_STATUS ...
    float _curDetectFreq;
	uint32_t _plVersionDate;
	uint32_t _plVersionTime;
    uint8_t _batteryCapacity;
    uint8_t _batAndAdapterState;
    uint32_t _dupProtocol;
    uint8_t _remainDupTimes;
    WhitelistManager *_whitelistManager = nullptr;
};

extern SysBase& EapGetSys();

extern "C"
{
    uint16_t Alink_GetSoftwareVersion_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_SystemReset_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_GetBoardSN_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_SetBoardSN_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_Get_Log_List_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_Get_Log_Data_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_Delete_Log_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_SetOrientationWorkMode_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_DeviceStatusInfo_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_GetDevInfo_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_GetWorkMode_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_C2HeartBeatPacket_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_C2CliCmd_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_C2BlueToothCmd_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_GetC2TimeInfo_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_GetSelfInspectInfo_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_C2SetFiredMode_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_C2GetFiredMode_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_C2StartFiredALL_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_C2StopFiredALL_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_C2GetFiredTime_Func(uint8_t * request, uint8_t * response);
    uint16_t Alink_C2SetFiredTime_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_SetAlarmMode_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_SetCovertMode_Func(uint8_t *request, uint8_t *response);
    uint16_t Alink_SetWhiteList_Func(uint8_t *request, uint8_t *response);
}
