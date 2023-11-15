#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//#include <sys/_stdint.h>
#include <string.h>

#include "eap_sys_cmds.h"
#include "eap_sys_drvcodes.h"
#include "eap_sys_funs.h"
#include "eap_product_features.h"
#include "log.h"

#define OS_LINUX
#ifndef NULL
#define NULL		(void*)0
#endif

#define UINT8_INVALID       0xFE
#define UINT16_INVALID      0xFEFE
#define UINT32_INVALID      0xFEFEFEFE
#define UINT8_ALL           0xFF
#define UINT16_ALL          0xFFFF
#define UINT32_ALL          0xFFFFFFFF

#define EAP_SUCCESS     0
#define EAP_FAIL        -1

#define EAP_CMD_REQUEST                         0
#define EAP_CMD_RESPONSE                        1

#define EAP_CMDTYPE_MODULE                      0
#define EAP_CMDTYPE_RUN_HEALTH                  1
#define EAP_CMDTYPE_TIMER_1                     2
#define EAP_CMDTYPE_TIMER_2                     3
#define EAP_CMDTYPE_TIMER_3                     4
#define EAP_CMDTYPE_TIMER_4                     5
#define EAP_CMDTYPE_TIMER_5                     6
#define EAP_CMDTYPE_TIMER_6                     7
#define EAP_CMDTYPE_TIMER_7                     8
#define EAP_CMDTYPE_TIMER_8                     9
#define EAP_CMDTYPE_STOPMODULE                  10

#define EAP_PROCESS_QUEUE_DRONE                 1

#define EAP_GET_LOC(processId, threadId)            (uint32_t)((processId << 16) | threadId)
#define EAP_GET_PROCESSID(loc)                      (int16_t)((loc >> 16) & 0xFFFF)
#define EAP_GET_THREADID(loc)                       (int16_t)(loc & 0xFFFF)

#define EAP_INTERVAL_BLANK                      "  "
#define EAP_BLANK_PREFIX(level)                 "\n" << setw((level) << 1) << EAP_INTERVAL_BLANK
#define EAP_OUT_THIS_PTR(level)                 cout << EAP_BLANK_PREFIX(level) << "Ptr: " << (uint64_t)this;
#define EAP_OUT_BACKSPACE                       "\b\b "

#define EAP_MIN(a, b)                           ((a) < (b) ? (a) : (b))

#define DECLARE_SINGLETON(classname)                                                                                                            \
public:                                                                                                                                         \
    static classname* Instance();                                                                                                               \
private:                                                                                                                                        \
    classname();                                                                                                                                \
    static classname* _instance;

#define DEFINE_SINGLETON(classname)                                                                                                             \
DEFINE_SINGLETON_NO_CONSTRUCT(classname)                                                                                                        \
classname::classname()                                                                                                                          \
{                                                                                                                                               \
}

#define DEFINE_SINGLETON_NO_CONSTRUCT(classname)                                                                                                             \
classname* classname::_instance = nullptr;                                                                                                      \
classname* classname::Instance()                                                                                                                \
{                                                                                                                                               \
    if (nullptr == _instance)                                                                                                                   \
    {                                                                                                                                           \
        _instance = new classname();                                                                                                            \
    }                                                                                                                                           \
    return _instance;                                                                                                                           \
}

void eap_printf(const char *ctrl1, ...);

#define GET_FILENAME(x) strrchr(x,'\\')?strrchr(x,'\\')+1:x
#define EAP_LOG_FATAL(fmt, args...)     lLOG_FATAL("[%s][%s][%d] " fmt, GET_FILENAME(__FILE__), __FUNCTION__,__LINE__ ,##args)
#define EAP_LOG_ERROR(fmt, args...)     lLOG_ERROR("[%s][%s][%d] " fmt, GET_FILENAME(__FILE__), __FUNCTION__,__LINE__ ,##args)
#define EAP_LOG_DEBUG(fmt, args...)     lLOG_DEBUG("[%s][%s][%d]" fmt, GET_FILENAME(__FILE__), __FUNCTION__, __LINE__,##args)
#define EAP_LOG_INFO(fmt, args...)      lLOG_INFO("[%s][%s][%d]" fmt, GET_FILENAME(__FILE__), __FUNCTION__, __LINE__,##args)
#define EAP_LOG_VERBOSE(fmt, args...)   lLOG_VERBOSE("[%s][%s][%d]" fmt, GET_FILENAME(__FILE__), __FUNCTION__, __LINE__,##args)

#define EAP_LOG_ERROR_EX(fmt, args...) \
    if (false == isLogByPass(LL_ERROR))\
    {\
        EAP_LOG_ERROR("[%s][%s][%d]" fmt, GET_FILENAME(__FILE__), __FUNCTION__, __LINE__,##args);\
    }


#define EAP_LOG_DEBUG_EX(fmt, args...) \
    if (false == isLogByPass(LL_DEBUG))\
    {\
        lLOG_DEBUG("[%s][%s][%d]" fmt, GET_FILENAME(__FILE__), __FUNCTION__, __LINE__,##args);\
    }

#define EAP_LOG_INFO_EX(fmt, args...)  \
    if (false == isLogByPass(LL_INFO))\
    {\
        lLOG_INFO("[%s][%s][%d]" fmt, GET_FILENAME(__FILE__), __FUNCTION__, __LINE__,##args);\
    }

#define EAP_LOG_VERBOSE_EX(fmt, args...)\
    if (false == isLogByPass(LL_VERB)) \
    {\
        lLOG_VERBOSE("[%s][%s][%d]" fmt, GET_FILENAME(__FILE__), __FUNCTION__, __LINE__,##args);\
    }

#define EAP_DELETE(obj)     \
{                           \
    if (nullptr != obj)     \
    {                       \
        delete obj;         \
        obj = nullptr;      \
    }                       \
}

#define EAP_DELETE_ARRAY(array)     \
{                                   \
    if (nullptr != array)           \
    {                               \
        delete[] array;             \
        array = nullptr;            \
    }                               \
}

#define CONVERT_16(value)       (IsBig_Endian() ? (value) : (uint16_t)((((uint16_t)(value) & 0x00FF) << 8) | \
                                                                       (((uint16_t)(value) & 0xFF00) >> 8) \
                                                                      ))

#define CONVERT_32(value)       (IsBig_Endian() ? (value) : (uint32_t)((((uint32_t)(value) & 0xFF000000) >> 24) | \
                                                                       (((uint32_t)(value) & 0x00FF0000) >> 8) | \
                                                                       (((uint32_t)(value) & 0x0000FF00) << 8) | \
                                                                       (((uint32_t)(value) & 0x000000FF) << 24) \
                                                                      ))

#define CONVERT_16_REV(value)       (!IsBig_Endian() ? (value) : (uint16_t)((((uint16_t)(value) & 0x00FF) << 8) | \
                                                                       (((uint16_t)(value) & 0xFF00) >> 8) \
                                                                      ))

#define CONVERT_32_REV(value)       (!IsBig_Endian() ? (value) : (uint32_t)((((uint32_t)(value) & 0xFF000000) >> 24) | \
                                                                       (((uint32_t)(value) & 0x00FF0000) >> 8) | \
                                                                       (((uint32_t)(value) & 0x0000FF00) << 8) | \
                                                                       (((uint32_t)(value) & 0x000000FF) << 24) \
                                                                      ))

#define EAP_ALINK_PROTOBUF          100

enum
{
    EAP_MODULE_ID_ALINK = 0,
    EAP_MODULE_ID_DRONEID,
    EAP_MODULE_ID_SPECTRUM,
    EAP_MODULE_ID_REMOTEID,
    EAP_MODULE_ID_FREQUENCY,
    EAP_MODULE_ID_HEARTBEAT,
    EAP_MODULE_ID_DRONE_SNIFFER,
    EAP_MODULE_ID_AIRBORNE_SNIFFER,
    EAP_MODULE_ID_BLUETOOTH,
    EAP_MODULE_ID_VEHICLE_MOUNTED,
#ifdef __UNITTEST__
    EAP_MODULE_ID_TEST1,
    EAP_MODULE_ID_TEST2,
#endif
    EAP_MODULE_ID_MAX
};


enum
{
    EAP_THREAD_ID_MAIN = 0,
    EAP_THREAD_ID_SYS = 1,
    EAP_THREAD_ID_CLI = 2,
    EAP_THREAD_ID_ALINK = 3,    
    EAP_THREAD_ID_REMOTEID = 4,
    EAP_THREAD_ID_RESP_MSG = 5,
    EAP_THREAD_ID_HEARTBEAT = 6,
    EAP_THREAD_ID_DRONEID = 7,
    EAP_THREAD_ID_FREQUENCY = 8,
    EAP_THREAD_ID_SPECTRUM = 9,
    EAP_THREAD_ID_DRONE_SNIFFER = 10,
    EAP_THREAD_ID_HEALTH_DETECT = 11,
    EAP_THREAD_ID_REMOTEIDWIFI = 12,
    EAP_THREAD_ID_TIMER = 13,
    EAP_THREAD_ID_AIRBORNE_SNIFFER = 14,
    EAP_THREAD_ID_BLUETOOTH = 15,
    EAP_THREAD_ID_VEHICLEMOUNTED = 16,
#ifdef __UNITTEST__
    EAP_THREAD_ID_TEST1 = 198,
    EAP_THREAD_ID_TEST2 = 199,
#endif
};

#define STACK_SIZE_MULTIPLE  8
enum 
{
    EAP_STACK_SIZE_SYS = 8192 * STACK_SIZE_MULTIPLE,
    EAP_STACK_SIZE_CLI = 8192* STACK_SIZE_MULTIPLE,
    EAP_STACK_SIZE_ALINK = 8192* STACK_SIZE_MULTIPLE,
    EAP_STACK_SIZE_REMOTEIDWIFI = 8192* STACK_SIZE_MULTIPLE,
    EAP_STACK_SIZE_REMOTEID = 8192* STACK_SIZE_MULTIPLE,
    EAP_STACK_SIZE_RESP_MSG = 8192* STACK_SIZE_MULTIPLE,
    EAP_STACK_SIZE_HEARTBEAT = 8192* STACK_SIZE_MULTIPLE,
    EAP_STACK_SIZE_DRONEID = 20480* STACK_SIZE_MULTIPLE,
    EAP_STACK_SIZE_FREQUENCY = 8192* STACK_SIZE_MULTIPLE,
    EAP_STACK_SIZE_SPECTRUM = 20480* STACK_SIZE_MULTIPLE,
    EAP_STACK_SIZE_DRONE_SNIFFER = 8192* STACK_SIZE_MULTIPLE,
    EAP_STACK_SIZE_AIRBORNE_SNIFFER = 8192* STACK_SIZE_MULTIPLE,
    EAP_STACK_SIZE_HEALTH_DETECT = 8192* STACK_SIZE_MULTIPLE,
    EAP_STACK_SIZE_TIMER = 1024 * STACK_SIZE_MULTIPLE,
    EAP_STACK_SIZE_BLUETOOTH = 1024 * STACK_SIZE_MULTIPLE,
    EAP_STACK_SIZE_VEHICLEMOUNTED = 1024 * STACK_SIZE_MULTIPLE,
#ifdef __UNITTEST__
    EAP_STACK_SIZE_TEST1 = 1024* STACK_SIZE_MULTIPLE,
    EAP_STACK_SIZE_TEST2 = 1024* STACK_SIZE_MULTIPLE,
#endif
} ;

enum
{
    EAP_THREAD_PRIOR_SYS = 3,
    EAP_THREAD_PRIOR_CLI = 7,
    EAP_THREAD_PRIOR_ALINK = 7,
    EAP_THREAD_PRIOR_REMOTEIDWIFI = 7,
    EAP_THREAD_PRIOR_REMOTEID = 7,
    EAP_THREAD_PRIOR_RESP_MSG = 7,
    EAP_THREAD_PRIOR_HEARTBEAT = 10,
    EAP_THREAD_PRIOR_DRONEID = 5,
    EAP_THREAD_PRIOR_FREQUENCY = 5,
    EAP_THREAD_PRIOR_SPECTRUM = 5,
    EAP_THREAD_PRIOR_DRONE_SNIFFER = 5,
    EAP_THREAD_PRIOR_AIRBORNE_SNIFFER = 5,
    EAP_THREAD_PRIOR_HEALTH_DETECT = 10,
    EAP_THREAD_PRIOR_TIMER = 11,
    EAP_THREAD_PRIOR_BLUETOOTH = 11,
    EAP_THREAD_PRIOR_VEHICLEMOUNTED = 11,
#ifdef __UNITTEST__
    EAP_THREAD_PRIOR_TEST1 = 7,
    EAP_THREAD_PRIOR_TEST2 = 7,
#endif
};

enum 
{
    DEV_NONE = 0, /*  | */
    DEV_AEAG = 0x01,
    DEV_SREEN = 0x02,
    DEV_RADAR = 0x03,
    DEV_PC = 0x04,
 	DEV_C2 = 0x06,
    DEV_TRACER = 0x07,
    DEV_UAV    = 0x08,
    DEV_TRACER_SEH100 = 0x17,    //低位7表示继承普通Tracer跟C2通信协议标准,高位1表示子设备类型为车载Tracer专有的C2下发打击功能
    MAV_COMPONENT_ENUM_END = 256, /*  | */
} ;

enum
{
    DEV_LED = 0x11,
    DEV_MOTOR = 0x12,
    DEV_BUZZER = 0x13,
};

enum
{
    LED_STATUS_NORMAL = 0,
    LED_STATUS_ABNORMAL = 1,
    // LED_STATUS_WARNING = 2,  // 告警灯由告警模块自动设置，请勿再配置
    LED_STATUS_LINK = 3,
};

enum
{
    CHANNEL_TYPE_NONE = 0,
    CHANNEL_TYPE_WIFI,
    CHANNEL_TYPE_TYPEC,
    CHANNEL_TYPE_BT
};

enum
{
    WORKMODE_DRONE = 1,
    WORKMODE_SPECTRUM_OMNI,
    WORKMODE_SPECTRUM_OMNI_ORIE,
    WORKMODE_AIRBORNE_OMNI,
    WORKMODE_AIRBORNE_OMNI_ORIE,
};

// 同步RTOS状态定义
typedef enum SYS_SystemStatusMode
{
	SYS_IDLE_STATUS = 0,
	SYS_DETECTION_TARGET_STATUS = 1,
	SYS_HIT_TARGET_STATUS = 2,
	SYS_SIMULATION_SIGAL_TO_TARGET_STATUS,
	SYS_MALFUNCTION_STATUS
} SYS_SystemStatusMode_t;

enum
{
	ORI_STATE_STANDBY = 0x00,
	ORI_STATE_DETECT = 0x01,
	ORI_STATE_HIT_REMOTE_IMG = 0x02,
	ORI_STATE_HIT_GNSS = 0x03,
	ORI_STATE_HIT_ALL = 0x04,
	ORI_STATE_SCANNING_HORI = 0x05,//正在水平扫描中
	ORI_STATE_HORI_AIMING = 0x06,//请水平瞄准
	ORI_STATE_SCANNING_PITCH = 0x07,//正在俯仰扫描中
	ORI_STATE_PITCH_AIMING = 0x08,//请俯仰瞄准
	ORI_STATE_AIM = 0x09,//已经瞄准
	ORI_STATE_ELIGIBLE_ENTRY_DIR = 0x0A,//允许进入定向
	ORI_STATE_NO_ELIGIBLE_ENTRY_DIR = 0x0B,//频率不在范围不允许进入定向
};

enum 
{
	ORI_STEP_IF_ENTER_DIR = 0,
	ORI_STEP_IF_AZIMUTH_ENOUGH,
	ORI_STEP_IF_AZIMUTH_AIMED,
	ORI_STEP_IF_PITCH_ENOUGH,
	ORI_STEP_IF_PITCH_AIMED,
	ORI_STEP_ALREADY_AIMED,
};

typedef enum
{
	eDATA_PATH_DDR_0_256M = 0x00,
	eDATA_PATH_DDR_256_512M = 0x01,
	eDATA_PATH_DDR_BURST = 0x02,
	eDATA_PATH_TIMER = 0x03,
} eDATA_PATH_INTR_FLAG;

enum 
{
	DETECT_MODE_NONE = 0x00,
	DETECT_MODE_IF_ENTER_DIR,
	DETECT_MODE_LEVELDETECTE,
	DETECT_MODE_PITCHANGLE,
};

enum 
{
	DEFINED_FREQ_MODE = 0,
	DEFINED_SWEEP_FREQ_MODE,
	AUTO_SWEEP_FREQ_MODE
} ;

typedef struct Cmdproc_Data
{
	uint32_t maxLen;
    uint32_t dataLen;
	uint8_t* data;
} Cmdproc_Data;

typedef struct Cmd_Stats
{
	uint32_t sendRequest;
    uint32_t recvRequest;
	uint32_t sendResponse;
    uint32_t recvResponse;
} Cmd_Stats;

#ifdef EAP_ALINK_MSG_MAX_BUFF
#define CMD_MSG_MAX_LEN         EAP_ALINK_MSG_MAX_BUFF + 1024
#else
#define CMD_MSG_MAX_LEN         (6*1024)
#endif

#define MSG_CHANNEL_TYPE_DEFAULT 1
#pragma pack(1)
typedef struct Thread_Msg_Head
{
	long  msg_channel_type;//for msgsnd()
    uint32_t from;
    uint32_t to;
    uint8_t reqType;
    uint8_t cmdType;
    uint16_t cmdCode;
    uint32_t seqNo;
    uint32_t length;
} Thread_Msg_Head;

typedef struct Thread_Msg
{
    Thread_Msg_Head head;
	uint8_t buf[CMD_MSG_MAX_LEN];
} Thread_Msg;
#pragma pack()

typedef enum
{
    WARNING_LEVEL_NONE = 0,
    WARNING_LEVEL_LOW,
    WARNING_LEVEL_MEDIUM,
    WARNING_LEVEL_HIGH,
} WarningLevel_t;
#define EAP_WARNING_LEVEL_MAX 4

#ifdef __cplusplus
}
#endif
