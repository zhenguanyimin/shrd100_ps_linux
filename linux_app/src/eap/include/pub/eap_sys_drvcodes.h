#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    EAP_DRVCODE_BEGIN = 0,
    EAP_DRVCODE_GET_SN,
    EAP_DRVCODE_SET_SN,
    EAP_DRVCODE_CLI_OUT,
    EAP_DRVCODE_RPT_INFO_BY_ALINK,
    EAP_DRVCODE_GET_DRONE_FREQ,
    EAP_DRVCODE_GET_DRONE_ADC,
    EAP_DRVCODE_DRONE_PL_TEST,
    EAP_DRVCODE_ALINK_RESPONSE,
    EAP_DRVCODE_GET_DETECTION_PARAM,
    EAP_DRVCODE_SET_DETECT_FREQ,
    EAP_DRVCODE_SET_GAIN,
    EAP_DRVCODE_SET_GAIN_AGC,
    EAP_DRVCODE_GET_ANGLE_INFO,
    EAP_DRVCODE_GET_ANGLE_INFO_EX,
    EAP_DRVCODE_SET_ANTENNA_MODE,
    EAP_DRVCODE_GET_PL_VERSION,
    EAP_DRVCODE_GET_GPIO_VALUE,
    EAP_DRVCODE_SET_GPIO_VALUE,
    EAP_DRVCODE_GET_REGISTER_VALUE,
    EAP_DRVCODE_SET_REGISTER_VALUE,
    EAP_DRVCODE_GET_BATTERY_CAPACITY,
    EAP_DRVCODE_GET_SILENT_MODE,
    EAP_DRVCODE_GET_ALARM_MODE,
    EAP_DRVCODE_SET_ALARM_MODE,
    EAP_DRVCODE_SET_COVERT_MODE,
    EAP_DRVCODE_SET_LED_STATE,
    EAP_DRVCODE_SET_MOTOR_STATE,
    EAP_DRVCODE_SET_BUZZER_STATE,
    EAP_DRVCODE_SEND_ALINK,
    EAP_DRVCODE_SET_UAV_STATUS,
    EAP_DRVCODE_SEND_ALINK_MSG,  // to simulate an ALink message
    EAP_DRVCODE_SET_LED_STATUS,  // set LED status
    EAP_DRVCODE_DEBUG_SET_LED_STATUS,    // to debug LED status
    EAP_DRVCODE_GET_WIFI_STATUS,         // get wifi status
    EAP_DRVCODE_SET_WIFI_STATUS,         // set wifi status
    // EAP_DRVCODE_RPT_UAV_MEASURE_INFO, // use EAP_DRVCODE_RPT_INFO_BY_ALINK is enough
    
    //REMOTEID SECTION BEGIN
    EAP_DRVCODE_REMOTEID_BUF_USE_FINISH,
    EAP_DRVCODE_REMOTEID_SET_CHANNEL,
    EAP_DRVCODE_REMOTEID_START_SNIFFER,
    EAP_DRVCODE_REMOTEID_STOP_SNIFFER,
	//REMOTEID SECTION END
    
    //DMA BEGIN
    EAP_DRVCODE_DMA_FFT_DATA,
    EAP_DRVCODE_DMA_DET_DATA,
    //DMA END

    EAP_DRVCODE_GET_REMOTE_ADDR,
    EAP_DRVCODE_C2_HEARTBEAT_TIMEOUT,

    //typec heartbeat
    EAP_DRVCODE_TYPEC_SET_HEARTBEAT_FLAG,

    //CONTROL RF
    EAP_DRVCODE_SET_RF_ENABLE,
    EAP_DRVCODE_GET_RF_ENABLE,

	//bluetooth
	EAP_DRVCODE_BLUTTOOTH_SET_NETWORK,

	//频谱侦测采集时频图数据
	EAP_DRVCODE_COLLECT_DATA,

	EAP_DRVCODE_GET_DEVICE_HARDWARE_INFO,
	//traces shrd106l
	EAP_DRVCODE_VEHICLE_MOUNTED_FIRED,
	EAP_DRVCODE_VEHICLE_MOUNTED_FIRED_MODE,
#ifndef __UNITTEST__
    EAP_DRVCODE_MAX
#else
    EAP_DRVCODE_MAX = 200
#endif
};

#define EAP_GET_DRVCODE_STR(drvCode) \
    { \
        switch (drvCode) { \
        EAP_CASE_CMD(EAP_DRVCODE_BEGIN); \
        EAP_CASE_CMD(EAP_DRVCODE_GET_SN); \
        EAP_CASE_CMD(EAP_DRVCODE_SET_SN); \
        EAP_CASE_CMD(EAP_DRVCODE_CLI_OUT); \
        EAP_CASE_CMD(EAP_DRVCODE_RPT_INFO_BY_ALINK); \
        EAP_CASE_CMD(EAP_DRVCODE_GET_DRONE_FREQ); \
        EAP_CASE_CMD(EAP_DRVCODE_GET_DRONE_ADC); \
        EAP_CASE_CMD(EAP_DRVCODE_DRONE_PL_TEST); \
        EAP_CASE_CMD(EAP_DRVCODE_ALINK_RESPONSE); \
        EAP_CASE_CMD(EAP_DRVCODE_GET_DETECTION_PARAM); \
        EAP_CASE_CMD(EAP_DRVCODE_SET_DETECT_FREQ); \
        EAP_CASE_CMD(EAP_DRVCODE_SET_GAIN); \
        EAP_CASE_CMD(EAP_DRVCODE_SET_GAIN_AGC); \
        EAP_CASE_CMD(EAP_DRVCODE_GET_ANGLE_INFO); \
        EAP_CASE_CMD(EAP_DRVCODE_GET_ANGLE_INFO_EX); \
        EAP_CASE_CMD(EAP_DRVCODE_SET_ANTENNA_MODE); \
        EAP_CASE_CMD(EAP_DRVCODE_GET_PL_VERSION); \
        EAP_CASE_CMD(EAP_DRVCODE_GET_GPIO_VALUE); \
        EAP_CASE_CMD(EAP_DRVCODE_SET_GPIO_VALUE); \
        EAP_CASE_CMD(EAP_DRVCODE_GET_REGISTER_VALUE); \
        EAP_CASE_CMD(EAP_DRVCODE_SET_REGISTER_VALUE); \
        EAP_CASE_CMD(EAP_DRVCODE_GET_BATTERY_CAPACITY); \
        EAP_CASE_CMD(EAP_DRVCODE_GET_SILENT_MODE); \
        EAP_CASE_CMD(EAP_DRVCODE_GET_ALARM_MODE); \
        EAP_CASE_CMD(EAP_DRVCODE_SET_ALARM_MODE); \
        EAP_CASE_CMD(EAP_DRVCODE_SET_COVERT_MODE); \
        EAP_CASE_CMD(EAP_DRVCODE_SET_LED_STATE); \
        EAP_CASE_CMD(EAP_DRVCODE_SET_MOTOR_STATE); \
        EAP_CASE_CMD(EAP_DRVCODE_SET_BUZZER_STATE); \
        EAP_CASE_CMD(EAP_DRVCODE_SEND_ALINK); \
        EAP_CASE_CMD(EAP_DRVCODE_SET_UAV_STATUS); \
        EAP_CASE_CMD(EAP_DRVCODE_REMOTEID_BUF_USE_FINISH); \
        EAP_CASE_CMD(EAP_DRVCODE_REMOTEID_SET_CHANNEL); \
        EAP_CASE_CMD(EAP_DRVCODE_REMOTEID_START_SNIFFER); \
        EAP_CASE_CMD(EAP_DRVCODE_REMOTEID_STOP_SNIFFER); \
        EAP_CASE_CMD(EAP_DRVCODE_DMA_FFT_DATA); \
        EAP_CASE_CMD(EAP_DRVCODE_DMA_DET_DATA); \
        EAP_CASE_CMD(EAP_DRVCODE_GET_REMOTE_ADDR); \
        default: \
            return "unknown"; \
        } \
    }

typedef struct Drv_Drone_Freq
{
    uint64_t sigFreq;
    uint32_t freqOffset;
} Drv_Drone_Freq;

typedef struct Drv_SetDetectFreq
{
    uint8_t mode; // 0-droneid
    uint8_t freqIndex;
    float freq;
} Drv_SetDetectFreq;

typedef struct Drv_AngleInfo
{
    float roll;
    float pitch;
    float yaw;
} Drv_AngleInfo;

typedef struct Drv_SetAntennaMode
{
    uint8_t mode; // 1: 2.4G    2: 5.8G
    int32_t amp;
} Drv_SetAntennaMode;

typedef struct Drv_GetPLVersion
{
	uint32_t PL_Version_date;
	uint32_t PL_Version_time;
} Drv_GetPLVersion;

typedef struct Drv_GetBatteryInfo
{
	uint8_t batteryCapacity;
	uint8_t batAndAdapterState;
} Drv_GetBatteryInfo;

typedef struct Drv_WarningInfo
{
    int8_t targetAppear;
    uint8_t workmode;
}Drv_WarningInfo;


typedef struct Drv_RemoteIdChnSet
{
	uint32_t chn;
} Drv_RemoteIdChnSet;

typedef struct Drv_HardWareInfo
{
    uint16_t tempFpga;
    uint16_t tempBatBoard1;
    uint16_t tempBatBoard2;
    uint16_t batteryCap;
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
}Drv_HardWareInfo;

#pragma pack(1)
typedef struct SAcc
{
	int16_t a[3];
	int16_t T;
}SAcc_t;
typedef struct SGyro
{
	int16_t w[3];
	int16_t T;
}SGyro_t;
typedef struct SAngle
{
	int16_t Angle[3];
	int16_t T;
}SAngle_t;
typedef struct SMag
{
	int16_t h[3];
	int16_t T;
}SMag_t;
#pragma pack()

typedef struct imu_info 
{
	
	float roll;
	float pitch;
	float yaw;
}imu_info_t;

#ifdef __cplusplus
}
#endif
