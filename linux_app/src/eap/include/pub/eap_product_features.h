#pragma once
#include "../../../inc/app_version.h"

#define PRODUCT_CFG_STD_PATH  "/etc"
#define PRODUCT_CFG_DEBUG_PATH  "/tmp/emmc_fat"
#define PRODUCT_HW_CFG_JSON_FILE  	"product_hw_cfg.json"
#define PRODUCT_HW_CFG_PB_FILE  	"product_hw_cfg.bin"

#define PRODUCT_SW_CFG_JSON_FILE  	"product_sw_cfg.json"
#define PRODUCT_SW_CFG_PB_FILE  	"product_sw_cfg.bin"

#define PRODUCT_DEBUG_CFG_JSON_FILE  	"product_debug_cfg.json"
#define PRODUCT_DEBUG_CFG_PB_FILE  		"product_debug_cfg.bin"


#define EAP_EMBED_COMPANY_NAME                      "SKYFEND"
#define EAP_EMBED_DEVICE_NAME                       "TRACER"
#define EAP_EMBED_DEFAULT_SN_CODE                   "TRACER101B112023497001"
#define EAP_EMBED_SOFTWARE_PS_VERSION_STR           APP_VERSION
#define EAP_EMBED_DEVICE_VERSION                    "v102"
#define EAP_MAX_RESULT_SIZE                             10240
#define EAP_PACKAGE_MAX_NUM                         30

#define EAP_ALINK_MSG_MAX_BUFF                      (4*1024 + 128)

#define EAP_DRONEID_MAX_NUM                         10
#define EAP_SPECTARGET_MAX_NUM                      10
#define EAP_REMOTEID_MA_NUM                         10

#define EAP_RPT_INTERVAL_NO_TARGET                  10

#define EAP_MAX_KEEP_REMOTEID_TIME                  15000


#define EAP_SYS_QUERY_DRV_TIMER                     3000
#define EAP_SYS_DUP_RESPONSE_TIMER                  1000
#define EAP_SYS_DUP_RESPONSE_TIMES                  3
#define EAP_DRONEID_FREQ_TIMER                      650
#define EAP_DRONEID_REP_TIMER                       100
#define EAP_C2_HEARTBEAT_CHECK_TIMER                1000
#define EAP_HEARTBEAT_TIMER                         1000
#define EAP_REMOTEID_REP_TIMER                      100
#define EAP_REMOTEID_REP_TIMER_WIFI                 700
#define EAP_SPECTRUM_REP_TIMER                      100
#define EAP_HEALTH_DETECT_TIMER                     10000
#define EAP_STD_ERR_LOG_REFRESH_TIMER               5000

// DroneID数据加了帧头帧位CRC校验
#define EAP_DRONEID_VALID_ADC_LEN                   19960
#define EAP_DRONEID_VALID_ADC_LEN_WITH_HEAD         (EAP_DRONEID_VALID_ADC_LEN + 20)
#define EAP_DRONEID_BUFFER_LEN                      EAP_DRONEID_VALID_ADC_LEN_WITH_HEAD

#define EAP_SPECTRUM_N                              1
#define EAP_SPECTRUM_LINE_NUM                       3000
#define EAP_SPECTRUM_COLUMN_NUM                     128
#define EAP_SPECTRUM_TARGET_REMAIN_TIMES            90  // 三轮切频都没检测到目标认为消失(= Freq_MAX_CNT * 3)
#define EAP_SPECTRUM_TARGET_MAX_NUM                 30

#define EAP_DRONE_SNIFFER_NUM                       EAP_SPECTRUM_TARGET_MAX_NUM
#define EAP_C2_HEARTBEAT_TIMEOUT                    5000

#define EAP_GEAR_POSITION_0                         0
#define EAP_GEAR_POSITION_1                         1
#define EAP_GEAR_POSITION_2                         2
#define EAP_GEAR_POSITION_3                         3
#define EAP_GEAR_POSITION_NUM                       4

#define EAP_TARGET_STATE_NOT                        0
#define EAP_TARGET_STATE_YES                        1
#define EAP_TARGET_STATE_NUM                        2

enum
{
    DRONE = 0,
    OMNI,
    OMNI_ORIE,
    OMNI_ORIE_DISCOVERY,
};
#define EAP_WARN_WORK_MODE_NUM                      4

