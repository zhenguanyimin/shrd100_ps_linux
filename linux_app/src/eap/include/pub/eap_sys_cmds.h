#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#include "eap_product_features.h"

#define EAP_SN_LENGTH           25
#define EAP_REMOTE_ADDR_LEN     16
#define EAP_LOG_FILE_NAME_LEN   56
#define EAP_LOG_FILE_LEN_MAX    1024*16

#define EAP_STR(x) #x

#define EAP_CASE_CMD(cmd) \
    case cmd: \
        return EAP_STR(cmd)

#define EAP_CHANNEL_TYPE_UDP                1
#define EAP_CHANNEL_TYPE_TCP                2
#define EAP_CHANNEL_TYPE_TYPEC              4
#define EAP_CHANNEL_TYPE_BLE_BT             8

#define EAP_GET_CHANNEL(type, id)           ((uint32_t)(((type) << 16) | id))
#define EAP_GET_CHANNEL_TYPE(channel)       ((uint16_t)(channel >> 16))
#define EAP_GET_CHANNEL_ID(channel)         ((uint16_t)(channel & 0xFFFF))

enum
{
    EAP_CMD_BEGIN = 0,
    EAP_CMD_SET_WORK_MODE,
    EAP_CMD_ALINK,
    EAP_CMD_REMOTEID,
    EAP_CMD_REMOTEID_RID_CAPTURE,
    EAP_CMD_DRONEID,
    EAP_CMD_DRONEID_SET_ALG_PRINT,
    EAP_CMD_DRONEID_SET_REMOVE_THRE,
    EAP_CMD_DRONEID_SET_REMOVE_THRE_LOST_DUR,
    EAP_CMD_DRONEID_SET_OUTPUT_SN_MODE,
    EAP_CMD_DRONEID_SET_TURBO_DEC_ITER_NUM,
    EAP_CMD_FREQUENCY,
    EAP_CMD_SPECTRUM,
    EAP_CMD_DRONE_SNIFFER,
    EAP_CMD_AIRBORNE_SNIFFER,
    EAP_CMD_AIRBORNE_STATE_CTRL,
    EAP_CMD_SET_FREQUENCY,  // 切到指定频率
    EAP_CMD_SET_FREQUENCY_MODE,
    EAP_CMD_CHANGE_FREQUENCY_PARAM,
    EAP_CMD_REFRESH_WORK_STATUS,
    EAP_CMD_REFRESH_FAULT_STATUS,
    EAP_CMD_SET_SN_CODE,
    EAP_CMD_GET_SN_CODE,
    EAP_CMD_CLEAR_ORIENTATE,
    EAP_CMD_TARGET_WARNING,
    EAP_CMD_SET_MSG_STORE_ON,
    EAP_CMD_SET_DRV_STORE_ON,
    EAP_CMD_RUN_HEALTH_DETECT,
    EAP_CMD_RUN_PAUSE,
    EAP_CMD_RUN_IDLING,
    EAP_CMD_RUN_RESUME,
    EAP_CMD_SHOW_FREQUENCY_MODULE,
    EAP_CMD_SHOW_HEARTBEAT_MODULE,
    EAP_CMD_SHOW_DRONEID_MODULE,
    EAP_CMD_SHOW_REMOTEID_MODULE,
    EAP_CMD_SHOW_SPECTRUM_MODULE,
    EAP_CMD_UPDATE_C2_HEARTBEAT_TIME,
    EAP_CMD_UPDATE_ALINK_TIME,
    EAP_CMD_SET_WIFI_NAME_AND_SECRET,
    EAP_CMD_SET_COVERT_MODE,
    EAP_CMD_SET_VEHICLE_MOUNTED_START_FIRED_ALL,
    EAP_CMD_SET_VEHICLE_MOUNTED_STOP_FIRED_ALL,
#ifndef __UNITTEST__
    EAP_CMD_MAX
#else
    EAP_CMD_MAX = 200
#endif
} ;

#define EAP_GET_CMD_STR(cmd) \
    { \
        switch (cmd) { \
        EAP_CASE_CMD(EAP_CMD_BEGIN); \
        EAP_CASE_CMD(EAP_CMD_SET_WORK_MODE); \
        EAP_CASE_CMD(EAP_CMD_ALINK); \
        EAP_CASE_CMD(EAP_CMD_REMOTEID); \
        EAP_CASE_CMD(EAP_CMD_REMOTEID_RID_CAPTURE); \
        EAP_CASE_CMD(EAP_CMD_DRONEID); \
        EAP_CASE_CMD(EAP_CMD_DRONEID_SET_ALG_PRINT); \
        EAP_CASE_CMD(EAP_CMD_DRONEID_SET_REMOVE_THRE); \
        EAP_CASE_CMD(EAP_CMD_DRONEID_SET_REMOVE_THRE_LOST_DUR); \
        EAP_CASE_CMD(EAP_CMD_DRONEID_SET_OUTPUT_SN_MODE); \
        EAP_CASE_CMD(EAP_CMD_DRONEID_SET_TURBO_DEC_ITER_NUM); \
        EAP_CASE_CMD(EAP_CMD_FREQUENCY); \
        EAP_CASE_CMD(EAP_CMD_SPECTRUM); \
        EAP_CASE_CMD(EAP_CMD_DRONE_SNIFFER); \
        EAP_CASE_CMD(EAP_CMD_AIRBORNE_SNIFFER); \
        EAP_CASE_CMD(EAP_CMD_AIRBORNE_STATE_CTRL); \
        EAP_CASE_CMD(EAP_CMD_SET_FREQUENCY); \
        EAP_CASE_CMD(EAP_CMD_SET_FREQUENCY_MODE); \
        EAP_CASE_CMD(EAP_CMD_CHANGE_FREQUENCY_PARAM); \
        EAP_CASE_CMD(EAP_CMD_REFRESH_WORK_STATUS); \
        EAP_CASE_CMD(EAP_CMD_REFRESH_FAULT_STATUS); \
        EAP_CASE_CMD(EAP_CMD_SET_SN_CODE); \
        EAP_CASE_CMD(EAP_CMD_GET_SN_CODE); \
        EAP_CASE_CMD(EAP_CMD_CLEAR_ORIENTATE); \
        EAP_CASE_CMD(EAP_CMD_TARGET_WARNING); \
        EAP_CASE_CMD(EAP_CMD_SET_MSG_STORE_ON); \
        EAP_CASE_CMD(EAP_CMD_SET_DRV_STORE_ON); \
        EAP_CASE_CMD(EAP_CMD_RUN_HEALTH_DETECT); \
        EAP_CASE_CMD(EAP_CMD_SHOW_FREQUENCY_MODULE); \
        EAP_CASE_CMD(EAP_CMD_SHOW_HEARTBEAT_MODULE); \
        EAP_CASE_CMD(EAP_CMD_SHOW_DRONEID_MODULE); \
        EAP_CASE_CMD(EAP_CMD_SHOW_REMOTEID_MODULE); \
        EAP_CASE_CMD(EAP_CMD_SHOW_SPECTRUM_MODULE); \
        EAP_CASE_CMD(EAP_CMD_UPDATE_C2_HEARTBEAT_TIME); \
        EAP_CASE_CMD(EAP_CMD_UPDATE_ALINK_TIME); \
        EAP_CASE_CMD(EAP_CMD_SET_WIFI_NAME_AND_SECRET); \
        EAP_CASE_CMD(EAP_CMD_SET_COVERT_MODE); \
        EAP_CASE_CMD(EAP_CMD_SET_VEHICLE_MOUNTED_START_FIRED_ALL); \
        EAP_CASE_CMD(EAP_CMD_SET_VEHICLE_MOUNTED_STOP_FIRED_ALL); \
        default: \
            return "unknown"; \
        } \
    }

//C2与Tracer交互信息ID
enum
{
    EAP_ALINK_CMD_XX = 0,
    EAP_ALINK_CMD_GET_DEV_INFO = 0x20,
    EAP_ALINK_CMD_SET_ORIENTATION_MODE = 0x21,
    EAP_ALINK_CMD_GET_WORK_MODE = 0x22,
    EAP_ALINK_CMD_C2_SEND_HEARTBEAT = 0x25,
    EAP_ALINK_CMD_C2_GET_LOG_LIST = 0x28,
    EAP_ALINK_CMD_C2_GET_LOG = 0x29,
    EAP_ALINK_CMD_C2_DELETE_LOG = 0x2a,
    EAP_ALINK_CMD_SET_RF_ENABLE = 0x2b,  // Raspberry pie control RF enable
    EAP_ALINK_CMD_C2_CLI_CMD = 0x2f,

    /* for tracer-s shrd106L start*/
    EAP_ALINK_CMD_C2_SET_FIRED_MODE = 0x30,
    EAP_ALINK_CMD_C2_GET_FIRED_MODE = 0x31,
    EAP_ALINK_CMD_C2_START_FIRED_ALL = 0x32,
    EAP_ALINK_CMD_C2_STOP_FIRED_ALL = 0x33,
    EAP_ALINK_CMD_C2_SET_FIRED_TIME = 0x34,
    EAP_ALINK_CMD_C2_GET_FIRED_TIME = 0x35,
    /* for tracer-s shrd106L end */
//上位机测试信息ID
    EAP_ALINK_CMD_GET_SELF_INSPECT_INFO = 0x73,
//生产配置信息ID
    EAP_ALINK_CMD_SET_CIRCUIT_BOARD_SN_CODE = 0x80,
    EAP_ALINK_CMD_GET_CIRCUIT_BOARD_SN_CODE = 0x81,
    EAP_ALINK_CMD_GET_SOFTWARE_VERSION = 0x82,
//上位机通过USB升级与Tracer交互消息ID
    EAP_ALINK_CMD_PC_REQUEST_SYS_RESET = 0xa1,//upgrade
    EAP_ALINK_CMD_PC_GET_LOG_LIST = 0xa2,
    EAP_ALINK_CMD_PC_GET_LOG = 0xa3,
    EAP_ALINK_CMD_PC_DELETE_LOG = 0xa4,
    EAP_ALINK_CMD_PC_GET_FIRMWARE_WRITE_STATUS = 0xa6,//upgrade
    EAP_ALINK_CMD_PC_REQUEST_FIRMWARE_UPGRADE = 0xa7,//upgrade
    EAP_ALINK_CMD_PC_REQUEST_DOWNLOAD_FIRMWARE_DATA = 0xa8,//upgrade
    EAP_ALINK_CMD_PC_REQUEST_WRITE_FIRMWARE_DATA = 0xa9,//upgrade
    EAP_ALINK_CMD_PC_GET_VERSION_INFO = 0xab,//upgrade
    EAP_ALINK_CMD_PC_REQUEST_VERITY_FIRMWARE_DATA = 0xac,//upgrade
    EAP_ALINK_CMD_PC_REQUEST_RUN_FIRMWARE = 0xad,//upgrade
    EAP_ALINK_CMD_PC_GET_TIMEOUT_TIME = 0xae,//upgrade
    EAP_ALINK_CMD_PC_REQURST_BOOT_SKIP_APP = 0xaf,

//设备组网协议
    EAP_ALINK_CMD_DUP_QUERY_BROADCAST = 0xBA,
	EAP_ALINK_CMD_TCP_LINK_INFO = 0xBC,
	EAP_ALINK_CMD_TCP_WIFI_INFO = 0xBD,

//C2控制Tracer接口
    EAP_ALINK_CMD_C2_SET_WHITELIST = 0xC5,
    EAP_ALINK_CMD_SET_ALARM_MODE = 0xC6,
    EAP_ALINK_CMD_SET_COVERT_MODE = 0xC7,
    EAP_ALINK_CMD_SET_WORK_MODE = 0xC9,
    EAP_ALINK_CMD_ENTER_ORIENTATE = 0xCA,
    EAP_ALINK_CMD_TRACER_UAV_MESSAGE = 0xE5,
    EAP_ALINK_CMD_MAX
};


#define EAP_GET_ALINKCMD_STR(cmd) \
    { \
        switch (cmd) { \
        EAP_CASE_CMD(EAP_ALINK_CMD_GET_DEV_INFO); \
        EAP_CASE_CMD(EAP_ALINK_CMD_SET_ORIENTATION_MODE); \
        EAP_CASE_CMD(EAP_ALINK_CMD_GET_WORK_MODE); \
        EAP_CASE_CMD(EAP_ALINK_CMD_C2_SEND_HEARTBEAT); \
        EAP_CASE_CMD(EAP_ALINK_CMD_C2_GET_LOG_LIST); \
        EAP_CASE_CMD(EAP_ALINK_CMD_C2_GET_LOG); \
        EAP_CASE_CMD(EAP_ALINK_CMD_C2_DELETE_LOG); \
        EAP_CASE_CMD(EAP_ALINK_CMD_SET_RF_ENABLE); \
        EAP_CASE_CMD(EAP_ALINK_CMD_C2_SET_FIRED_MODE); \
        EAP_CASE_CMD(EAP_ALINK_CMD_C2_GET_FIRED_MODE); \
        EAP_CASE_CMD(EAP_ALINK_CMD_C2_START_FIRED_ALL); \
        EAP_CASE_CMD(EAP_ALINK_CMD_C2_STOP_FIRED_ALL); \
        EAP_CASE_CMD(EAP_ALINK_CMD_C2_SET_FIRED_TIME); \
        EAP_CASE_CMD(EAP_ALINK_CMD_GET_SELF_INSPECT_INFO); \
        EAP_CASE_CMD(EAP_ALINK_CMD_SET_CIRCUIT_BOARD_SN_CODE); \
        EAP_CASE_CMD(EAP_ALINK_CMD_GET_CIRCUIT_BOARD_SN_CODE); \
        EAP_CASE_CMD(EAP_ALINK_CMD_GET_SOFTWARE_VERSION); \
        EAP_CASE_CMD(EAP_ALINK_CMD_PC_REQUEST_SYS_RESET); \
        EAP_CASE_CMD(EAP_ALINK_CMD_PC_GET_LOG_LIST); \
        EAP_CASE_CMD(EAP_ALINK_CMD_PC_GET_LOG); \
        EAP_CASE_CMD(EAP_ALINK_CMD_PC_DELETE_LOG); \
        EAP_CASE_CMD(EAP_ALINK_CMD_PC_GET_FIRMWARE_WRITE_STATUS); \
        EAP_CASE_CMD(EAP_ALINK_CMD_PC_REQUEST_FIRMWARE_UPGRADE); \
        EAP_CASE_CMD(EAP_ALINK_CMD_PC_REQUEST_DOWNLOAD_FIRMWARE_DATA); \
        EAP_CASE_CMD(EAP_ALINK_CMD_PC_REQUEST_WRITE_FIRMWARE_DATA); \
        EAP_CASE_CMD(EAP_ALINK_CMD_PC_GET_VERSION_INFO); \
        EAP_CASE_CMD(EAP_ALINK_CMD_PC_REQUEST_VERITY_FIRMWARE_DATA); \
        EAP_CASE_CMD(EAP_ALINK_CMD_PC_REQUEST_RUN_FIRMWARE); \
        EAP_CASE_CMD(EAP_ALINK_CMD_PC_GET_TIMEOUT_TIME); \
        EAP_CASE_CMD(EAP_ALINK_CMD_PC_REQURST_BOOT_SKIP_APP); \
        EAP_CASE_CMD(EAP_ALINK_CMD_DUP_QUERY_BROADCAST); \
        EAP_CASE_CMD(EAP_ALINK_CMD_TCP_LINK_INFO); \
        EAP_CASE_CMD(EAP_ALINK_CMD_TCP_WIFI_INFO); \
        EAP_CASE_CMD(EAP_ALINK_CMD_SET_WORK_MODE); \
        EAP_CASE_CMD(EAP_ALINK_CMD_ENTER_ORIENTATE); \
        default: \
            return "unknown"; \
        } \
    }

//Tracer与C2交互信息ID
enum
{
	EAP_ALINK_RPT_DEVICE_STATUS_INFO = 0xBB,
    EAP_ALINK_RPT_DRONEID = 0xE0,
    EAP_ALINK_RPT_REMOTEID = 0xE1,
    EAP_ALINK_RPT_SPECTRUM = 0xE2,
    EAP_ALINK_RPT_TIME_INFO = 0xE3,
    EAP_ALINK_RPT_TRACER_FIRE_STATE = 0xE4,            // trace-s SHRD106L 
    EAP_ALINK_RPT_TRACER_UAV_MESSAGE = 0xE5,       // EAP_ALINK_CMD_TRACER_UAV_MESSAGE = 0xE5
    EAP_ALINK_RPT_HEARTBEAT = 0xEF,
};


enum
{
    EAP_TARGET_SCENE_DRONEID = 1,
    EAP_TARGET_SCENE_REMOTEID,
    EAP_TARGET_SCENE_SPECTRUM
};

#pragma pack(1)

typedef struct Rpt_RemoteInfo
{
	uint8_t productType;
	char droneName[25];
	char serialNum[32];
	int32_t droneLongitude;
	int32_t droneLatitude;
	int16_t droneHeight;
	int16_t droneDirection;
	int16_t droneSpeed;
	uint8_t speedderection;
	int16_t droneVerticalSpeed;
	int32_t droneSailLongitude;
	int32_t droneSailLatitude;
	uint32_t uFreq;
	uint16_t uDistance;
	uint16_t dangerLevels;
} Rpt_RemoteInfo;

typedef struct Rpt_DroneInfo
{
	uint8_t productType;
	char droneName[25];
	char serialNum[32];
	int32_t droneLongitude;
	int32_t droneLatitude;
	int16_t droneHeight;
	int16_t droneYawAngle;
	int16_t droneSpeed;
	int16_t droneVerticalSpeed;
	int32_t pilotLongitude;
	int32_t pilotLatitude;
	uint32_t uFreq;
	uint16_t uDistance;
	uint16_t dangerLevels;
} Rpt_DroneInfo;

typedef struct Rpt_HeartBeat
{
    uint32_t timeStamp;
    uint8_t electricity;
    uint8_t batteryStatus;
    uint8_t workMode;
    uint8_t workStatus;
    uint8_t alarmLevel;
    char snName[EAP_SN_LENGTH];
} Rpt_HeartBeat;

typedef struct Rpt_SpectrumDroneInfo
{
    uint8_t uavNumber;
	char droneName[25];
	int32_t droneHorizon;
	uint32_t uFreq;
	uint16_t dangerLevels;
    int32_t recerve;
} Rpt_SpectrumDroneInfo;

typedef struct Spectrum_DataBuffer
{
	uint32_t flag; // 1 for in using ,0 for idle
	uint32_t type;
	uint32_t ts;
	uint32_t length; // length of the pData
	uint8_t *pData;
} Spectrum_DataBuffer;

typedef struct Spectrum_DataPingPong
{
	Spectrum_DataBuffer buff1;
	Spectrum_DataBuffer buff2;
	volatile uint32_t revLen;
	uint32_t usedId;
} Spectrum_DataPingPong;

typedef struct SpectrumAlgResult
{
    int ID;             //
    char name[50];      //
    int psbID[10];		//可能的机型序号
    int nPsbID;			//机型序号可能性数量
    char flag;          //
    float freq[128];     //
    float burstTime[128];//
    float width[128];    //
    float bw;           //
    float amp;          //
    float range;        //
} SpectrumAlgResult;

typedef struct TargetWarning
{
    uint8_t scene;  // 1-droneid  2-remoteid  3-spectrum
    uint8_t targetState; // 0-disappear  1-appear   2-orientation
} TargetWarning;

typedef struct AlinkDupBroadcastReq
{
    uint32_t protocol;
    uint8_t sn[32];
} AlinkDupBroadcastReq;

typedef struct AlinkDupBroadcastResp
{
    uint32_t protocol;
    uint8_t sn[32];
    uint16_t type;
    uint16_t status;
    char version[64];
} AlinkDupBroadcastResp;

typedef struct AlinkDevInfoResp
{
    char Company_name[16];
    char Device_name[16];
    char PS_version[32];
    char PL_version[32];
    char Device_sn[16];
    char Device_ip[16];
} AlinkDevInfoResp;

typedef struct alink_data_crc
{
	uint16_t crc;
}alink_data_crc_t;

typedef struct alink_msg_head
{
    uint8_t magic;    ///< protocol magic marker
    uint8_t len_lo;   ///< Low byte of length of payload
    uint8_t len_hi;   ///< Hight byte of length of payload
    uint8_t seq;      ///< Sequence of packet
    uint8_t destid;   ///< ID of message destination
    uint8_t sourceid; ///< ID of the message sounrce
    uint8_t msgid;    ///< ID of message in payload
    uint8_t ans;      ///< Whether the command requires an answer 0:no  1:yes
    uint8_t checksum; ///< Checksum of message header
    uint8_t payload[0];  ///< point to the start of payload data
} alink_msg_head_t;


typedef struct self_inspect_info_resp
{
    uint16_t version;
    uint16_t tempFpga;
    uint16_t tempBatBoard1;
    uint16_t tempBatBoard2;
    uint16_t batteryCap;
    uint8_t  workMode;
    uint8_t  digitComStatus;
    uint8_t  fanRateStatus;
    uint8_t  fanRate;
    uint8_t  tcDevStatus;
    uint8_t  tcConnectStatus;
    uint8_t  tcFrequence;
    uint8_t  wifiRemoteIdDevStatus;
    uint8_t  wifiRemoteIdConnectStatus;
    uint8_t  wifiNetDevStatus;
    uint8_t  wifiNetConnectStatus;
	uint8_t  reserve;
}SelfInspectInfoResp;

#define ALINK_MSG_MAGIC_VAL (0XFD)

#define ALINK_MAX_PAYLOAD_LEN (EAP_ALINK_MSG_MAX_BUFF - sizeof(alink_msg_head_t) -sizeof(alink_data_crc_t)) 
#define ALINK_MAX_PAYLOAD_LEN_WITH_CRC (EAP_ALINK_MSG_MAX_BUFF - sizeof(alink_msg_head_t)) 

#define ALINK_MSG_T_HEADER_SIZE 	(offsetof(alink_msg_t, buffer))

#define ALINK_MSG_BD_PRO_PAYLOAD_SIZE  68

typedef struct alink_msg
{
    uint16_t channelType;
    uint16_t channelId;
    alink_msg_head_t msg_head;     //  message header
    uint8_t buffer[ALINK_MAX_PAYLOAD_LEN_WITH_CRC]; 
} alink_msg_t;

struct AlinkLogListResp
{
    uint32_t Log_name_len;
    uint32_t Log_data_len;
    char Log_name[EAP_LOG_FILE_NAME_LEN];
};

struct AlinkLogReq
{
    uint32_t Log_id;
    uint32_t Log_name_len;
    uint8_t Log_name[EAP_LOG_FILE_NAME_LEN];
};

struct AlnikLogResp
{
    uint32_t Log_id;
    uint32_t Pkg_total_num;
    uint32_t Pkg_cur_num;
    uint32_t Data_len;
};


typedef struct alink_cli_cmd
{
	uint32_t handle;
	char	 cmd[0];
} alink_cli_cmd_t;
typedef struct alink_cli_cmd_ack
{
	uint32_t handle;
	int32_t  cmd_parse_retcode;
	int32_t  cmd_exec_retcode;
	char	 cmd_exec_output[0];
} alink_cli_cmd_ack_t;
	

#pragma pack()

#ifdef __cplusplus
}
#endif
