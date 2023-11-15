#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <sstream>

#include "SysBase.h"
#include "ThreadMgr.h"
#include "Module.h"
#include "HalMgr.h"
#include "CLI_Template.h"
#include "CmdprocBase.h"
#include "ModuleStrategy.h"
#include "AlinkCmdproc.h"
#include "TracerDataPath.h"
#include "hal/c2_alink/check/checksum.h"

#include "json_pb_converter.h"
#include "../alink_pb/TracerUavMsg.pb.h"
#include "../../../app/alg/det_alg/sfAirBorneSniffer.h"
#include "WhitelistManager.h"

#ifdef __cplusplus
extern "C" {
#endif

// TODO: 临时为ADS1115 温度传感器添加调试命令
/***************Debug for ADS1115 begin*************************/
void (*sTestFuncPtr_Ads1115)(int channel) = NULL;

#ifdef __cplusplus
}
#endif

extern int g_scan_channels_all[3];

void Ads1115GetTemp(int channel)
{
#ifndef __UNITTEST__
    if(sTestFuncPtr_Ads1115)
    {
        sTestFuncPtr_Ads1115(channel);
    }
    EAP_LOG_DEBUG("Ads1115GetTemp() success!\n");
#endif
}
/***************Debug for ADS1115 end*************************/

void cli_log_lvl_func(int32_t len, const char *param, char *pcWriteBuffer, size_t xWriteBufferLen)
{
    if (len > 0)
    {
        uint32_t lvl = strtoul(param, NULL, 0);
        log_set_lvl((log_level_t)lvl);
        snprintf(pcWriteBuffer, xWriteBufferLen, "\r\nOK\r\n");
    }
    else
    {
        uint32_t lvl = (uint32_t)log_get_lvl_num();
        const char *lvl_str = log_get_lvl_str();
        snprintf(pcWriteBuffer, xWriteBufferLen, "\r\n%lu: %s\r\n", lvl, lvl_str);
    }
}

void cli_version_func(char *pcWriteBuffer, size_t xWriteBufferLen)
{
	lLOG_DEBUG(" Firmware compile time:%s %s\n", __DATE__, __TIME__);
	lLOG_DEBUG("version:%s\r\n", EAP_EMBED_SOFTWARE_PS_VERSION_STR);
	//LOG_DEBUG("PL_Version: %X %X\r\n", EapGetSys()._plVersionTime(), EapGetSys()._plVersionTime());
	snprintf(pcWriteBuffer, xWriteBufferLen, "OK");
}

DEFINE_CLI_HANDLER1_FUNC(log_lvl, "\r\n log_lvl (level):\r\n Set or get the log level\r\n", cli_log_lvl_func)
DEFINE_CLI_HANDLER_FUNC(Version, "\r\n Version\r\n  \r\n", cli_version_func)

SysBase::SysBase(uint8_t deviceType, int16_t processId, char *productCfgStdPath, char *productCfgDebugPath)
    : _processId(processId), _deviceType(deviceType)
{
    _threadMgr = new ThreadMgr;
    _halMgr = nullptr;
    _snLen = EAP_MIN(sizeof(EAP_EMBED_DEFAULT_SN_CODE), EAP_SN_LENGTH);
    memcpy(_snName, EAP_EMBED_DEFAULT_SN_CODE, _snLen);
    memset(_remoteAddr, 0, sizeof(_remoteAddr));
    memset(_modules, 0, sizeof(_modules));

    _sysStatus = 0;
    _curDetectFreq = 0;
    _plVersionDate = 0;
    _plVersionTime = 0;
    _batteryCapacity = 100;
    _batAndAdapterState = 0;
    _dupProtocol = 0;
    _remainDupTimes = 0;
#ifndef __UNITTEST__	
	ProductCfgLoad(productCfgStdPath, productCfgDebugPath);
#endif
    // 设置工作模式
    if (ProductSwCfgGet().issupportdroneid())
        _workMode = WORKMODE_DRONE;
    else if (ProductSwCfgGet().issupportspectrum())
        _workMode = WORKMODE_SPECTRUM_OMNI;
    else if (ProductSwCfgGet().issupportairborne())
        _workMode = WORKMODE_AIRBORNE_OMNI;
    // 更新设备类型(普通Tracer设备类型为0x7,车载Tracer设备类型为0x17)
    _deviceType = ProductSwCfgGet().devicetype();
    EAP_LOG_DEBUG("_deviceType = 0x%x", _deviceType);

    EAP_LOG_DEBUG("SupportWhitelist = %d", ProductSwCfgGet().issupportwarningwhitelist());
    if (ProductSwCfgGet().issupportwarningwhitelist())
    {
        _whitelistManager = new WhitelistManager();
    }
}

SysBase::~SysBase()
{
    EAP_DELETE(_threadMgr);

    for (int i = 0; i < EAP_MODULE_ID_MAX; ++i)
    {
        EAP_DELETE(_modules[i]);
    }

    EAP_DELETE(_whitelistManager);
}

ThreadMgr& SysBase::GetThreadMgr()
{
    assert(nullptr != _threadMgr);
    return *_threadMgr;
}

void SysBase::_InstallModule(Module* module)
{
    uint8_t moduleId = module->GetId();
    if (moduleId >= EAP_MODULE_ID_MAX)
    {
        EAP_LOG_ERROR("moduleId(%d) is error!", moduleId);
        return;
    }
    if (_modules[moduleId] != nullptr)
    {
        EAP_DELETE(_modules[moduleId]);
    }
    _modules[moduleId] = module;
}

void SysBase::_InstallHalMgr(HalMgr* halMgr)
{
    if (nullptr != _halMgr) EAP_DELETE(_halMgr);
    _halMgr = halMgr;
}

void SysBase::Init()
{
    CmdprocBase::Init();
    for (int i = 0; i < EAP_MODULE_ID_MAX; ++i)
    {
        if (nullptr == _modules[i]) continue;
        _modules[i]->PreInit();
    }
   
    _RegisterAlinkCmd();
    _RegisterAlgCb();
    if (nullptr != _halMgr) 
    {
        _halMgr->Init();
        _GetSnFromHal();
        _GetPLVersionFromHal();
        _GetRemoteAddrFromHal();
    }
    GetThreadMgr().CreateThreads();
    _RegisterCliCmd();

    if (nullptr != _whitelistManager)
    {
        _whitelistManager->Init();
    }

    EAP_LOG_INFO("SysBase::Init code is %d",1);
}

void SysBase::_RegisterAlinkCmd()
{
    register_alink_cmd(EAP_ALINK_CMD_GET_SOFTWARE_VERSION, Alink_GetSoftwareVersion_Func);
    register_alink_cmd(EAP_ALINK_CMD_SET_ORIENTATION_MODE, Alink_SetOrientationWorkMode_Func);
    register_alink_cmd(EAP_ALINK_CMD_DUP_QUERY_BROADCAST, Alink_DeviceStatusInfo_Func);
    register_alink_cmd(EAP_ALINK_CMD_GET_DEV_INFO, Alink_GetDevInfo_Func);
    register_alink_cmd(EAP_ALINK_CMD_SET_CIRCUIT_BOARD_SN_CODE, Alink_SetBoardSN_Func);
    register_alink_cmd(EAP_ALINK_CMD_GET_CIRCUIT_BOARD_SN_CODE, Alink_GetBoardSN_Func);
	register_alink_cmd(EAP_ALINK_CMD_GET_WORK_MODE, Alink_GetWorkMode_Func);
    register_alink_cmd(EAP_ALINK_CMD_C2_GET_LOG_LIST, Alink_Get_Log_List_Func);
    register_alink_cmd(EAP_ALINK_CMD_C2_GET_LOG, Alink_Get_Log_Data_Func);
    register_alink_cmd(EAP_ALINK_CMD_C2_DELETE_LOG, Alink_Delete_Log_Func);
    register_alink_cmd(EAP_ALINK_CMD_C2_SEND_HEARTBEAT, Alink_C2HeartBeatPacket_Func);
    register_alink_cmd(EAP_ALINK_CMD_C2_CLI_CMD, Alink_C2CliCmd_Func);
    register_alink_cmd(EAP_ALINK_CMD_C2_SET_WHITELIST, Alink_SetWhiteList_Func);
    register_alink_cmd(EAP_ALINK_CMD_TCP_WIFI_INFO, Alink_C2BlueToothCmd_Func);/*bluetooth*/
    register_alink_cmd(EAP_ALINK_RPT_TIME_INFO, Alink_GetC2TimeInfo_Func);
    register_alink_cmd(EAP_ALINK_CMD_GET_SELF_INSPECT_INFO, Alink_GetSelfInspectInfo_Func);
    /*traces shrd106l*/
    register_alink_cmd(EAP_ALINK_CMD_C2_SET_FIRED_MODE, Alink_C2SetFiredMode_Func);
    register_alink_cmd(EAP_ALINK_CMD_C2_GET_FIRED_MODE, Alink_C2GetFiredMode_Func);
    register_alink_cmd(EAP_ALINK_CMD_C2_START_FIRED_ALL, Alink_C2StartFiredALL_Func);
    register_alink_cmd(EAP_ALINK_CMD_C2_STOP_FIRED_ALL, Alink_C2StopFiredALL_Func);
    register_alink_cmd(EAP_ALINK_CMD_C2_SET_FIRED_TIME, Alink_C2SetFiredTime_Func);
    register_alink_cmd(EAP_ALINK_CMD_C2_GET_FIRED_TIME, Alink_C2GetFiredTime_Func);

    register_alink_cmd(EAP_ALINK_CMD_SET_ALARM_MODE, Alink_SetAlarmMode_Func);
    register_alink_cmd(EAP_ALINK_CMD_SET_COVERT_MODE, Alink_SetCovertMode_Func);
}

void SysBase::_GetPLVersionFromHal()
{
    Hal_Data inData = {0, nullptr};
    Drv_GetPLVersion version;
    Hal_Data outData = {sizeof(Drv_GetPLVersion), (uint8_t*)&version};
    if (EAP_SUCCESS == _halMgr->OnGet(EAP_DRVCODE_GET_PL_VERSION, inData, outData))
    {
        _plVersionDate = version.PL_Version_date & 0xFFFF;
        _plVersionTime = version.PL_Version_time;
    }
}

void SysBase::_GetRemoteAddrFromHal()
{
    Hal_Data inData = {0, nullptr};
    char remoteAddr[EAP_REMOTE_ADDR_LEN] = {0};
    Hal_Data outData = {EAP_REMOTE_ADDR_LEN, (uint8_t*)remoteAddr};
    if (EAP_SUCCESS == _halMgr->OnGet(EAP_DRVCODE_GET_REMOTE_ADDR, inData, outData))
    {
        memcpy(_remoteAddr, remoteAddr, EAP_REMOTE_ADDR_LEN);
    }
}

void SysBase::_GetSnFromHal()
{
    Hal_Data inData = {0, nullptr};
    Hal_Data outData = {sizeof(_snName), (uint8_t*)_snName};
    if (EAP_SUCCESS == _halMgr->OnGet(EAP_DRVCODE_GET_SN, inData, outData))
    {
        _snLen = outData.length;
    }
}

void SysBase::SetSnName(char* snName, uint8_t snLen)
{
    if (snLen != 0 && snLen <= EAP_SN_LENGTH)
    {
        memcpy(_snName, snName, snLen);
        _snLen = snLen;
    }
}

void SysBase::SetRemoteAddr(char* remoteAddr)
{
    memcpy(_remoteAddr, remoteAddr, EAP_REMOTE_ADDR_LEN);
}

void SysBase::_RegisterCliCmd()
{
    FreeRTOS_CLIRegisterCommand(&log_lvl_cmd);
    FreeRTOS_CLIRegisterCommand(&Version_cmd);
}

uint32_t SysBase::GetModulesNum()
{
    uint32_t num = 0;
    for (int i = 0; i < EAP_MODULE_ID_MAX; ++i)
    {
        if (_modules[i] != nullptr) ++num;
    }
    return num;
}

Module* SysBase::GetModule(uint8_t moduleId)
{
    if (moduleId >= EAP_MODULE_ID_MAX) return nullptr;
    return _modules[moduleId];
}

ModuleStrategy* SysBase::GetModuleStrategy(uint8_t moduleId)
{
    Module* module = GetModule(moduleId);
    return (nullptr == module) ? nullptr : &(module->Strategy());
}

void SysBase::SetWorkMode(uint8_t workMode)
{
    _workMode = workMode;
}

void SysBase::SetSysStatus(uint8_t status)
{
    _sysStatus = status;
    // 同步系统状态到心跳模块的工作状态（注意：打击时需要停止定向侦测流程）
    SendAsynData(EAP_CMD_REFRESH_WORK_STATUS, (uint8_t*)&_sysStatus, 1);
}

void SysBase::RefreshBatteryCapacity()
{
    Hal_Data inData = {0, nullptr};
    Drv_GetBatteryInfo batteryInfo = {0};
    Hal_Data outData = {sizeof(batteryInfo), (uint8_t*)&batteryInfo};
    if (EAP_SUCCESS == _halMgr->OnGet(EAP_DRVCODE_GET_BATTERY_CAPACITY, inData, outData))
    {
        _batteryCapacity = batteryInfo.batteryCapacity;
        _batAndAdapterState = batteryInfo.batAndAdapterState;
    }
}

void SysBase::ShowModule(uint8_t moduleId)
{
    Module *module = GetModule(moduleId);
    if (nullptr != module)
    {
        cout << "\n" << "module: " << hex;
        module->RequestShowInfo();
    }
    else
    {
        cout << "\nParam is invalid!";
    }
}

void SysBase::SetTargetAppear(uint8_t isTargetAppear)
{
    TargetWarning warning = {EAP_TARGET_SCENE_DRONEID, (uint8_t)(isTargetAppear ? 1 : 0)};
    SendAsynData(EAP_CMD_TARGET_WARNING, (uint8_t*)&warning, sizeof(TargetWarning));
}


// eap_do(30,2,4,1)
void SysBase::SendALinkMsg(uint8_t channelType, uint8_t msgType, uint32_t p1, uint32_t p2, uint32_t p3)
{
    //std::cout << "channelType "  << (std::uint16_t)channelType 
    //          <<  ", msgType "  << (std::uint16_t)msgType 
    //          <<  ", p1 "  << (std::uint16_t)p1
    //          <<  ", p2 "  << (std::uint16_t)p2
    //          <<  ", p3 "  << (std::uint16_t)p3 
    //          << std::endl;

    if ((EAP_CHANNEL_TYPE_UDP == channelType)
        || (EAP_CHANNEL_TYPE_TCP == channelType)
        || (EAP_CHANNEL_TYPE_TYPEC == channelType))
    {
        std::uint8_t buff[20];
        buff[0] = channelType;
        buff[1] = msgType;
        buff[2] = p1 >> 24;
        buff[3] = (p1 >> 16) & 0xFF;
        buff[4] = (p1 >> 8) & 0xFF;
        buff[5] = (p1) & 0xFF;
        buff[6] = p2 >> 24;
        buff[7] = (p2 >> 16) & 0xFF;
        buff[8] = (p2 >> 8) & 0xFF;
        buff[9] = (p2) & 0xFF;
        buff[10] = p3 >> 24;
        buff[11] = (p3 >> 16) & 0xFF;
        buff[12] = (p3 >> 8) & 0xFF;
        buff[13] = (p3) & 0xFF;

        Hal_Data inData = {14, buff};

        if (10 > msgType)  // simulate to receive flystatus Alink message from UAV
        {
            _halMgr->OnSet(EAP_DRVCODE_SEND_ALINK_MSG, inData);
        }
        else    // simulate to send measure data Alink message to UAV
        {
            std::uint8_t buffer[512] = {0};
            const std::uint32_t maxLength = 512;
            bool result = false;

            protobuf::traceruav::MeasureInfoPb meaInfoData;
            protobuf::traceruav::TracerUavMessagePB uavMsgData;

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

            uavMsgData.mutable_measureinfo()->CopyFrom(meaInfoData);

            uavMsgData.set_msgchannel("");
            uavMsgData.set_msgtype(protobuf::traceruav::TRACERUAVMSG_MEASURE_INFO_TYPE);
            uavMsgData.set_msgname("");
            uavMsgData.set_msghandle(0);
            uavMsgData.set_msgseq(1);

            std::size_t dataLength = uavMsgData.ByteSizeLong();

            if (dataLength <= maxLength)
            {
                result = uavMsgData.SerializeToArray(buffer, dataLength);
            }

            std::ostringstream out;
            for (uint32_t pos = 0; pos < dataLength; pos++)
            {
                out << " " << std::hex << std::setw(2) << std::setfill('0') << (uint16_t)(buffer[pos]);
            }
            std::cout << "SysBase::SendALinkMsg dataLength = " << std::dec << dataLength << ", " << out.str() << std::endl;

            alink_msg_t msg = {0};
            msg.channelType = EAP_CHANNEL_TYPE_TCP | EAP_CHANNEL_TYPE_TYPEC;
            msg.channelId   = UINT16_ALL;
            memcpy(msg.buffer, buffer, dataLength);

            AlinkCmdproc::SendAlinkCmd(msg, dataLength, EAP_ALINK_RPT_TRACER_UAV_MESSAGE);
        }
    }
}

//eap_do(36,2,43,0)
//eap_do(36,2,43,1)
//eap_do(36,7,229,82,4,16,129,192,2) //0xa001 52 82 4 16 129 192 2
//eap_do(36,7,229,82,4,16,130,192,2) //0xa002 82 4 16 130 192 2
void SysBase::SetReceivedALinkMsg(const int* buff, const std::uint32_t length)
{
    if ((buff == nullptr) || (length < 1) || (length > 128))
    {
        return;
    }

    // workaround, when parameter number is bigger than 6, the debug tool cannot handler
    if (length > 8)
    {
        static std::uint8_t para6 = 130;
        if (129 == para6)
        {
            para6 = 130;
        }
        else
        {
            para6 = 129;
        }

        int* buffAmend = (int*)buff;
        buffAmend[6] = para6;
        buffAmend[7] = 192;
        buffAmend[8] = 2;
    }

    Thread_Msg msg = {0};
    alink_msg_t* alink_msg = (alink_msg_t* )msg.buf;
    
    alink_msg->channelType = EAP_CHANNEL_TYPE_UDP;
    alink_msg->channelId = 0;

    std::uint8_t buffer[256] = {0};
    std::uint32_t payloadLength = length - 1;

    alink_msg_head_t msgHeader;
    msgHeader.magic    = 0xFD;
    msgHeader.len_lo   = (payloadLength & 0x00FF);
    msgHeader.len_hi   = (payloadLength & 0xFF00) >> 8;
    msgHeader.seq      = 0x0;
    msgHeader.destid   = DEV_UAV;  //dest id
    msgHeader.sourceid = DEV_C2;   //source id
    msgHeader.msgid    = buff[0];
    msgHeader.ans      = 1;
    msgHeader.checksum = 0;

    std::uint8_t *pkt = (uint8_t *)&msgHeader;
    for (std::uint8_t i = 0; i < ALINK_CORE_HEADER_LEN; i++)
    {
        msgHeader.checksum += pkt[i];
    }

    memcpy(buffer, &msgHeader, 9);
    for(std::uint32_t i = 0; i < payloadLength; i++)
    {
        buffer[9 + i] = buff[1 + i];
    }  // memcpy(buffer + 9, buff + 1, payloadLength); // type is not same


    std::uint16_t crc = crc_calculate(buffer + 1, payloadLength + sizeof(alink_msg_head_t) - 1);
    buffer[payloadLength + 9] = crc & 0xFF;
    buffer[payloadLength + 10] = (crc & 0xFF00) >> 8;

    memcpy((void*)&alink_msg->msg_head, buffer, sizeof(alink_msg_head_t) + payloadLength + 2); // add 2 bytes CRC

    std::ostringstream out;
    for (std::uint32_t pos = 0; pos < sizeof(alink_msg_head_t) + payloadLength + 2; pos++)
    {
        out << " " << std::hex << std::setw(2) << std::setfill('0') << (std::uint16_t)(((uint8_t *)&alink_msg->msg_head)[pos]);
    }
    std::cout << "SysBase::SetReceivedALinkMsg : " << out.str() << std::endl;
    // fd 08 00 00 08 06 e5 01 f9 18 01 52 04 10 81 c0 02 71 58 enable
    // fd 08 00 00 08 06 e5 01 f9 18 01 52 04 10 82 c0 02 15 b7 disable
    
    std::uint16_t payload_len = alink_msg->msg_head.len_hi << 8 | alink_msg->msg_head.len_lo;
    std::size_t msg_len = sizeof(alink_msg_head_t) + payload_len + sizeof(alink_data_crc_t);
    (void)msg_len;
   
    // add 2 bytes CRC with 4 bytes channelType and channelId;
    SendAsynMsg(EAP_CMD_ALINK, (uint8_t *)&msg, sizeof(alink_msg_head_t) + payloadLength + 6);
}

void SysBase::SetLedStatus(uint8_t ledNo, uint8_t status)
{
    std::uint8_t buff[2];
    buff[0] = ledNo;
    buff[1] = status;

    Hal_Data inData = {2, buff};
    _halMgr->OnSet(EAP_DRVCODE_SET_LED_STATUS, inData);
}

void SysBase::AirBorneCtrl(uint16_t workMode, uint16_t dxNum, uint16_t dxFreq)
{
    ioController_t state = {workMode, dxNum, dxFreq};
    SendAsynData(EAP_CMD_AIRBORNE_STATE_CTRL, (uint8_t*)&state, sizeof(ioController_t));
}

void SysBase::AdcSetGain(uint32_t value)
{
    HalMgr* halMgr = EapGetSys().GetHalMgr();
    Hal_Data inData = {sizeof(uint32_t), (uint8_t*)&value};
    if (nullptr != halMgr) halMgr->OnSet(EAP_DRVCODE_SET_GAIN, inData);
    EAP_LOG_DEBUG("设置增益为0x%x\n", value);
}

void SysBase::ShowModuleUnSafe(uint8_t moduleId)
{
    Module *module = GetModule(moduleId);
    if (nullptr != module)
    {
        cout << "\n" << "module: " << hex;
        module->ShowInfo();
    }
}

void SysBase::SetModuleFilterLevel(const uint8_t moduleId, const log_level_t level)
{
    Module *module = GetModule(moduleId);
    if (nullptr != module)
    {
        module->Strategy().setLogFilterLevel(level);
    }
}

void SysBase::SetOtherModulesFilterLevel(const uint8_t moduleId, const log_level_t level)
{
    for (auto& module : _modules)
    {
        if ((nullptr != module) && (module->Strategy().GetModuleId() != moduleId))
        {
            module->Strategy().setLogFilterLevel(level);
        }
    }
}

void SysBase::ShowInfo()
{
    cout << hex;
    cout << "\n" << "Class: " << typeid(*this).name();
    cout << "\n" << "_processId: " << _processId;
    cout << "\n" << "_snName: " << _snName;
    cout << "\n" << "_snLen: " << (uint16_t)_snLen;
    cout << "\n" << "_remoteAddr: " << _remoteAddr;
    cout << "\n" << "_deviceType: " << (uint16_t)_deviceType;
    cout << "\n" << "_workMode: " << (uint16_t)_workMode;
    cout << "\n" << "_sysStatus: " << (uint16_t)_sysStatus;
    cout << "\n" << "_curDetectFreq: " << _curDetectFreq;
    cout << "\n" << "_plVersionDate: " << _plVersionDate;
    cout << "\n" << "_plVersionTime: " << _plVersionTime;
    cout << "\n" << "_batteryCapacity: " << (uint16_t)_batteryCapacity;
    cout << "\n" << "_batAndAdapterState: " << (uint16_t)_batAndAdapterState;
    cout << "\n" << "_dupProtocol: " << _dupProtocol;
    cout << "\n" << "_remainDupTimes: " << (uint16_t)_remainDupTimes;
    cout << "\n" << "_modules: ";    
    for (int i = 0; i < EAP_MODULE_ID_MAX; ++i)
    {
        cout << EAP_BLANK_PREFIX(1) << "Module[" << i << "]: ";
        if (nullptr == _modules[i]) continue;
        _modules[i]->Strategy().ShowKeyInfo(2);
    }
}

void SysBase::SetDupInfo(uint32_t protocol)
{
    //if (_remainDupTimes > 0) return;
    _dupProtocol = protocol;
    _remainDupTimes = EAP_SYS_DUP_RESPONSE_TIMES;
    //DupResponse();
}

int16_t SysBase::_FillDupBroadcastResponseInfo(uint8_t* buf)
{
    AlinkDupBroadcastResp *response = (AlinkDupBroadcastResp *)buf;
    response->protocol = _dupProtocol;
    memcpy(response->sn, _snName, _snLen);
    response->type = _deviceType;
    response->status = 2; // 已有连接
    memcpy(response->version, EAP_EMBED_SOFTWARE_PS_VERSION_STR, strlen(EAP_EMBED_SOFTWARE_PS_VERSION_STR));
    return sizeof(AlinkDupBroadcastResp);
}

void SysBase::DupResponse()
{
    //if (_remainDupTimes > 0)
    {
        --_remainDupTimes;
        alink_msg_t msg = {0};
        msg.channelType = EAP_CHANNEL_TYPE_UDP;
        msg.channelId   = UINT16_ALL;
        int16_t dataLen = _FillDupBroadcastResponseInfo(msg.buffer);
        AlinkCmdproc::SendAlinkCmd(msg, dataLen, EAP_ALINK_RPT_DEVICE_STATUS_INFO);
    }
}

extern SysBase& EapGetSys(void);


extern "C"{

	void send_BB_msg(void)
	{
#if 0
		SysBase *p = &EapGetSys();
	
		if(!p){
			EAP_LOG_DEBUG("g_sys is NULL\n");
			return ;
		}
		EAP_LOG_DEBUG("g_sys is ready\n");
#endif 
		EapGetSys().DupResponse();
	}


	void set_eth_proto(uint32_t protocol)
	{
		EapGetSys().SetDupInfo(protocol);	
	}
}



void SysBase::Telnet_GetSN()
{
    uint8_t sn[EAP_SN_LENGTH] = {0};
    Hal_Data inData = {0, nullptr};
    Hal_Data outData = {sizeof(sn), sn};
    if (EAP_SUCCESS == _halMgr->OnGet(EAP_DRVCODE_GET_SN, inData, outData))
    {
        cout << "Telnet_GetSN: " << outData.buf << endl;
    }
}

void SysBase::Telnet_SetSN(char *sn, int length)
{
    Hal_Data inData = {length, (uint8_t*)sn};
    if (EAP_SUCCESS == _halMgr->OnSet(EAP_DRVCODE_SET_SN, inData))
    {
        cout << "Telnet_SetSN: " << inData.buf << endl;
    }
}

void SysBase::Telnet_GetRemoteAddr()
{
    Hal_Data inData = {0, nullptr};
    char remoteAddr[EAP_REMOTE_ADDR_LEN] = {0};
    Hal_Data outData = {EAP_REMOTE_ADDR_LEN, (uint8_t*)remoteAddr};
    if (EAP_SUCCESS == _halMgr->OnGet(EAP_DRVCODE_GET_REMOTE_ADDR, inData, outData))
    {
//        cout << __func__ << " ok addr: " << remoteAddr << endl;
    }
    cout << __func__ << " addr: " << remoteAddr << endl;
}

extern "C"
{
    
    void eap_main()
    {
        EapGetSys().Init();
    }

    void eap_help()
    {
        int i = 0;
        cout << hex;
        cout << "\n  " << "No.  | Func ";
        cout << "\n  " << setw(4) << i++ << " | " << "ShowAllThreadsMsgStats()";
        cout << "\n  " << setw(4) << i++ << " | " << "ShowOneThreadMsgStats(uint16_t threadId)";
        cout << "\n  " << setw(4) << i++ << " | " << "OpenThreadRunDetect()";
        cout << "\n  " << setw(4) << i++ << " | " << "ThreadPause(uint16_t threadId)";
        cout << "\n  " << setw(4) << i++ << " | " << "ThreadIdling(uint16_t threadId)";
        cout << "\n  " << setw(4) << i++ << " | " << "ThreadResume(uint16_t threadId)";
        cout << "\n  " << setw(4) << i++ << " | " << "ShowAllThreadsInfo()";
        cout << "\n  " << setw(4) << i++ << " | " << "EnableStoreCmd(uint8_t type)";
        cout << "\n  " << setw(4) << i++ << " | " << "DisableStoreCmd(uint8_t type)";
        cout << "\n  " << setw(4) << i++ << " | " << "ShowOneModuleInfo(uint8_t moduleId)";
        cout << "\n  " << setw(4) << i++ << " | " << "ShowAllCmdStat()";
        cout << "\n  " << setw(4) << i++ << " | " << "ShowOneCmdStat(uint16_t cmdCode)";
        cout << "\n  " << setw(4) << i++ << " | " << "ShowCmdprocRegisterInfo()";
        cout << "\n  " << setw(4) << i++ << " | " << "ShowSysInfo()";
        cout << "\n  " << setw(4) << i++ << " | " << "ShowProductFeatures()";
        cout << "\n  " << setw(4) << i++ << " | " << "ShowHalMgrInfo()";
        cout << "\n  " << setw(4) << i++ << " | " << "ShowOneHalInfo(uint8_t index)";
        cout << "\n  " << setw(4) << i++ << " | " << "ShowHalRegisterInfo()";
        cout << "\n  " << setw(4) << i++ << " | " << "ClearAllCmdStat()";
        cout << "\n  " << setw(4) << i++ << " | " << "ClearOneCmdStat(uint16_t cmdCode)";
        cout << "\n  " << setw(4) << i++ << " | " << "ShowAlinkCmdRegisterInfo()";
        cout << "\n  " << setw(4) << i++ << " | " << "ShowCliRegisteredCmds()";
        cout << "\n  " << setw(4) << i++ << " | " << "AddIgnoreCmd(uint16_t cmdCode)";
        cout << "\n  " << setw(4) << i++ << " | " << "DelIgnoreCmd(uint16_t cmdCode)";
        cout << "\n  " << setw(4) << i++ << " | " << "ReCreateThread(uint16_t threadId)";
        cout << "\n  " << setw(4) << i++ << " | " << "ClearRemainDupTimes()";
        cout << "\n  " << setw(4) << i++ << " | " << "SetWarningLevel(uint8_t gearPosition,uint8_t targetState,uint8_t workMode,uint8_t level)";
        cout << "\n  " << setw(4) << i++ << " | " << "ShowVersionInfo()";
		cout << "\n  " << setw(4) << i++ << " | " << "ShowOneThreadInfo(uint16_t threadId)";
        cout << "\n  " << setw(4) << i++ << " | " << "SetTargetAppear(uint8_t isTargetAppear)";
        cout << "\n  " << setw(4) << i++ << " | " << "SendALinkMsg(uint8_t channelType, uint8_t msgType)";
        cout << "\n  " << setw(4) << i++ << " | " << "SetOutputSpecMat(uint8_t value)";
        cout << "\n  " << setw(4) << i++ << " | " << "AirBorneCtrl(uint16_t workMode, uint16_t dxNum, uint16_t dxFreq)";
        cout << "\n  " << setw(4) << i++ << " | " << "SetLedStatus(uint8_t ledNo, uint8_t status)";
        cout << "\n  " << setw(4) << i++ << " | " << "AdcSetGain(uint32_t value)";
        cout << "\n  " << setw(4) << i++ << " | " << "Ads1115GetTemp(int channel)";
        cout << "\n  " << setw(4) << i++ << " | " << "SetReceivedALinkMsg(int p0, ...)";
        cout << "\n  " << setw(4) << i++ << " | " << "SetRemoteIdWifiChannel(int p0, ...)";
    }

    void eap_do(uint16_t index, int count, ...)
    {
        int params[10] = {0};
        va_list args;
        va_start(args, count);

        for (int i = 0; i < count && i < 10; i++) 
        {
            params[i] = va_arg(args, int);
        }
        va_end(args);
        switch (index)
        {
        case 0: ShowAllThreadsMsgStats(); break;
        case 1: ShowOneThreadMsgStats(params[0]); break;
        case 2: OpenThreadRunDetect(); break;
        case 3: ThreadPause(params[0]); break;
        case 4: ThreadIdling(params[0]); break;
        case 5: ThreadResume(params[0]); break;
        case 6: ShowAllThreadsInfo(); break;
        case 7: EnableStoreCmd(params[0]); break;
        case 8: DisableStoreCmd(params[0]); break;
        case 9: ShowOneModuleInfo(params[0]); break;
        case 10: ShowAllCmdStat(); break;
        case 11: ShowOneCmdStat(params[0]); break;
        case 12: ShowCmdprocRegisterInfo(); break;
        case 13: ShowSysInfo(); break;
        case 14: ShowProductFeatures(); break;
        case 15: ShowHalMgrInfo(); break;
        case 16: ShowOneHalInfo(params[0]); break;
        case 17: ShowHalRegisterInfo(); break;
        case 18: ClearAllCmdStat(); break;
        case 19: ClearOneCmdStat(params[0]); break;
        case 20: ShowAlinkCmdRegisterInfo(); break;
        case 21: ShowCliRegisteredCmds(); break;
        case 22: AddIgnoreCmd(params[0]); break;
        case 23: DelIgnoreCmd(params[0]); break;
        case 24: ReCreateThread(params[0]); break;
        case 25: ClearRemainDupTimes(); break;
        case 26: SetWarningLevel(params[0], params[1], params[2], params[3]); break;
        case 27: ShowVersionInfo(); break;
        case 28: ShowOneThreadInfo(params[0]); break;
        case 29: SetTargetAppear(params[0]); break;
        case 30: SendALinkMsg(params[0], params[1], params[2], params[3], params[4]); break;
        case 31: SetOutputSpecMat(params[0]); break;
        case 32: AirBorneCtrl(params[0], params[1], params[2]); break;
        case 33: AdcSetGain(params[0]); break;
        case 34: SetLedStatus(params[0], params[1]); break;
        case 35: Ads1115GetTemp(params[0]); break;
        case 36: SetReceivedALinkMsg(params, count); break;
        case 37: SetRemoteIdWifiChannel(params, count); break;
        default: cout << "\n Input Error!"; break;
        }
    }

    void ShowOneModuleInfo(uint8_t moduleId)
    {
        EapGetSys().ShowModule(moduleId);
    }

    void ShowOneModuleInfoUnSafe(uint8_t moduleId)
    {
        EapGetSys().ShowModuleUnSafe(moduleId);
    }

    void ClearRemainDupTimes()
    {
        EapGetSys().ClearRemainDupTimes();
    }

    void ShowSysInfo()
    {
        EapGetSys().ShowInfo();
    }

    void SetTargetAppear(uint8_t isTargetAppear)
    {
        EapGetSys().SetTargetAppear(isTargetAppear);
    }

    void SendALinkMsg(uint8_t channelType, uint8_t msgType, uint32_t p1, uint32_t p2, uint32_t p3)
    {
        EapGetSys().SendALinkMsg(channelType, msgType, p1, p2, p3);
    }

    void SetLedStatus(uint8_t ledNo, uint8_t status)
    {
        EapGetSys().SetLedStatus(ledNo, status);
    }

    //eap_do(36,2,43,0)
    //eap_do(36,2,43,1)
    void SetReceivedALinkMsg(const int * para, const std::uint32_t length)
    {
        EapGetSys().SetReceivedALinkMsg(para, length);
    }

    //eap_do(37,3,1,1,1) eap_do(37,3,6,6,6) eap_do(37,3,11,11,11)
    void SetRemoteIdWifiChannel(const int * para, const std::uint32_t length)
    {
        if ((para == nullptr) || (length < 1) || (length > 3))
        {
            return;
        }

        for (std::uint32_t i = 0; i < length; i++)
        {
            g_scan_channels_all[i] = para[i];
            std::cout << "[" << i << "] = " << g_scan_channels_all[i] << std::endl;
        }
    }

    void AirBorneCtrl(uint16_t workMode, uint16_t dxNum, uint16_t dxFreq)
    {
        EapGetSys().AirBorneCtrl(workMode, dxNum, dxFreq);
    }

    void AdcSetGain(uint32_t value)
    {
        EapGetSys().AdcSetGain(value);
    }

    void ShowProductFeatures()
    {
        cout << hex;
        cout << "\n" << "EAP_EMBED_COMPANY_NAME: " << EAP_EMBED_COMPANY_NAME;
        cout << "\n" << "EAP_EMBED_DEVICE_NAME: " << EAP_EMBED_DEVICE_NAME;
        cout << "\n" << "EAP_EMBED_DEFAULT_SN_CODE: " << EAP_EMBED_DEFAULT_SN_CODE;
        cout << "\n" << "EAP_EMBED_SOFTWARE_PS_VERSION_STR: " << EAP_EMBED_SOFTWARE_PS_VERSION_STR;
        cout << "\n" << "EAP_EMBED_DEVICE_VERSION: " << EAP_EMBED_DEVICE_VERSION;
        cout << "\n" << "EAP_ALINK_MSG_MAX_BUFF: " << EAP_ALINK_MSG_MAX_BUFF;
        cout << "\n" << "EAP_DRONEID_MAX_NUM: " << EAP_DRONEID_MAX_NUM;
        cout << "\n" << "EAP_SPECTARGET_MAX_NUM: " << EAP_SPECTARGET_MAX_NUM;
        cout << "\n" << "EAP_REMOTEID_MA_NUM: " << EAP_REMOTEID_MA_NUM;
        cout << "\n" << "EAP_MAX_KEEP_REMOTEID_TIME: " << EAP_MAX_KEEP_REMOTEID_TIME;
        cout << "\n" << "EAP_SYS_QUERY_DRV_TIMER: " << EAP_SYS_QUERY_DRV_TIMER;
        cout << "\n" << "EAP_SYS_DUP_RESPONSE_TIMER: " << EAP_SYS_DUP_RESPONSE_TIMER;
        cout << "\n" << "EAP_SYS_DUP_RESPONSE_TIMES: " << EAP_SYS_DUP_RESPONSE_TIMES;
        cout << "\n" << "EAP_DRONEID_FREQ_TIMER: " << EAP_DRONEID_FREQ_TIMER;
        cout << "\n" << "EAP_DRONEID_REP_TIMER: " << EAP_DRONEID_REP_TIMER;
        cout << "\n" << "EAP_HEARTBEAT_TIMER: " << EAP_HEARTBEAT_TIMER;
        cout << "\n" << "EAP_REMOTEID_REP_TIMER: " << EAP_REMOTEID_REP_TIMER;
        cout << "\n" << "EAP_SPECTRUM_REP_TIMER: " << EAP_SPECTRUM_REP_TIMER;
        cout << "\n" << "EAP_HEALTH_DETECT_TIMER: " << EAP_HEALTH_DETECT_TIMER;
        cout << "\n" << "EAP_DRONEID_VALID_ADC_LEN: " << EAP_DRONEID_VALID_ADC_LEN;
        cout << "\n" << "EAP_SPECTRUM_N: " << EAP_SPECTRUM_N;
        cout << "\n" << "EAP_SPECTRUM_LINE_NUM: " << EAP_SPECTRUM_LINE_NUM;
        cout << "\n" << "EAP_SPECTRUM_COLUMN_NUM: " << EAP_SPECTRUM_COLUMN_NUM;
        cout << "\n" << "EAP_SPECTRUM_TARGET_REMAIN_TIMES: " << EAP_SPECTRUM_TARGET_REMAIN_TIMES;
        cout << "\n" << "EAP_SPECTRUM_TARGET_MAX_NUM: " << EAP_SPECTRUM_TARGET_MAX_NUM;
        cout << "\n" << "EAP_DRONE_SNIFFER_NUM: " << EAP_DRONE_SNIFFER_NUM;
        cout << "\n" << "EAP_RPT_INTERVAL_NO_TARGET: " << EAP_RPT_INTERVAL_NO_TARGET;
        cout << "\n" << "EAP_C2_HEARTBEAT_TIMEOUT: " << EAP_C2_HEARTBEAT_TIMEOUT;
    }
    void ShowVersionInfo()
    {
        cout << hex;
        cout << "\n" << "APP_VERSION: " << APP_VERSION;
        cout << "\n" << "GIT_VERSION: " << GIT_VERSION;
        //cout << "\n" << "COMPILE_DATE: " << COMPILE_DATE;
    }

    void TN_GetSN()
    {
        EapGetSys().Telnet_GetSN();
    }

    void TN_SetSN()
    {
        char *sn = "SN0123456789";
        EapGetSys().Telnet_SetSN(sn, strlen(sn));
    }

    void TN_GetRemoteAddr()
    {
        EapGetSys().Telnet_GetRemoteAddr();
    }

}
