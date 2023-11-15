#pragma once

#ifndef TRACERDEVICEHAL_H
#define TRACERDEVICEHAL_H

#include <stdint.h>
#include <unistd.h>
#include <mutex>
#include <thread>

#include "HalBase.h"
#include "hal/aaps_info/AAPSInfo.hpp"
#include "hal/tmp75c/tmp75c.hpp"
#include "hal/ads1115/ads1115.hpp"
#include "hal/libattery/libattery.hpp"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
#include "eap_pub.h"
#include "hal.h"
#include "gpio/gpioapp.h"
#include "output/output.h"
#include "input/input.h"
#ifdef __cplusplus
}
#endif

enum KEY_BOARD_TYPE
{
    OLD_KEY_BOARD,
    NEW_KEY_BOARD
};

enum RF_STATUS
{
    RF_DISABLE,
    RF_ENABLE
};

// 供电方式
enum POWER_SUPPLY_MODE
{
    BATTERY_DISCHARGE = 0,  // 电池放电
    BATTERY_CHARGE,         // 电池充电
    POWER_ADAPTER           // 电源适配器
};

enum FAN_SPEED_CTRL : std::uint8_t
{
    FAN_SPEED_LEV0  = 0,
    FAN_SPEED_LEV1  = 1,
    FAN_SPEED_LEV2  = 2,
    FAN_SPEED_LEV3  = 3,
    FAN_SPEED_LEV4  = 4,
    FAN_SPEED_LEV5  = 5,
    FAN_SPEED_LEV6  = 6,
    FAN_SPEED_LEV7  = 7,
    FAN_SPEED_LEV8  = 8,
    FAN_SPEED_LEV9  = 9,
    FAN_SPEED_LEV10 = 10,
};

enum ENUM_PE42442_CHANNEL : std::uint8_t
{
    CHANNEL_RN440      = 1,
    CHANNEL_PROTECTION = 2,
    CHANNEL_RESERVE    = 3,
    CHANNEL_RFBOARD    = 4,
};

typedef struct __PE42442_CTRL_STRUCT__
{
    std::uint8_t _v1 : 1;
    std::uint8_t _v2 : 1;
    std::uint8_t _v3 : 1;
} PE42442_CTRL_STRUCT;

class TracerDeviceHal : public HalBase
{
public:
    TracerDeviceHal(HalMgr* halMgr);
    ~TracerDeviceHal();
	void Init() override;

protected:
    int _GetBatteryCapacity(const Hal_Data& inData, Hal_Data& outData);
    int _GetSilentMode(const Hal_Data& inData, Hal_Data& outData);
    int _GetAlarmMode(const Hal_Data& inData, Hal_Data& outData);
    int _SetAlarmMode(const Hal_Data& inData, Hal_Data& outData);
    int _SetCovertMode(const Hal_Data& inData, Hal_Data& outData);
    int _SetLedState(const Hal_Data& inData, Hal_Data& outData);
    int _SetMotorState(const Hal_Data& inData, Hal_Data& outData);
    int _SetBuzzerState(const Hal_Data& inData, Hal_Data& outData);
    int _GetAngleInfo(const Hal_Data& inData, Hal_Data& outData);
    int _SetUavStatus(const Hal_Data& inData, Hal_Data&);
    int _GetAngleInfoEx(const Hal_Data& inData, Hal_Data& outData);
    int _SendALinkMsg(const Hal_Data& inData, Hal_Data& outData);
    int _SetLedStatus(const Hal_Data& inData, Hal_Data& outData);
    int _GetWifiStatus(const Hal_Data&, Hal_Data& outData);
    int _SetWifiStatus(const Hal_Data&, Hal_Data& outData);
    int _SetRFEnable(const Hal_Data&, Hal_Data& outData);
    int _GetRFEnable(const Hal_Data&, Hal_Data& outData);
    int _GetDeviceHardWareInfo(const Hal_Data&, Hal_Data& outData);

    void _GetFanSpeed(void);
    void _SetFanSpeed(const std::uint8_t fanSpeed);
    void _FanCtrlThread(void);
private:
    std::string _GetWifiName(void);
    void _SetWifiEnable(const bool enable);
    void _SwitchPE42442RFProtectChannel();
    void _RecoverPE42442RFChannel() const;

    void _Switch9361RFProtectChannel();
    void _Recover9361RFChannel();

    hal::driver::AAPSInfo _aapsInfoData;
    std::shared_ptr<hal::driver::I2CBus> _i2cBus1;
    std::shared_ptr<hal::driver::I2CBus> _i2cBus3;

    std::shared_ptr<hal::driver::Tmp75c> _tmp75c;
    std::shared_ptr<hal::driver::Ads1115> _ads1115;
    std::shared_ptr<hal::driver::LiBattery> _battery;
    std::uint8_t _batteryRemainCapacity;
    std::uint8_t _batteryStatus;

    PE42442_CTRL_STRUCT _pe42442Ctrl;
    uint8_t _rf_enable;

    std::thread  _fanCtrlThread;

    std::int16_t _ads1115Chan0Temp;
    std::int16_t _ads1115Chan1Temp;
    std::uint16_t _tmp75cTemp;
    std::uint8_t _lastFanSpeed;

};

#endif
