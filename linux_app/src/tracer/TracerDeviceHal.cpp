#include "TracerDeviceHal.h"
#include "TracerCommonHal.h"
#include "TracerMainHalMgr.h"

#include "TracerUtils_c.h"
#include "hal/c2_alink/check/checksum.h"
#include "hal/c2_alink/check/alink_check.h"
#include "AlinkCmdproc.h"
#include "../eap/alink_pb/TracerUavMsg.pb.h"
#include "../app/alg/det_alg/sfAirBorneSniffer.h"
#include "hal/tmp75c/tmp75c.hpp"
#include "hal/ads1115/ads1115.hpp"

#include <sstream>
#include <unistd.h>

#ifndef __UNITTEST__
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#endif

extern "C" {

// Tracer量产新按键板把告警拨码开关有4档改为2档,开启状态下由C2下发控制
static uint8_t key_board_type = NEW_KEY_BOARD;
static WarningLevel_t _C2WarningLeve;
static WarningLevel_t _WarningLeve = WARNING_LEVEL_NONE;
static WarningLevel_t _LedState = WARNING_LEVEL_NONE;
static WarningLevel_t _MotorState = WARNING_LEVEL_NONE;
static WarningLevel_t _BuzzerState = WARNING_LEVEL_NONE;
static uint32_t _WarningPeriodTime[EAP_WARNING_LEVEL_MAX] = {1000, 800, 400, 200};
// 隐蔽模式0-不开启; 1-关闭所有LED灯; 每次重启后需要C2重新设置
static uint8_t _CovertMode = 0;

// 支持调节蜂鸣器音量(效果不明显)
static int _BuzzerVolume[EAP_WARNING_LEVEL_MAX] = {0x0, 50*200, 100*200, 150*200};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
    #define GYRO_SERIAL_PORT_DEFAULT "/dev/ttyUL6"  
    static char *sg_gyro_dev = GYRO_SERIAL_PORT_DEFAULT;
#pragma GCC diagnostic pop

#define POWER_BUTTON_SERIAL_PORT_DEFAULT "/dev/ttyPS1"
//static char *sg_powerbutton_dev = POWER_BUTTON_SERIAL_PORT_DEFAULT;
static const char *sg_powerbutton_dev = NULL;


#define FLAG_DEBUG_DISABLE_BUZZ_MOTOR "/run/media/mmcblk0p1/flag_debug_disable_buzz_motor"
static std::uint8_t g_flag_debug_disable_buzz_motor = 0;

std::mutex g_RFMutex;

std::atomic<bool> fanThreadFlag(true);

#ifndef __UNITTEST__
#include "ev.h"
#include "gyro.h"
#include "powerbutton.h"

static struct ev_loop *loop = NULL;
static ev_timer led_timer;
static ev_timer motor_timer;
static ev_timer buzzer_timer;

static void * sg_gyro_handle   = NULL;
static void * sg_powerbutton_handle = NULL;

void *devhal_thread(void* arg);
uint8_t Get2SwitchKey();
uint8_t Get4SwitchKey();
uint8_t GetKeyBoardType();
void led_timer_cb(EV_P_ ev_timer *w, int revents);
void motor_timer_cb(EV_P_ ev_timer *w, int revents);
void buzzer_timer_cb(EV_P_ ev_timer *w, int revents);
void warning_timer_reset(uint8_t dev_type);
void *warning_indicate(void* arg);

#else

uint8_t Get2SwitchKey() {return 0;};
uint8_t Get4SwitchKey() {return 0;};
uint8_t GetKeyBoardType() {return 0;};
void warning_timer_reset(uint8_t dev_type) {return;};
void *warning_indicate(void* arg) {return NULL;};

#endif
}

#include "json_pb_converter.h"

int TracerDeviceHal::_GetBatteryCapacity(const Hal_Data& inData, Hal_Data& outData)
{
    Drv_GetBatteryInfo *batteryInfo = (Drv_GetBatteryInfo *)(outData.buf);

    // 0x00_8001_0000----0x00_8001_1FFF：BAT I2C 电池电量获取 - 待定
    // batteryInfo->batteryCapacity = axi_read_data(PL_BATTERY_CAPACITY);
    batteryInfo->batteryCapacity = _batteryRemainCapacity;
    batteryInfo->batAndAdapterState = _batteryStatus;

    return EAP_SUCCESS;
}

int TracerDeviceHal::_GetSilentMode(const Hal_Data& inData, Hal_Data& outData)
{
    uint8_t *value = OUTDATA_U8(outData);

    switch(get_device_type()){
        case BOARD_TYPE_T2:
        case BOARD_TYPE_T3:
            *value = Get2SwitchKey();
            break;
        case BOARD_TYPE_T3_1:
            *value = 0;
            break;
        default:
            EAP_LOG_ERROR("get board type err!\n");
            return EAP_FAIL;
    }

    return EAP_SUCCESS;
}

int TracerDeviceHal::_GetAlarmMode(const Hal_Data& inData, Hal_Data& outData)
{
    uint8_t *value = OUTDATA_U8(outData);

    switch(get_device_type()){
        case BOARD_TYPE_T2:
        case BOARD_TYPE_T3:
            if (key_board_type == NEW_KEY_BOARD)
            {
                if (Input_GetValue(MIO_SWITCH_MODE0) == 0x01)
                    _WarningLeve = WARNING_LEVEL_NONE;
                else
                    _WarningLeve = _C2WarningLeve;
                *value = _WarningLeve;
            }
            else
                *value = Get4SwitchKey();
            break;
        case BOARD_TYPE_T3_1:
            *value = WARNING_LEVEL_NONE;
            break;
        default:
            EAP_LOG_DEBUG("get board type err!\n");
            return -1;
    }

    return EAP_SUCCESS;
}

int TracerDeviceHal::_SetAlarmMode(const Hal_Data& inData, Hal_Data& outData)
{
    _C2WarningLeve = (WarningLevel_t)INDATA_U8(inData);
    saveWarningLevelToUserCfg(_C2WarningLeve);
    EAP_LOG_DEBUG("_C2WarningLeve=%d\n", _C2WarningLeve);

    return EAP_SUCCESS;
}

int TracerDeviceHal::_SetCovertMode(const Hal_Data& inData, Hal_Data& outData)
{
    _CovertMode = INDATA_U8(inData);
    // 开启隐蔽模式关闭所有LED灯，退出隐蔽模式恢复系统状态灯，其他灯走正常流程
    if (_CovertMode)
    {
        GPIO_OutputCtrl(EMIO_SYS_LED0, 0);
        GPIO_OutputCtrl(EMIO_SYS_LED1, 0);
        GPIO_OutputCtrl(EMIO_SYS_LED2, 0);
        GPIO_OutputCtrl(EMIO_SYS_LED3, 0);
    }
    else
        GPIO_OutputCtrl(EMIO_SYS_LED0, 1);
    EAP_LOG_DEBUG("_CovertMode=%d\n", _CovertMode);

    return EAP_SUCCESS;
}

int TracerDeviceHal::_SetLedState(const Hal_Data& inData, Hal_Data& outData)
{
    uint8_t warning_level = INDATA_U8(inData);

    if (warning_level < EAP_WARNING_LEVEL_MAX)
        _LedState = (WarningLevel_t)warning_level;
    else
        printf("unsupported led warning level: %d\n", warning_level);

    warning_timer_reset(DEV_LED);

    return EAP_SUCCESS;
}

int TracerDeviceHal::_SetMotorState(const Hal_Data& inData, Hal_Data& outData)
{
    uint8_t warning_level = INDATA_U8(inData);

    if (warning_level < EAP_WARNING_LEVEL_MAX)
        _MotorState = (WarningLevel_t)warning_level;
    else
        printf("unsupported motor warning level: %d\n", warning_level);

    warning_timer_reset(DEV_MOTOR);

    return EAP_SUCCESS;
}

int TracerDeviceHal::_SetBuzzerState(const Hal_Data& inData, Hal_Data& outData)
{
    uint8_t warning_level = INDATA_U8(inData);

    if (warning_level < EAP_WARNING_LEVEL_MAX)
        _BuzzerState = (WarningLevel_t)warning_level;
    else
        printf("unsupported buzzer warning level: %d\n", warning_level);

    warning_timer_reset(DEV_BUZZER);

    return EAP_SUCCESS;
}

int TracerDeviceHal::_GetAngleInfo(const Hal_Data& inData, Hal_Data& outData)
{
    int status = EAP_SUCCESS;
    int ret;
    Drv_AngleInfo *angleinfo= (Drv_AngleInfo*)(outData.buf);

#ifndef __UNITTEST__
    ret = gyro_get_angle_info(angleinfo,1);
    if(ret < 0)
        status = EAP_FAIL;
    else
        status = EAP_SUCCESS;
#endif

    return status;
}

int TracerDeviceHal::_GetWifiStatus(const Hal_Data&, Hal_Data& outData)
{
    static std::uint32_t counter = 0;
    static std::uint8_t isLinkUp = 0;

    int ret = EAP_FAIL;

    counter++;

#ifndef __UNITTEST__
    if (1 == (counter % 5))
    {
        if (outData.length >= 1)
        {
            outData.buf[0] = 0;  // initialize to 0: linkdown

            int sock = socket(AF_INET, SOCK_DGRAM, 0);
            if (sock < 0)
            {
                std::cout << "socket error" << std::endl;
                ret = EAP_FAIL;
            }
            else
            {
                struct ifreq ifr;
                strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ);

                if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
                {
                    //wifi interface may not create
                }
                else
                {
                    if (ifr.ifr_flags & IFF_UP)
                    {
                        isLinkUp = 1;
                        outData.buf[0] = 1;
                    }
                    else
                    {
                        isLinkUp = 0;
                        outData.buf[0] = 0;
                    }

                    ret = EAP_SUCCESS;
                }

                close(sock);
            }
        }
    }
    else
    {
        outData.buf[0] = isLinkUp;
    }
#endif

    return ret;
}

int TracerDeviceHal::_SetWifiStatus(const Hal_Data& inData, Hal_Data& outData)
{
    bool status = false;;
    if (inData.length >= 1)
    {
        status = (inData.buf[0] == 0) ? false : true;

        _SetWifiEnable(status);
    }

    return EAP_SUCCESS;
}

void TracerDeviceHal::_SetWifiEnable(const bool enable)
{
    if (!enable)
    {
        system("sudo /usr/bin/killall wpa_supplicant udhcpc &");
    }

    GPIO_OutputCtrl(EMIO_RN440_WL_DIS, enable);
    GPIO_OutputCtrl(EMIO_RN440_BT_DIS, enable);
    GPIO_OutputCtrl(EMIO_RG450_BT_EN, enable);
    GPIO_OutputCtrl(EMIO_RG450_WL_EN, enable);

    if (enable)
    {
        std::string command = "sudo /usr/sbin/wpa_supplicant -D nl80211 -i ";
        command += _GetWifiName();
        command += " -c /etc/wpa_supplicant.conf &";
        system(command.c_str());

        command = "sudo /sbin/udhcpc -i ";
        command += _GetWifiName();
        command += " &";

        system(command.c_str());;
    }
}

std::string TracerDeviceHal::_GetWifiName()
{
    std::string wlanName = "wlan0";
    FILE *file;
    char buffer[100];
    memset(buffer, 0, 100);

    file = fopen("/tmp/ifname_wifi", "r");
    if (file != NULL)
    {
        fgets(buffer, sizeof(buffer), file);
        fclose(file);

        wlanName = buffer;
    }

    return wlanName;
}

void TracerDeviceHal::Init()
{
    if (access(FLAG_DEBUG_DISABLE_BUZZ_MOTOR, F_OK) == 0)
    {
        g_flag_debug_disable_buzz_motor = 1;
    }
    else
    {
        g_flag_debug_disable_buzz_motor = 0;
    }   

    pthread_t thread;
    pthread_create(&thread, NULL, warning_indicate, NULL);
    pthread_setname_np(thread, "warning_indicate");

    // 灯带LED1（异常显示灯+电源显示灯，双色）：开机后常亮蓝色；异常状态时，蓝色关闭，红色打开；
    // 灯带LED2（电源指示灯）：开机没检测到无人机时，不亮；检测到无人机时蓝灯闪烁；
    // 灯带LED3（连接指示灯）：开机后不亮，连接指示灯，连接USB、WiFi或图传设备成功后，常亮蓝灯
    GPIO_OutputCtrl(EMIO_SYS_LED0, 1);  // LED 1 blue on
    GPIO_OutputCtrl(EMIO_SYS_LED1, 0);  // LED 1 red off
    GPIO_OutputCtrl(EMIO_SYS_LED2, 0);  // UAV status off
    GPIO_OutputCtrl(EMIO_SYS_LED3, 0);  // Link status off

    if (ProductSwCfgGet().issupportairborne())
    {
        _SetWifiEnable(false);
    }

    auto threadFunc = std::bind(&TracerDeviceHal::_FanCtrlThread, this);

    _fanCtrlThread = std::move(std::thread(threadFunc));
    _fanCtrlThread.detach();
}

TracerDeviceHal::TracerDeviceHal(HalMgr* halMgr) : HalBase(halMgr)
{
    sg_powerbutton_dev = GetTtyName_Mcu2Ps();
    //printf("%s():****sg_powerbutton_dev='%s'.\n", __FUNCTION__, sg_powerbutton_dev);
    _Register(EAP_DRVCODE_GET_BATTERY_CAPACITY, (Hal_Func)&TracerDeviceHal::_GetBatteryCapacity);
    _Register(EAP_DRVCODE_GET_SILENT_MODE, (Hal_Func)&TracerDeviceHal::_GetSilentMode);
    _Register(EAP_DRVCODE_GET_ALARM_MODE, (Hal_Func)&TracerDeviceHal::_GetAlarmMode);
    _Register(EAP_DRVCODE_SET_ALARM_MODE, (Hal_Func)&TracerDeviceHal::_SetAlarmMode);
    _Register(EAP_DRVCODE_SET_COVERT_MODE, (Hal_Func)&TracerDeviceHal::_SetCovertMode);
    _Register(EAP_DRVCODE_SET_LED_STATE, (Hal_Func)&TracerDeviceHal::_SetLedState);
    _Register(EAP_DRVCODE_SET_MOTOR_STATE, (Hal_Func)&TracerDeviceHal::_SetMotorState);
    _Register(EAP_DRVCODE_SET_BUZZER_STATE, (Hal_Func)&TracerDeviceHal::_SetBuzzerState);
    _Register(EAP_DRVCODE_GET_ANGLE_INFO, (Hal_Func)&TracerDeviceHal::_GetAngleInfo);
    _Register(EAP_DRVCODE_SET_UAV_STATUS, (Hal_Func)&TracerDeviceHal::_SetUavStatus);
    _Register(EAP_DRVCODE_GET_ANGLE_INFO_EX, (Hal_Func)&TracerDeviceHal::_GetAngleInfoEx);
    _Register(EAP_DRVCODE_SEND_ALINK_MSG, (Hal_Func)&TracerDeviceHal::_SendALinkMsg);
    _Register(EAP_DRVCODE_SET_LED_STATUS, (Hal_Func)&TracerDeviceHal::_SetLedStatus);
    _Register(EAP_DRVCODE_GET_WIFI_STATUS, (Hal_Func)&TracerDeviceHal::_GetWifiStatus);
    _Register(EAP_DRVCODE_SET_WIFI_STATUS, (Hal_Func)&TracerDeviceHal::_SetWifiStatus);
    _Register(EAP_DRVCODE_SET_RF_ENABLE, (Hal_Func)&TracerDeviceHal::_SetRFEnable);
    _Register(EAP_DRVCODE_GET_RF_ENABLE, (Hal_Func)&TracerDeviceHal::_GetRFEnable);
    _Register(EAP_DRVCODE_GET_DEVICE_HARDWARE_INFO, (Hal_Func)&TracerDeviceHal::_GetDeviceHardWareInfo);

    _batteryRemainCapacity = 100;
    _batteryStatus = BATTERY_DISCHARGE;
    // Tracer-P版本需要将#2天线接到RN440芯片供RemoteID用
    if (ProductSwCfgGet().issupportremoteid())
    {
        _pe42442Ctrl._v1 = 1;
        _pe42442Ctrl._v2 = 0;
        _pe42442Ctrl._v3 = 0;
    }
    // 频谱侦测版本将#2天线接到射频板
    else if (ProductSwCfgGet().issupportspectrum() || ProductSwCfgGet().issupportairborne())
    {
        _pe42442Ctrl._v1 = 0;
        _pe42442Ctrl._v2 = 0;
        _pe42442Ctrl._v3 = 1;
    }

    _i2cBus1 = std::shared_ptr<hal::driver::I2CBus>(new hal::driver::I2CBus("/dev/i2c-1"));
    _tmp75c  = std::shared_ptr<hal::driver::Tmp75c>(new hal::driver::Tmp75c(_i2cBus1, 0x48));
    _ads1115 = std::shared_ptr<hal::driver::Ads1115>(new hal::driver::Ads1115(_i2cBus1, 0x49));

    _i2cBus3 = std::shared_ptr<hal::driver::I2CBus>(new hal::driver::I2CBus("/dev/i2c-3"));
    _battery = std::shared_ptr<hal::driver::LiBattery> (new hal::driver::LiBattery(_i2cBus3, 0x55));

    _ads1115Chan0Temp = 0; 
    _ads1115Chan1Temp = 0;
    _tmp75cTemp = 0;
    _lastFanSpeed = 0;
}

TracerDeviceHal::~TracerDeviceHal()
{
    fanThreadFlag = false;
}

int TracerDeviceHal::_SetUavStatus(const Hal_Data& inData, Hal_Data&)
{
    if (false == _aapsInfoData.setData(hal::driver::ENUM_AAPS_TYPE, inData.buf, inData.length))
    {
        return EAP_FAIL;
    }

    return EAP_SUCCESS;
}

// for debug perpose, send a simulate ALink message
int TracerDeviceHal::_SendALinkMsg(const Hal_Data& inData, Hal_Data&)
{
    //std::cout << "inData.length = " << std::dec << inData.length 
    //          << ", data: " << (std::uint32_t)inData.buf[0] 
    //          << ", " << (std::uint32_t)inData.buf[1] << std::endl;

    std::uint8_t msgType = 0;
    std::uint8_t channel = 0;
    std::uint32_t p1 = 2;
    std::uint32_t p2 = 2;
    std::uint32_t p3 = 0x1234;

    (void)channel;

    if (inData.length >= 2)
    {
        channel = inData.buf[0];
        msgType = inData.buf[1];
    }

    if (inData.length >= 14)
    {
        channel = inData.buf[0];
        msgType = inData.buf[1];

        p1 = ((((std::uint32_t)inData.buf[2] << 24) & 0xFF000000)
            | (((std::uint32_t)inData.buf[3] << 16) & 0xFF0000)
            | (((std::uint32_t)inData.buf[4] << 8) & 0xFF00)
            | (((std::uint32_t)inData.buf[5]) & 0xFF));

        p2 = ((((std::uint32_t)inData.buf[6] << 24) & 0xFF000000)
            | (((std::uint32_t)inData.buf[7] << 16) & 0xFF0000)
            | (((std::uint32_t)inData.buf[8] << 8) & 0xFF00)
            | (((std::uint32_t)inData.buf[9]) & 0xFF));

        p3 = ((((std::uint32_t)inData.buf[10] << 24) & 0xFF000000)
            | (((std::uint32_t)inData.buf[11] << 16) & 0xFF0000)
            | (((std::uint32_t)inData.buf[12] << 8) & 0xFF00)
            | (((std::uint32_t)inData.buf[13]) & 0xFF));
    }    

    std::uint8_t buffer[512] = {0};
    const std::uint32_t maxLength = 512;
    bool result = false;
    protobuf::traceruav::TracerUavMessagePB uavMsgData;

    if (0 == msgType)
    {
        uavMsgData.set_msgchannel("");
        uavMsgData.set_msgtype(protobuf::traceruav::TRACERUAVMSG_HEART_BEAT);
        uavMsgData.set_msgname("heartbeat");
        uavMsgData.set_msghandle(0);
        uavMsgData.set_msgseq(1);
    }
    else if (1 == msgType)
    {
        protobuf::traceruav::CtrlWorkModePB workMode;
        workMode.set_workmode(p1);
        workMode.set_dxnum(p2);
        workMode.set_dxfreq(p3);

        uavMsgData.set_msgchannel("");
        uavMsgData.set_msgtype(protobuf::traceruav::TRACERUAVMSG_WORK_MODE_TYPE);
        uavMsgData.set_msgname("workmode");
        uavMsgData.set_msghandle(0);
        uavMsgData.set_msgseq(1);
        uavMsgData.mutable_workmode()->CopyFrom(workMode);
    }
    else if (3 == msgType)
    {
        protobuf::traceruav::MeasureInfoPb meaInfoData;
        meaInfoData.set_workstatus(1);
        meaInfoData.set_tarnum(2);
        
        protobuf::traceruav::MeasureInfoPb_MeasureInfoData *meaData = meaInfoData.add_meainfodata();
        meaData->set_tnum(29);
        meaData->set_tfreq(5745);
        meaData->set_tamp(12669);
        meaData->set_tmfw(0);
        meaData->set_tmfy(0);
        meaData->set_tffw(163);
        meaData->set_tffy(179);
        
        protobuf::traceruav::MeasureInfoPb_MeasureInfoData *meaData2 = meaInfoData.add_meainfodata();
        meaData2->set_tnum(32);
        meaData2->set_tfreq(2420);
        meaData2->set_tamp(12771);
        meaData2->set_tmfw(0);
        meaData2->set_tmfy(0);
        meaData2->set_tffw(0);
        meaData2->set_tffy(0);

        uavMsgData.set_msgchannel("");
        uavMsgData.set_msgtype(protobuf::traceruav::TRACERUAVMSG_MEASURE_INFO_TYPE);
        uavMsgData.set_msgname("meaInfoData");
        uavMsgData.set_msghandle(0);
        uavMsgData.set_msgseq(1);
        uavMsgData.mutable_measureinfo()->CopyFrom(meaInfoData);
    }
    else
    {
        protobuf::traceruav::FlyStatusPB flyData;// = new protobuf::traceruav::FlyStatusPB;
        flyData.set_yaw(0);
        flyData.set_pitch(0);
        flyData.set_roll(0);
        flyData.set_wy(0);
        flyData.set_wp(0);
        flyData.set_wr(0);
        flyData.set_longitude(0);
        flyData.set_latitude(0);
        flyData.set_altitude(0);
        flyData.set_vx(0);
        flyData.set_vy(0);
        flyData.set_vz(0);
        
        uavMsgData.set_msgchannel("");
        uavMsgData.set_msgtype(protobuf::traceruav::TRACERUAVMSG_FLY_STATUS_TYPE);
        uavMsgData.set_msgname("flystatus");
        uavMsgData.set_msghandle(0);
        uavMsgData.set_msgseq(1);
        uavMsgData.mutable_flystatus()->CopyFrom(flyData);
    }

    const std::size_t dataLength = uavMsgData.ByteSizeLong();
    // std::cout << "dataLength = " << std::dec << dataLength << std::endl;

    if (dataLength <= maxLength)
    {
        result = uavMsgData.SerializeToArray(buffer + 9, dataLength);
        (void)result;

        alink_msg_head_t msgHeader;
        msgHeader.magic    = 0xFD;
        msgHeader.len_lo   = (dataLength & 0x00FF);
        msgHeader.len_hi   = (dataLength & 0xFF00) >> 8;
        msgHeader.seq      = 0x0;
        msgHeader.destid   = DEV_TRACER;  //dest id
        msgHeader.sourceid = DEV_UAV;     //source id
        if ((3 == msgType) || (0 == msgType))
        {
            msgHeader.destid   = DEV_UAV;    //dest id
            msgHeader.sourceid = DEV_TRACER; //source id
        }

        msgHeader.msgid    = EAP_ALINK_CMD_TRACER_UAV_MESSAGE;
        msgHeader.ans      = 0;
        msgHeader.checksum = 0;

        uint8_t *pkt = (uint8_t *)&msgHeader;
        uint8_t i = 0;

        for (i = 0; i < ALINK_CORE_HEADER_LEN; i++)
        {
            msgHeader.checksum += pkt[i];
        }

        memcpy(buffer, &msgHeader, 9);

        std::uint16_t crc = crc_calculate(buffer + 1, dataLength + sizeof(alink_msg_head_t) - 1);
        buffer[dataLength + 9] = crc & 0xFF;
        buffer[dataLength + 10] = (crc & 0xFF00) >> 8;

        std::ostringstream out;
        for (uint32_t pos = 0; pos <= dataLength + 10 + 4; pos++)
        {
            out << " " << std::hex << std::setw(2) << std::setfill('0') << (uint16_t)(buffer[pos]);
        }
        std::cout << "TracerDeviceHal::_SendALinkMsg dataLength = " << std::dec << dataLength
                  << ", crc = 0x" << std::hex << crc << ", " << out.str() << std::endl;

        Thread_Msg msg = {0};
        alink_msg_t* alink_msg = (alink_msg_t* )msg.buf;

        alink_msg->channelType = EAP_CHANNEL_TYPE_TYPEC;
        alink_msg->channelId = 0;
        memcpy((void*)&alink_msg->msg_head, buffer, sizeof(alink_msg_head_t) + dataLength + 2); // add 2 bytes CRC

        std::uint16_t payload_len = alink_msg->msg_head.len_hi << 8 | alink_msg->msg_head.len_lo;
        std::size_t msg_len = sizeof(alink_msg_head_t) + payload_len + sizeof(alink_data_crc_t);
        (void)msg_len;
        // bool isCRCOK = alink_check_crc((std::uint8_t *)(&alink_msg->msg_head), msg_len);
        // std::cout << "isCRCOK = " << std::boolalpha << isCRCOK << std::endl;

        // add 2 bytes CRC with 4 bytes channelType and channelId;
        SendAsynMsg(EAP_CMD_ALINK, (uint8_t *)&msg, sizeof(alink_msg_head_t) + dataLength + 6);
        // send_alinkmsg_to_eap(EAP_CHANNEL_TYPE_TYPEC, 0, (uint8_t*)buffer, sizeof(alink_msg_head_t) + dataLength);
    }

    return EAP_SUCCESS;
}

int TracerDeviceHal::_SetLedStatus(const Hal_Data& inData, Hal_Data&)
{
    OUTPUT_NAME ledId;
    std::uint8_t ledNo;
    std::uint8_t status;

    if (inData.length >= 2)
    {
        ledNo = inData.buf[0];
        status = inData.buf[1];

        switch (ledNo)
        {
            case LED_STATUS_NORMAL:
                ledId = EMIO_SYS_LED0;
                break;
            case LED_STATUS_ABNORMAL:
                ledId = EMIO_SYS_LED1;
                break;
            case LED_STATUS_LINK:
                ledId = EMIO_SYS_LED3;
                break;
            default:
                return EAP_FAIL;
        }
        if (_CovertMode)
            status = 0;
        GPIO_OutputCtrl(ledId, status);
    }

    return EAP_SUCCESS;
}

// protobuf::traceruav::FlyStatusPB
// aapsData.ParseFromArray(buff, length);
int TracerDeviceHal::_GetAngleInfoEx(const Hal_Data& inData, Hal_Data& outData)
{
    protobuf::traceruav::FlyStatusPB data;

    if (false == _aapsInfoData.getAAPSData(data))
    {
        return EAP_FAIL;
    }

    ioFlyStatus_t flyStatus;

    flyStatus.yaw       = data.yaw();
    flyStatus.pitch     = data.pitch();
    flyStatus.roll      = data.roll();
    flyStatus.wY        = data.wy();
    flyStatus.wP        = data.wp();
    flyStatus.wR        = data.wr();
    flyStatus.longitude = data.longitude();
    flyStatus.latitude  = data.latitude();
    flyStatus.altitude  = data.altitude();
    flyStatus.vX        = data.vx();
    flyStatus.vY        = data.vy();
    flyStatus.vZ        = data.vz();

    memcpy(outData.buf, &flyStatus, outData.length);

    return EAP_SUCCESS;
}

// 应李瑶需求, 温控转速区间改为45°-70°
// 测试数据2023-10-13
// temp = 45, fanSpeed = 5, lastFanSpeed = 5
// temp = 46, fanSpeed = 6, lastFanSpeed = 6
// temp = 50, fanSpeed = 6, lastFanSpeed = 6
// temp = 51, fanSpeed = 7, lastFanSpeed = 7
// temp = 55, fanSpeed = 7, lastFanSpeed = 7
// temp = 56, fanSpeed = 8, lastFanSpeed = 8
// temp = 60, fanSpeed = 8, lastFanSpeed = 8
// temp = 61, fanSpeed = 9, lastFanSpeed = 9
// temp = 65, fanSpeed = 9, lastFanSpeed = 9
// temp = 66, fanSpeed = 10, lastFanSpeed = 10

// 测试数据2023-10-07
// temp = 65, fanSpeed = 5, lastFanSpeed = 6
// temp = 66, fanSpeed = 6, lastFanSpeed = 6
// temp = 69, fanSpeed = 6, lastFanSpeed = 6
// temp = 70, fanSpeed = 7, lastFanSpeed = 7
// temp = 73, fanSpeed = 7, lastFanSpeed = 7
// temp = 74, fanSpeed = 8, lastFanSpeed = 8
// temp = 77, fanSpeed = 8, lastFanSpeed = 8
// temp = 78, fanSpeed = 9, lastFanSpeed = 9
// temp = 81, fanSpeed = 9, lastFanSpeed = 9
// temp = 82, fanSpeed = 10, lastFanSpeed = 10
// 65°及65°以下风扇转速5(约全速的45%)
// 66°-69°风扇转速4(约全速的56%)
// 70°-73°风扇转速3(约全速的67%)
// 74°-77°风扇转速2(约全速的78%)
// 78°-81°风扇转速1(约全速的89%)
// 82°及82°以上风扇转速0(FPGA寄存器值, 全速)
void TracerDeviceHal::_FanCtrlThread(void)
{
    const std::uint16_t MIN_TEMPERATURE_ADJUSMENT = 45;
    const std::uint16_t MAX_TEMPERATURE_ADJUSMENT = 65;
    const std::uint16_t FAN_SPEED_HOLD_CIRCLE = 60;

    std::uint8_t fanSpeed      = static_cast<std::uint8_t>(FAN_SPEED_CTRL::FAN_SPEED_LEV5);
    std::uint16_t fanSpeedHold = 0;

    const std::uint16_t tempDistance = MAX_TEMPERATURE_ADJUSMENT - MIN_TEMPERATURE_ADJUSMENT;

    std::uint8_t fanSpeedDistance = static_cast<std::uint8_t>(FAN_SPEED_CTRL::FAN_SPEED_LEV10
                                        - FAN_SPEED_CTRL::FAN_SPEED_LEV5);

    std::uint16_t lastTemperature = _tmp75c->getTemperature();

    std::uint16_t tempChange = 0;
    std::uint16_t count      = 0;

    //std::uint16_t revert = 0;
    //std::uint16_t temperature = 0;
    pthread_setname_np(pthread_self(), "_FanCtrlThread");

    while (fanThreadFlag)
    {
        sleep(1);
        count++;

        // 每5秒更新电池剩余电量
        if ((count % 5) == 1)
        {
            std::uint16_t remainCapacity = 0;
            bool result = _battery->getRelativeStateOfCharge(remainCapacity);
            if (true == result)
            {
                _batteryRemainCapacity = static_cast<std::uint8_t>(remainCapacity);
                /*std::cout << std::endl << "********** BatteryRemainCapacity = " << std::dec 
                          << (std::uint16_t)_batteryRemainCapacity << std::endl;*/
            }
            // 根据电流判断供电方式
            std::int16_t current = 0;
            result = _battery->getCurrent(current);
            if (true == result)
            {
                if (current > 0)
                    _batteryStatus = BATTERY_CHARGE;
                else
                    _batteryStatus = BATTERY_DISCHARGE;
            }
            else
                _batteryStatus = POWER_ADAPTER;
        }

        // Debug code
        // if (0 == revert){temperature++;}else{temperature--;}
        // if (temperature > 100){revert = 1;} if ((0 != revert) && (temperature < 5)){revert = 0;}

        std::uint16_t tempDeviation = 0;

        _ads1115Chan0Temp = _ads1115->getTemperature(hal::driver::Ads1115Channel::ADS1115_CHANNEL_0);
        _ads1115Chan1Temp = _ads1115->getTemperature(hal::driver::Ads1115Channel::ADS1115_CHANNEL_1);
        _tmp75cTemp = _tmp75c->getTemperature();
        std::uint16_t ads1115Temp = 0;

        if ((_ads1115Chan0Temp >= _ads1115Chan1Temp) && (_ads1115Chan0Temp > 0))
        {
            ads1115Temp = _ads1115Chan0Temp;
        }
        else if ((_ads1115Chan1Temp >= _ads1115Chan0Temp) && (_ads1115Chan1Temp > 0))
        {
            ads1115Temp = _ads1115Chan1Temp;
        }

        const std::uint16_t temperature = std::max(ads1115Temp, _tmp75cTemp);
        if (temperature > lastTemperature)
        {
            tempDeviation = temperature - lastTemperature;
        }
        else
        {
            tempDeviation = lastTemperature - temperature;
        }

        lastTemperature = temperature;

        // std::cout << std::dec << "ads1115Chan0Temp = " << ads1115Chan0Temp << ", ads1115Chan1Temp = " << ads1115Chan1Temp
        // << ", tmp75cTemp = " << tmp75cTemp << ", ads1115Temp = " << ads1115Temp 
        // << ", temperature = " << temperature << std::endl;

        // filter big temperature change 3 times
        if ((tempDeviation > 5) && (tempChange < 3))
        {
            tempChange++;
            continue;
        }

        tempChange = 0;

        if (temperature <= MIN_TEMPERATURE_ADJUSMENT)
        {
            fanSpeed = FAN_SPEED_CTRL::FAN_SPEED_LEV5;
        }
        else if (temperature >= MAX_TEMPERATURE_ADJUSMENT)
        {
            fanSpeed = FAN_SPEED_CTRL::FAN_SPEED_LEV10;
        }
        else
        {
            std::uint16_t caculateTemp = (temperature - MIN_TEMPERATURE_ADJUSMENT) * fanSpeedDistance;
            fanSpeed = caculateTemp / tempDistance;
            if (0 != (caculateTemp % tempDistance))
            {
                fanSpeed += 1;
            }

            fanSpeed += static_cast<std::uint8_t>(FAN_SPEED_CTRL::FAN_SPEED_LEV5);
            if (fanSpeed > FAN_SPEED_CTRL::FAN_SPEED_LEV10)
            {
                fanSpeed = FAN_SPEED_CTRL::FAN_SPEED_LEV10;
            }
        }

        // increase at once, but decrease after 60s
        if (fanSpeed > _lastFanSpeed)
        {
            _lastFanSpeed = fanSpeed;
            _SetFanSpeed(_lastFanSpeed);
        }
        else if (fanSpeed < _lastFanSpeed)
        {
            if (fanSpeedHold < FAN_SPEED_HOLD_CIRCLE)
            {
                fanSpeedHold++;
            }
            else
            {
                fanSpeedHold = 0;
                _lastFanSpeed = fanSpeed;
                _SetFanSpeed(_lastFanSpeed);
            }
        }

        //std::cout <<"temp = " << temperature << ", fanSpeed = " << (std::uint32_t)fanSpeed
        //<< ", lastFanSpeed = " << (std::uint32_t)lastFanSpeed << std::endl;
        //_GetFanSpeed();
    }

    return;
}

void TracerDeviceHal::_SetFanSpeed(const std::uint8_t fanSpeed)
{
    if (static_cast<std::uint8_t>(FAN_SPEED_LEV10) >= fanSpeed)
    {
        // FPGA value 0-9, 0 is highest, 9 is stopped
        int value = static_cast<std::uint8_t>(FAN_SPEED_LEV10) - fanSpeed;
        axi_write_data(PL_FAN_PWM_RATE, value);

        //auto speed = axi_read_data(PL_FAN_PWM_RATE);
        //std::cout << "_SetFanSpeed to = " << speed << std::endl;
    }

    return;
}

void TracerDeviceHal::_GetFanSpeed(void)
{
    auto speed1 = axi_read_data(PL_I_FPGA_TEMP1);
    auto speed2 = axi_read_data(PL_I_FPGA_CURR0);

    std::cout << "speed1 = " << speed1 << ", speed2 = " << speed2 << std::endl;

    return;
}

int TracerDeviceHal::_SetRFEnable(const Hal_Data& inData, Hal_Data&)
{
    int result          = EAP_FAIL;
    std::uint8_t status = 0;

    if (inData.length >= 1)
    {
        status = inData.buf[0];
        std::lock_guard<std::mutex> lock(g_RFMutex);

        if (RF_DISABLE == status)
        {
            // 顺序关闭业务、射频、天线
            EapGetSys().SetSysStatus(SYS_HIT_TARGET_STATUS);
            _Switch9361RFProtectChannel();
            _SwitchPE42442RFProtectChannel();
            result = EAP_SUCCESS;
            EAP_LOG_DEBUG("set _rf_enable=%u\n", _rf_enable);
        }
        else if (RF_ENABLE == status)
        {
            // 逆序恢复天线、射频、业务
            _RecoverPE42442RFChannel();
            _Recover9361RFChannel();
            EapGetSys().SetSysStatus(SYS_DETECTION_TARGET_STATUS);
            // 频谱侦测需要重新恢复切频
            if (ProductSwCfgGet().issupportspectrum() || ProductSwCfgGet().issupportairborne())
                SendAsynData(EAP_CMD_FREQUENCY, 0, 0);
            result = EAP_SUCCESS;
            EAP_LOG_DEBUG("set _rf_enable=%u\n", _rf_enable);
        }
    }

    return result;
}

// Mode      V3,V2,V1
//RF1 on      001      --- 射频信号到 RN440
//RF2 on      010      --- 射频信号旁路(保护)
//RF3 on      011      --- 未用
//RF4 on      100      --- 射频信号到 射频板
int TracerDeviceHal::_GetRFEnable(const Hal_Data&, Hal_Data& outData)
{
    if (outData.length >= 1)
    {
        outData.buf[0] = _rf_enable;
        EAP_LOG_DEBUG("get _rf_enable=%u\n", _rf_enable);
    }

    return EAP_SUCCESS;
}
int TracerDeviceHal::_GetDeviceHardWareInfo(const Hal_Data&, Hal_Data& outData)
{
    int angle_status = EAP_SUCCESS;
    Drv_AngleInfo angleInfo = {0};
    Hal_Data angleInfo_outData = {sizeof(Drv_AngleInfo), (uint8_t*)&angleInfo};
    Drv_AngleInfo *angleinfo= (Drv_AngleInfo*)(angleInfo_outData.buf);
    int ret = -1;
#ifndef __UNITTEST__
    ret = gyro_get_angle_info(angleinfo,1);
#endif
    if(ret < 0)//异常：0 正常：1
        angle_status = 0;
    else
        angle_status = 1;

    Drv_HardWareInfo* pHardWareInfo= (Drv_HardWareInfo*)(outData.buf);
    pHardWareInfo->tempFpga = _tmp75cTemp;
    pHardWareInfo->tempBatBoard1 = _ads1115Chan0Temp;
    pHardWareInfo->tempBatBoard2 = _ads1115Chan1Temp;
    pHardWareInfo->batteryCap = _batteryRemainCapacity;
    pHardWareInfo->digitComStatus = angle_status;
    pHardWareInfo->fanRateStatus = 1;
    pHardWareInfo->fanRate = _lastFanSpeed;
    pHardWareInfo->tcDevStatus = 1;
    pHardWareInfo->tcConnectStatus = 1;
    pHardWareInfo->tcFrequence = 1;
    pHardWareInfo->wifiRemoteIdDevStatus = 1;
    pHardWareInfo->wifiRemoteIdConnectStatus = 1;
    pHardWareInfo->wifiNetDevStatus = 1;
    pHardWareInfo->wifiNetConnectStatus = 1;

    return EAP_SUCCESS;
}
void TracerDeviceHal::_SwitchPE42442RFProtectChannel()
{
    // {ENUM_PE42442_CHANNEL::CHANNEL_RN440,      {0, 0, 1}}
    // {ENUM_PE42442_CHANNEL::CHANNEL_PROTECTION, {0, 1, 0}}
    // {ENUM_PE42442_CHANNEL::CHANNEL_RESERVE,    {0, 1, 1}}
    // {ENUM_PE42442_CHANNEL::CHANNEL_RFBOARD,    {1, 0, 0}}

    PE42442_CTRL_STRUCT pe42442Ctrl;
    pe42442Ctrl._v1 = Output_GetValue(EMIO_PE42442_V1);
    pe42442Ctrl._v2 = Output_GetValue(EMIO_PE42442_V2);
    pe42442Ctrl._v3 = Output_GetValue(EMIO_PE42442_V3);

    if ((pe42442Ctrl._v1 == 0)
         && (pe42442Ctrl._v2 == 1)
         && (pe42442Ctrl._v3 == 0))
    {
        // 在保护态下不执行切换
    }
    else
    {
        _pe42442Ctrl = pe42442Ctrl;

        const std::uint8_t pe42442V1 = 0;
        const std::uint8_t pe42442V2 = 1;
        const std::uint8_t pe42442V3 = 0;

        if (pe42442V1 != _pe42442Ctrl._v1)
        {
            GPIO_OutputCtrl(EMIO_PE42442_V1, pe42442V1);
        }

        if (pe42442V2 != _pe42442Ctrl._v2)
        {
            GPIO_OutputCtrl(EMIO_PE42442_V2, pe42442V2);
        }

        if (pe42442V3 != _pe42442Ctrl._v3)
        {
            GPIO_OutputCtrl(EMIO_PE42442_V3, pe42442V3);
        }
    }
}

void TracerDeviceHal::_RecoverPE42442RFChannel() const
{
    PE42442_CTRL_STRUCT pe42442Ctrl;

    pe42442Ctrl._v1 = Output_GetValue(EMIO_PE42442_V1);
    pe42442Ctrl._v2 = Output_GetValue(EMIO_PE42442_V2);
    pe42442Ctrl._v3 = Output_GetValue(EMIO_PE42442_V3);

    // 只有在保护态下才执行恢复
    if ((pe42442Ctrl._v1 == 0)
         && (pe42442Ctrl._v2 == 1)
         && (pe42442Ctrl._v3 == 0))
    {
        if (pe42442Ctrl._v1 != _pe42442Ctrl._v1)
        {
            GPIO_OutputCtrl(EMIO_PE42442_V1, _pe42442Ctrl._v1);
        }

        if (pe42442Ctrl._v2 != _pe42442Ctrl._v2)
        {
            GPIO_OutputCtrl(EMIO_PE42442_V2, _pe42442Ctrl._v2);
        }

        if (pe42442Ctrl._v3 != _pe42442Ctrl._v3)
        {
            GPIO_OutputCtrl(EMIO_PE42442_V3, _pe42442Ctrl._v3);
        }
    }
}

void TracerDeviceHal::_Switch9361RFProtectChannel()
{
    uint8_t rf_enable  = Output_GetValue(EMIO_RF_GPIO_4);

    if (RF_DISABLE == rf_enable)
    {
        // 在保护态下不执行切换
    }
    else
    {
        _rf_enable = RF_DISABLE;
        GPIO_OutputCtrl(EMIO_RF_GPIO_4, 0);
        GPIO_OutputCtrl(EMIO_RF_CTLA_0, 0);
        GPIO_OutputCtrl(EMIO_RF_CTLA_1, 0);
        GPIO_OutputCtrl(EMIO_RF_CTLA_2, 0);
        GPIO_OutputCtrl(EMIO_RF_CTLB_0, 0);
        GPIO_OutputCtrl(EMIO_RF_CTLB_1, 0);
        GPIO_OutputCtrl(EMIO_RF_CTLB_2, 0);
    }
}

void TracerDeviceHal::_Recover9361RFChannel()
{

    uint8_t rf_enable  = Output_GetValue(EMIO_RF_GPIO_4);

    // 只有在保护态下才执行恢复
    if (RF_DISABLE == rf_enable)
    {
        _rf_enable = RF_ENABLE;
        GPIO_OutputCtrl(EMIO_RF_GPIO_4, 1);
    }
}

extern "C" {
#ifndef __UNITTEST__

uint8_t Get2SwitchKey()
{
    uint8_t Status;
    Status = Input_GetValue(MIO_MUTE_SWITCH);  // 1: mute, 0: normal

    return Status;
}

uint8_t Get4SwitchKey()
{
    if (Input_GetValue(MIO_SWITCH_MODE0) == 0x01)
        _WarningLeve = WARNING_LEVEL_NONE;
    else if (Input_GetValue(MIO_SWITCH_MODE1) == 0x01)
        _WarningLeve = WARNING_LEVEL_LOW;
    else if (Input_GetValue(MIO_SWITCH_MODE2) == 0x01)
        _WarningLeve = WARNING_LEVEL_MEDIUM;
    else if (Input_GetValue(MIO_SWITCH_MODE3) == 0x01)
        _WarningLeve = WARNING_LEVEL_HIGH;
    else _WarningLeve = WARNING_LEVEL_NONE; // set to WARNING_LEVEL_NONE when no hardware indication

    return _WarningLeve;
}

uint8_t GetKeyBoardType()
{
    int key[4] = {IO_VOL};

    if (Input_GetValue(MIO_SWITCH_MODE0) == 0x01)
        key[0] = IO_VOH;
    if (Input_GetValue(MIO_SWITCH_MODE1) == 0x01)
        key[1] = IO_VOH;
    if (Input_GetValue(MIO_SWITCH_MODE2) == 0x01)
        key[2] = IO_VOH;
    if (Input_GetValue(MIO_SWITCH_MODE3) == 0x01)
        key[3] = IO_VOH;
    if (key[1] == IO_VOH && key[2] == IO_VOH && key[3] == IO_VOH)
    {
        // 新按键板只有2档位，其余档位MIO值都是1，没有按键板时也全是1
        return NEW_KEY_BOARD;
    }

    return OLD_KEY_BOARD;
}

void led_timer_cb(EV_P_ ev_timer *w, int revents)
{
    static int led_state = 0;

    // set LED
    if (_LedState && !_CovertMode)
        led_state = !led_state;
    else
        led_state = 0;
    GPIO_OutputCtrl(EMIO_SYS_LED2, led_state);

    return;
}

void motor_timer_cb(EV_P_ ev_timer *w, int revents)
{
    static int motor_state = 0;

    // set Motor
    if (_MotorState)
        motor_state = !motor_state;
    else
        motor_state = 0;

    if (g_flag_debug_disable_buzz_motor)
    {
        motor_state = 0;  // disable motor when debug flag is enabled
    }

    // 车载Tracer没有马达
    if (get_device_type() != BOARD_TYPE_T3_1)
        GPIO_OutputCtrl(EMIO_VBRATOR_EN, motor_state);

    return;
}

void buzzer_timer_cb(EV_P_ ev_timer *w, int revents)
{
    static int buzzer_state = 0;

    // set Buzzer
    if (_BuzzerState)
        buzzer_state = !buzzer_state;
    else
        buzzer_state = 0;

    if (g_flag_debug_disable_buzz_motor)
    {
        buzzer_state = 0;  // disable buzz when debug flag is enabled
    }

    // 车载Tracer没有蜂鸣器
    if (get_device_type() != BOARD_TYPE_T3_1)
    {
        axi_write_data(PL_BUZZER_PARA, _BuzzerVolume[_BuzzerState]);
        axi_write_data(PL_BUZZER_EN, buzzer_state);
    }

    return;
}

void warning_timer_reset(uint8_t dev_type)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
    switch(dev_type)
    {
        case DEV_LED:
            ev_timer_stop(loop, &led_timer);
            ev_timer_set(&led_timer, 0.0, _WarningPeriodTime[_WarningLeve]/1000.0);
            ev_timer_start(loop, &led_timer);
            break;
        case DEV_MOTOR:
            ev_timer_stop(loop, &motor_timer);
            ev_timer_set(&motor_timer, 0.0, _WarningPeriodTime[_WarningLeve]/1000.0);
            ev_timer_start(loop, &motor_timer);
            break;
        case DEV_BUZZER:
            ev_timer_stop(loop, &buzzer_timer);
            ev_timer_set(&buzzer_timer, 0.0, _WarningPeriodTime[_WarningLeve]/1000.0);
            ev_timer_start(loop, &buzzer_timer);
            break;
        default:
            break;
    }
#pragma GCC diagnostic pop
}


void *warning_indicate(void* arg)
{
    loop = ev_loop_new(EVFLAG_AUTO); 

    // 读取C2设置的告警等级，默认为低
    _C2WarningLeve = (WarningLevel_t)readWarningLevelFromUserCfg();

    // 获取拨码开关初始等级
    switch(get_device_type()){
        case BOARD_TYPE_T2:
        case BOARD_TYPE_T3:
            key_board_type = GetKeyBoardType();
            if (key_board_type == NEW_KEY_BOARD)
            {
                if (Input_GetValue(MIO_SWITCH_MODE0) == 0x01)
                    _WarningLeve = WARNING_LEVEL_NONE;
                else
                {
                    _WarningLeve = _C2WarningLeve;
                }
            }
            else
                Get4SwitchKey();
            EAP_LOG_DEBUG("key_board_type=%d, _WarningLeve=%d, _C2WarningLeve=%d\n", key_board_type, _WarningLeve, _C2WarningLeve);
            break;
        case BOARD_TYPE_T3_1:
            break;
        default:
            EAP_LOG_ERROR("get board type err!\n");
            return NULL;
    }

    // if (_WarningLeve)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
        ev_timer_init(&led_timer, led_timer_cb, 0.0, _WarningPeriodTime[_WarningLeve]/1000.0);
        ev_timer_start(loop, &led_timer);

        ev_timer_init(&motor_timer, motor_timer_cb, 0.0, _WarningPeriodTime[_WarningLeve]/1000.0);
        ev_timer_start(loop, &motor_timer);

        ev_timer_init(&buzzer_timer, buzzer_timer_cb, 0.0, _WarningPeriodTime[_WarningLeve]/1000.0);
        ev_timer_start(loop, &buzzer_timer);
#pragma GCC diagnostic pop

        sg_gyro_handle   = gyro_init(sg_gyro_dev, loop);
        sg_powerbutton_handle = powerbutton_init(const_cast<char*>(sg_powerbutton_dev), loop);

        ev_run(loop, 0);

        // 退出清理资源
        // printf("%s,%d: exit loop, clean resource!\n", __func__, __LINE__);
        // ev_timer_stop(loop, &warning_timer);
        // ev_loop_destroy(loop);
    }

    return 0;
}

#endif


void showAds1115Temp(int channel)
{
#ifndef __UNITTEST__
    cout << "showAds1115Temp(" << channel << ")" << endl;
    auto i2cBus1_temp_ptr = std::shared_ptr<hal::driver::I2CBus>(new hal::driver::I2CBus("/dev/i2c-1"));
    hal::driver::Ads1115 ads1115TempChip(i2cBus1_temp_ptr, 0x49);
    if( channel <= 2)
    {
        for(int i = 0; i < 5;)
        {
            auto lTemp = ads1115TempChip.getTemperature((hal::driver::Ads1115Channel)channel);
            cout << "Temperature[" << (std::uint16_t)channel << "][" << ++i << "]=" << lTemp << endl;
            sleep(1);
        }

    }
#endif
}

#ifndef __UNITTEST__
typedef void (*TestFunc_t)(int chnnael);
extern TestFunc_t sTestFuncPtr_Ads1115;
TestFunc_t sTestFuncPtr_Ads1115 = showAds1115Temp;
#endif

}
