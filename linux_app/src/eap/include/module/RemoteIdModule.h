#pragma once

#include <map>
#include <mutex>
#include <string>
#include "ModuleReal.h"
#include "RptController.h"
#include "RemoteIdInfo.h"

using namespace std;

class RemoteIdKey
{
public:
    RemoteIdKey() : deviceType(0)
    {
    }

    bool operator<(const RemoteIdKey &key) const //注意这里的两个const
    {
        if (deviceType == key.deviceType) return key.mac.compare(mac);
        return deviceType < key.deviceType;
    }
    uint8_t deviceType;
	string mac;
private:
};

class RemoteIdModule : public ModuleReal
{
public:
    RemoteIdModule(uint8_t coreId);
    virtual void BeaconFreamProcess(uint8_t* RevBuff, uint32_t len);
	virtual void RemoteIdCmdRidCapture(RemoteId_Info *rid_info);
    int16_t FillRemoteIdInfo(uint8_t* buf, uint16_t bufLen);
    void OldRemoteIdInfo();
    uint16_t GetRemoteIdNum();
    void Clear();
    uint8_t GetModuleId() override;
    void RequestShowInfo() override;
    void ShowInfo(uint8_t level) override;
    bool IsActiveTargetInChannel(const std::uint32_t& channel);
protected:
    ThreadBase* _CreateThread() override;
    CmdprocBase* _CreateCmdproc() override;
private:
    void _debugRecord(const RemoteId_Info& info);
    void _debugRecordReport(const Rpt_RemoteInfo& info);
    int8_t _GetDeviceType(uint8_t* revBuff);
	UAV_DEVICE_TYPE _GetDeviceTypeByMac(uint8_t* mac);
    UAV_DEVICE_TYPE_BY_SN _GetDeviceTypeBySn(char* sn);
    void _ParseUavMesssgeInfo(uint8_t* revBuff, uint16_t len, RemoteId_Info& info);
    void _ParseSysMesssgeInfo(uint8_t* revBuff, uint16_t len, RemoteId_Info& info);
    void _ParseSnCodeMesssgeInfo(uint8_t* revBuff, uint16_t len, RemoteIdKey& key,RemoteId_Info& info);
    void _RefreshUavMesssgeInfo(const RemoteId_Info& from, RemoteId_Info& to);
    void _RefreshSysMesssgeInfo(const RemoteId_Info& from, RemoteId_Info& to);
    void _FillDroneName(uint8_t type, char* droneName);
    void _FillDroneNameBySn(uint8_t type, char* droneName);
    int16_t _GetDroneAltitude(uint8_t type, uint16_t uavHeightAgl);
    int16_t _GetDroneSpeed(uint8_t type, const RemoteId_Info& info);
    int16_t _GetDroneVerticalSpeed(uint8_t type, const RemoteId_Info& info);
    void _FillPilotInfo(uint8_t type, const RemoteId_Info& info, Rpt_RemoteInfo& fillInfo);
    void _TargetWarningProc(uint16_t oldTargetNum);
    void _TargetWarningProcWhitelist();

    map<RemoteIdKey, RemoteId_Info> _remoteIdInfos;
    RptController _rptCtrl;

    bool _debugLogEnable;
    std::mutex _remoteIdInfoMutex;
    uint16_t _enemyTargetNum;
};

