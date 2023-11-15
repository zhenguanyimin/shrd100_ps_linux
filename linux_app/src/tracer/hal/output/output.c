#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
//#include <stdbool.h>
#include "output.h"
#include "../input/input.h"
#include "../../TracerUtils_c.h"


#define PL_DEVICE_MAX        10

static const OUTPUT_ARG* OutputArg = NULL;

//const  OUTPUT_ARG OutputArg_T2[OUTPUT_NAME_MAX] =
static const  OUTPUT_ARG OutputArg_T2[] ={
    // {MIO_PS_WD           , ENABLE, MIO, GPIO_DEVICE_ID, 0, 6, IO_VOL},
    // {MIO_PS_ENET_RESET   , ENABLE, MIO, GPIO_DEVICE_ID, 0, 9, IO_VOH},
    // {MIO_EMMC0_SD0_RSTN  , ENABLE, MIO, GPIO_DEVICE_ID, 0, 23, IO_VOH},
    {MIO_V3_1            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 26, IO_VOL},
    {MIO_V2_1            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 27, IO_VOL},
    {MIO_V1_1            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 28, IO_VOL},
    {MIO_V3_2            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 29, IO_VOL},
    {MIO_V2_2            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 30, IO_VOL},
    {MIO_V1_2            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 31, IO_VOL},
    {MIO_GPIO0_DISPLAY   , ENABLE, MIO, GPIO_DEVICE_ID, 0, 36, IO_VOL},
    {MIO_GPIO1_DISPLAY   , ENABLE, MIO, GPIO_DEVICE_ID, 0, 37, IO_VOL},
    {MIO_USB_5V_EN       , ENABLE, MIO, GPIO_DEVICE_ID, 0, 38, IO_VOH},
    {MIO_EN_2            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 40, IO_VOH},
    // {MIO_ETH_VCC_1V2_EN  , ENABLE, MIO, GPIO_DEVICE_ID, 0, 41, IO_VOH},
    {MIO_VDD_3V3_EN      , ENABLE, MIO, GPIO_DEVICE_ID, 0, 42, IO_VOH},
    {MIO_VDD_1V8_EN      , ENABLE, MIO, GPIO_DEVICE_ID, 0, 43, IO_VOH},
    {MIO_VCC_5V_EN       , ENABLE, MIO, GPIO_DEVICE_ID, 0, 44, IO_VOH},

    // TODO: Need check--begin
    {EMIO_DX_CTRL_OUT0        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 10, IO_VOH},
    {EMIO_DX_CTRL_OUT1        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 11, IO_VOH},
    // TODO: Need check--end

    {EMIO_SYS_LED0        , ENABLE, MIO, GPIO_DEVICE_ID, 0, 24, IO_VOH},
    {EMIO_SYS_LED1        , ENABLE, MIO, GPIO_DEVICE_ID, 0, 25, IO_VOL},
    {EMIO_SYS_LED2        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 16, IO_VOL},
    {EMIO_SYS_LED3        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 15, IO_VOL},
    {EMIO_FAN_EN         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 17, IO_VOH},
    {EMIO_VBRATOR_EN     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 19, IO_VOL},

    {EMIO_RF_CTLA_0        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 1, IO_VOH},
    {EMIO_RF_CTLA_1        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 2, IO_VOH},
    {EMIO_RF_CTLA_2        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 3, IO_VOH},
    {EMIO_RF_CTLA_3        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 4, IO_VOH},
    {EMIO_RF_CTLB_0        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 5, IO_VOH},
    {EMIO_RF_CTLB_1        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 6, IO_VOH},
    {EMIO_RF_CTLB_2        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 7, IO_VOH},
    {EMIO_RF_CTLB_3        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 8, IO_VOH},
    {EMIO_SWITCH_5V_EN      , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 9, IO_VOH},

    {EMIO_GYRO_EN       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 27, IO_VOH},

    //{EMIO_RSV_3V3_IO0, DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 65, IO_VOL}, // PE42442 射频通道切换控制信号 V1
    //{EMIO_RSV_3V3_IO6, DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 71, IO_VOL}, // PE42442 射频通道切换控制信号 V2
    //{EMIO_RSV_3V3_IO7, DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 72, IO_VOH}, // PE42442 射频通道切换控制信号 V3

    //  {EMIO_WIFI_WAKEUP_IN_OUT    , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 64, IO_VOL},

    {OUTPUT_NAME_MAX    , ENABLE, EMIO, GPIO_DEVICE_ID, 0, 0, IO_VOL}
};


// TODO: GIPO configuration for T3 board
static const  OUTPUT_ARG OutputArg_T3[] =
{
    // {MIO_PS_WD           , ENABLE, MIO, GPIO_DEVICE_ID, 0, 6, IO_VOL},
    // {MIO_PS_ENET_RESET   , ENABLE, MIO, GPIO_DEVICE_ID, 0, 9, IO_VOH},
    // {MIO_EMMC0_SD0_RSTN  , ENABLE, MIO, GPIO_DEVICE_ID, 0, 23, IO_VOH},
    {MIO_V3_1            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 26, IO_VOL},
    {MIO_V2_1            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 27, IO_VOL},
    {MIO_V1_1            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 28, IO_VOL},
    {MIO_V3_2            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 29, IO_VOL},
    {MIO_V2_2            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 30, IO_VOL},
    {MIO_V1_2            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 31, IO_VOL},

    // TODO: Need check--begin
    {EMIO_DX_CTRL_OUT0        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 10, IO_VOH},
    {EMIO_DX_CTRL_OUT1        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 11, IO_VOH},
    // TODO: Need check--end

    {MIO_GPIO0_DISPLAY   , ENABLE, MIO, GPIO_DEVICE_ID, 0, 36, IO_VOL},
    {MIO_GPIO1_DISPLAY   , ENABLE, MIO, GPIO_DEVICE_ID, 0, 37, IO_VOL},
    
    {MIO_USB_5V_EN       , ENABLE, MIO, GPIO_DEVICE_ID, 0, 38, IO_VOH},
    {MIO_EN_2            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 40, IO_VOH},
    // {MIO_ETH_VCC_1V2_EN  , ENABLE, MIO, GPIO_DEVICE_ID, 0, 41, IO_VOH},
    {MIO_VDD_3V3_EN      , ENABLE, MIO, GPIO_DEVICE_ID, 0, 42, IO_VOH},
    {MIO_VDD_1V8_EN      , ENABLE, MIO, GPIO_DEVICE_ID, 0, 43, IO_VOH},
    {MIO_VCC_5V_EN       , ENABLE, MIO, GPIO_DEVICE_ID, 0, 44, IO_VOH}, //MIO_PS_UART1_TX or T3

    {EMIO_SYS_LED0        , ENABLE, MIO, GPIO_DEVICE_ID, 0, 24, IO_VOH},
    {EMIO_SYS_LED1        , ENABLE, MIO, GPIO_DEVICE_ID, 0, 25, IO_VOL},
    {EMIO_SYS_LED2        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 16, IO_VOL},
    {EMIO_SYS_LED3        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 15, IO_VOL},
    {EMIO_FAN_EN         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 17, IO_VOH},
    {EMIO_VBRATOR_EN     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 19, IO_VOL},

    {EMIO_RF_CTLA_0        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 1, IO_VOH},
    {EMIO_RF_CTLA_1        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 2, IO_VOH},
    {EMIO_RF_CTLA_2        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 3, IO_VOH},
    {EMIO_RF_CTLA_3        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 4, IO_VOH},
    {EMIO_RF_CTLB_0        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 5, IO_VOH},
    {EMIO_RF_CTLB_1        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 6, IO_VOH},
    {EMIO_RF_CTLB_2        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 7, IO_VOH},
    {EMIO_RF_CTLB_3        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 8, IO_VOH},
    {EMIO_SWITCH_5V_EN     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 9, IO_VOH},

    {EMIO_RF_GPIO_0        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 18, IO_VOL},
    {EMIO_RF_GPIO_1        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 73, IO_VOL},
    {EMIO_RF_GPIO_2        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 74, IO_VOL},
    {EMIO_RF_GPIO_3        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 75, IO_VOL},
    {EMIO_RF_GPIO_4        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 76, IO_VOH},
    {EMIO_RF_GPIO_5        , DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 77, IO_VOL},
    {EMIO_RF_GPIO_6        , DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 78, IO_VOL},
    {EMIO_RF_GPIO_7        , DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 79, IO_VOL},

    {EMIO_GYRO_EN       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 27, IO_VOH},

    //  {EMIO_WIFI_WAKEUP_IN_OUT    , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 64, IO_VOL},

    /***For T3 board, begin ******************/
    //Below 3 GPIOs have been initialized in uBoot
#if 0
    {EMIO_RN440_3V3_EN     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 80, IO_VOH},
    {EMIO_RG450_3V3_EN     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 81, IO_VOH},
    {EMIO_LT3045_EN        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 68, IO_VOH},
#endif

    // {EMIO_ADL5904_RST      , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 82, IO_VOH}, // TODO: Need check the behavior
    /************************************************
        Mode      V3,V2,V1
        RF1 on      001      --- 射频信号到 RN440(Default)
        RF2 on      010      --- 射频信号旁路(保护)
        RF3 on      011      --- 未用
        RF4 on      100      --- 射频信号到 射频板"
    *****************************************************************/

    // 射频信号到 RN440(Default)
    {EMIO_PE42442_V1       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 65, IO_VOH}, 
    {EMIO_PE42442_V2       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 71, IO_VOL}, 
    {EMIO_PE42442_V3       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 72, IO_VOL}, 

    // 机载Tracer需要开启天线5V输出,供电天线开关板
    {EMIO_ANT_CTL_5V_EN    , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 56, IO_VOH},
    // TODO: Need check 
#if 0
    {MIO_PS_UART1_TX      , ENABLE, MIO, GPIO_DEVICE_ID, 0, 44, IO_VOH}, // MIO_VCC_5V_EN on T2
    // {EMIO_PL_UART1_TX      , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 101, IO_VOH}, // TODO initLevel=??, offSet=???

    {EMIO_ANT_CTL3_OE      , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 83, IO_VOL},
    {EMIO_ANT_CTL3         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 84, IO_VOL}, // TODO initLevel=??
    {EMIO_ANT_CTL4         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 85, IO_VOL}, // TODO initLevel=??
    {EMIO_ANT_CTL5         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 86, IO_VOL}, // TODO initLevel=??
#endif

    {EMIO_RN440_WL_DIS      , DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 41, IO_VOH}, 
    {EMIO_RN440_BT_DIS      , DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 42, IO_VOH}, 
    {EMIO_RG450_BT_EN       , DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 21, IO_VOH}, 
    {EMIO_RG450_WL_EN       , DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 22, IO_VOH}, 
    /***For T3 board, end ******************/

    {OUTPUT_NAME_MAX    , ENABLE, EMIO, GPIO_DEVICE_ID, 0, 0, IO_VOL}


};

// 车载Tracer专用,按键板替换为功放板
static const  OUTPUT_ARG OutputArg_T3_1[] =
{
    // {MIO_PS_WD           , ENABLE, MIO, GPIO_DEVICE_ID, 0, 6, IO_VOL},
    // {MIO_PS_ENET_RESET   , ENABLE, MIO, GPIO_DEVICE_ID, 0, 9, IO_VOH},
    // {MIO_EMMC0_SD0_RSTN  , ENABLE, MIO, GPIO_DEVICE_ID, 0, 23, IO_VOH},
    {MIO_V3_1            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 26, IO_VOL},
    {MIO_V2_1            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 27, IO_VOL},
    {MIO_V1_1            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 28, IO_VOL},
    {MIO_V3_2            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 29, IO_VOL},
    {MIO_V2_2            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 30, IO_VOL},
    {MIO_V1_2            , ENABLE, MIO, GPIO_DEVICE_ID, 0, 31, IO_VOL},

    // TODO: Need check--begin
    {EMIO_DX_CTRL_OUT0        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 10, IO_VOH},
    {EMIO_DX_CTRL_OUT1        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 11, IO_VOH},
    // TODO: Need check--end

    {MIO_GPIO0_DISPLAY   , ENABLE, MIO, GPIO_DEVICE_ID, 0, 36, IO_VOL},
    
    // {MIO_ETH_VCC_1V2_EN  , ENABLE, MIO, GPIO_DEVICE_ID, 0, 41, IO_VOH},
    {MIO_VDD_3V3_EN      , ENABLE, MIO, GPIO_DEVICE_ID, 0, 42, IO_VOH},
    {MIO_VCC_5V_EN       , ENABLE, MIO, GPIO_DEVICE_ID, 0, 44, IO_VOH}, //MIO_PS_UART1_TX or T3

    {EMIO_SYS_LED0        , ENABLE, MIO, GPIO_DEVICE_ID, 0, 24, IO_VOH},
    {EMIO_SYS_LED1        , ENABLE, MIO, GPIO_DEVICE_ID, 0, 25, IO_VOL},
    {EMIO_SYS_LED2        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 16, IO_VOL},
    {EMIO_SYS_LED3        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 15, IO_VOL},
    {EMIO_FAN_EN         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 17, IO_VOH},
    {EMIO_VBRATOR_EN     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 19, IO_VOL},

    {EMIO_RF_CTLA_0        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 1, IO_VOH},
    {EMIO_RF_CTLA_1        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 2, IO_VOH},
    {EMIO_RF_CTLA_2        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 3, IO_VOH},
    {EMIO_RF_CTLA_3        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 4, IO_VOH},
    {EMIO_RF_CTLB_0        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 5, IO_VOH},
    {EMIO_RF_CTLB_1        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 6, IO_VOH},
    {EMIO_RF_CTLB_2        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 7, IO_VOH},
    {EMIO_RF_CTLB_3        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 8, IO_VOH},
    {EMIO_SWITCH_5V_EN     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 9, IO_VOH},

    {EMIO_RF_GPIO_0        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 18, IO_VOL},
    {EMIO_RF_GPIO_1        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 73, IO_VOL},
    {EMIO_RF_GPIO_2        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 74, IO_VOL},
    {EMIO_RF_GPIO_3        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 75, IO_VOL},
    {EMIO_RF_GPIO_4        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 76, IO_VOH},
    {EMIO_RF_GPIO_5        , DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 77, IO_VOL},
    {EMIO_RF_GPIO_6        , DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 78, IO_VOL},
    {EMIO_RF_GPIO_7        , DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 79, IO_VOL},

    {EMIO_GYRO_EN       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 27, IO_VOH},

    //  {EMIO_WIFI_WAKEUP_IN_OUT    , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 64, IO_VOL},

    /***For T3 board, begin ******************/
    //Below 3 GPIOs have been initialized in uBoot
#if 0
    {EMIO_RN440_3V3_EN     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 80, IO_VOH},
    {EMIO_RG450_3V3_EN     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 81, IO_VOH},
    {EMIO_LT3045_EN        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 68, IO_VOH},
#endif

    // {EMIO_ADL5904_RST      , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 82, IO_VOH}, // TODO: Need check the behavior
    /************************************************
        Mode      V3,V2,V1
        RF1 on      001      --- 射频信号到 RN440(Default)
        RF2 on      010      --- 射频信号旁路(保护)
        RF3 on      011      --- 未用
        RF4 on      100      --- 射频信号到 射频板"
    *****************************************************************/

    // 射频信号到 RN440(Default)
    {EMIO_PE42442_V1       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 65, IO_VOH}, 
    {EMIO_PE42442_V2       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 71, IO_VOL}, 
    {EMIO_PE42442_V3       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 72, IO_VOL}, 

    // 机载Tracer需要开启天线5V输出,供电天线开关板
    {EMIO_ANT_CTL_5V_EN    , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 56, IO_VOH},
    // TODO: Need check 
#if 0
    {MIO_PS_UART1_TX      , ENABLE, MIO, GPIO_DEVICE_ID, 0, 44, IO_VOH}, // MIO_VCC_5V_EN on T2
    // {EMIO_PL_UART1_TX      , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 101, IO_VOH}, // TODO initLevel=??, offSet=???

    {EMIO_ANT_CTL3_OE      , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 83, IO_VOL},
    {EMIO_ANT_CTL3         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 84, IO_VOL}, // TODO initLevel=??
    {EMIO_ANT_CTL4         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 85, IO_VOL}, // TODO initLevel=??
    {EMIO_ANT_CTL5         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 86, IO_VOL}, // TODO initLevel=??
#endif

    {EMIO_RN440_WL_DIS      , DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 41, IO_VOH}, 
    {EMIO_RN440_BT_DIS      , DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 42, IO_VOH}, 
    {EMIO_RG450_BT_EN       , DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 21, IO_VOH}, 
    {EMIO_RG450_WL_EN       , DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 22, IO_VOH}, 
    /***For T3 board, end ******************/
	{MIO_SWITCH_CHANNEL_0		, ENABLE, MIO, GPIO_DEVICE_ID, 0, 38, IO_VOL},					   // tracer-s channel 0
	{MIO_SWITCH_CHANNEL_1		, ENABLE, MIO, GPIO_DEVICE_ID, 0, 40, IO_VOL},					   // tracer-s channel 1
	{MIO_SWITCH_CHANNEL_2		, ENABLE, MIO, GPIO_DEVICE_ID, 0, 43, IO_VOL},					   // tracer-s channel 2
	{MIO_SWITCH_CHANNEL_3		, ENABLE, MIO, GPIO_DEVICE_ID, 0, 8,  IO_VOL},					   // tracer-s channel 3
	{EMIO_SWITCH_CHANNEL_4		, ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 86, IO_VOL},	   // tracer-s channel 4
	{MIO_SWITCH_CHANNEL_5		, ENABLE, MIO, GPIO_DEVICE_ID, 0, 37, IO_VOL},					   // tracer-s channel 5

    {OUTPUT_NAME_MAX    , ENABLE, EMIO, GPIO_DEVICE_ID, 0, 0, IO_VOL}


};


#if 1 // Linux

int32_t OutputInit(void)
{

    int32_t Status = 0;
    int pin;

    switch(get_device_type())
    {
        case BOARD_TYPE_T2:
            EAP_LOG_DEBUG("BOARD_TYPE_T2\n");
            OutputArg = OutputArg_T2;
            break;
        case BOARD_TYPE_T3:
            EAP_LOG_DEBUG("BOARD_TYPE_T3\n");
            OutputArg = OutputArg_T3;
            break;
        case BOARD_TYPE_T3_1:
            EAP_LOG_DEBUG("BOARD_TYPE_T3_1\n");
            OutputArg = OutputArg_T3_1;
            break;
        default:
            EAP_LOG_DEBUG("get board type err!\n");
            return -1;
    }

    for(int i = 0; i < 0xFFFF; ++i)
    {
        if(OUTPUT_NAME_MAX == OutputArg[i].Name)
        {
            break;
        }

        if(OutputArg[i].Enable == ENABLE)
        {
            if ((OutputArg[i].GpioMode == MIO) || (OutputArg[i].GpioMode == EMIO))
            {
                pin = OutputArg[i].Pin + LINUX_GPIO_OFFSET;
                gpio_unexport(pin);
                gpio_export(pin);
                gpio_direction(pin, 1);
                gpio_write(pin, OutputArg[i].InitLevel);
            }
        }
    }


    return Status;
}

int32_t OutputUninit(void)
{
    int32_t Status = 0;
    int pin;

    for(int i = 0; i < 0xFFFF; ++i)
    {
        if(OUTPUT_NAME_MAX == OutputArg[i].Name)
        {
            break;
        }

        if(OutputArg[i].Enable == ENABLE && i != EMIO_SWITCH_5V_EN)
        {
            if ((OutputArg[i].GpioMode == MIO) || (OutputArg[i].GpioMode == EMIO))
            {
                pin = OutputArg[i].Pin + LINUX_GPIO_OFFSET;
                gpio_unexport(pin);
            }
        }
    }
    
    printf("%s():****Success to uninstall output GPIO.\n", __FUNCTION__);

    return Status;
}
int GetGpioArrayIdndexByOutputName(OUTPUT_NAME pName)
{
    int index = 0;
    if( OUTPUT_NAME_MAX <= pName)
    {
        return -1;
    }

    for(; index < 0xFFFF; ++index)
    {
        if(OUTPUT_NAME_MAX == OutputArg[index].Name)
        {
            break;
        }
        if(pName == OutputArg[index].Name)
        {
            return index;
        }
    }

    //not found
    return -1;
}

void GPIO_OutputCtrl(OUTPUT_NAME name, uint8_t value)
{
    int gpioOutputIndex = GetGpioArrayIdndexByOutputName(name);

    if (gpioOutputIndex >= 0)
    {
        int pin;
        if (( OutputArg[gpioOutputIndex].Enable == ENABLE) || ( OutputArg[gpioOutputIndex].Enable == DISABLE))
        {
            if ((OutputArg[gpioOutputIndex].GpioMode == MIO) || (OutputArg[gpioOutputIndex].GpioMode == EMIO))
            {
                if ((OutputArg[gpioOutputIndex].GpioMode == MIO) || (OutputArg[gpioOutputIndex].GpioMode == EMIO))
                {
                    pin = OutputArg[gpioOutputIndex].Pin + LINUX_GPIO_OFFSET;
                    gpio_write(pin, value);
                    // printf("gpio_write: pin=%d, value=%d\n", pin, value);
                }
            }
        }
        else
        {
            printf("%s():****Info*****, GPIO is disable.! OUTPUT_NAME = %d, targetValue=%d\n", __FUNCTION__, (uint8_t)name, value);
        }

    }
    else
    {
        printf("%s()###Error*****, GPIO is not found! OUTPUT_NAME = %d, targetValue=%d\n", __FUNCTION__, (uint8_t)name, value);
    }
}

uint8_t Output_GetValue(OUTPUT_NAME name)
{
    uint8_t status = 0;

    int gpioOutputIndex = GetGpioArrayIdndexByOutputName(name);
    if (gpioOutputIndex >= 0)
    {
        int pin;
        if(OutputArg[gpioOutputIndex].Enable == ENABLE)
        {
            if ((OutputArg[gpioOutputIndex].GpioMode == MIO) || (OutputArg[gpioOutputIndex].GpioMode == EMIO))
            {
                pin = OutputArg[gpioOutputIndex].Pin + LINUX_GPIO_OFFSET;
                status = (uint8_t)gpio_read(pin);
            }
        }
        else
        {
            printf("%s():****Info*****, GPIO is disable.! OUTPUT_NAME = %d\n", __FUNCTION__, (uint8_t)name);
        }

    }
    else
    {
        printf("%s()###Error*****, GPIO is not found! OUTPUT_NAME = %d\n", __FUNCTION__, (uint8_t)name);
    }

    return status;
}

// TODO: Need remove
#if 0
int32_t OutputInit(void)
{
    
    int32_t Status = 0;
    int pin;

    for(uint16_t i = 0; i < OUTPUT_NAME_MAX; i++)
    {
        if(OutputArg[i].Enable == ENABLE)
        {
            if ((OutputArg[i].GpioMode == MIO) || (OutputArg[i].GpioMode == EMIO))
            {
                pin = OutputArg[i].Pin + LINUX_GPIO_OFFSET;
                gpio_unexport(pin);
                gpio_export(pin);
                gpio_direction(pin, 1);
                gpio_write(pin, OutputArg[i].InitLevel);
            }
        }
    }

    return Status;
}

int32_t OutputUninit(void)
{
    int32_t Status = 0;
    int pin;

    for(uint16_t i = 0; i < OUTPUT_NAME_MAX; i++)
    {
        if(OutputArg[i].Enable == ENABLE && i != EMIO_SWITCH_5V_EN)
        {
            if ((OutputArg[i].GpioMode == MIO) || (OutputArg[i].GpioMode == EMIO))
            {
                pin = OutputArg[i].Pin + LINUX_GPIO_OFFSET;
                gpio_unexport(pin);
            }
        }
    }

    return Status;
}

void GPIO_OutputCtrl(OUTPUT_NAME name, uint8_t value)
{
    uint8_t i = (uint8_t)name;
    int pin;
    if ((i < OUTPUT_NAME_MAX) && (OutputArg[i].Enable == ENABLE))
    {
        if ((OutputArg[i].GpioMode == MIO) || (OutputArg[i].GpioMode == EMIO))
        {
            pin = OutputArg[i].Pin + LINUX_GPIO_OFFSET;
            gpio_write(pin, value);
            // printf("gpio_write: pin=%d, value=%d\n", pin, value);
        }
    }
}

uint8_t Output_GetValue(OUTPUT_NAME name)
{
    uint8_t i = (uint8_t)name;
    uint8_t status = 0;
    int pin;

    if((i < OUTPUT_NAME_MAX) && (OutputArg[i].Enable == ENABLE))
    {
        if ((OutputArg[i].GpioMode == MIO) || (OutputArg[i].GpioMode == EMIO))
        {
            pin = OutputArg[i].Pin + LINUX_GPIO_OFFSET;
            status = (uint8_t)gpio_read(pin);
        }
    }

    return status;
}
#endif //#if 0 , need to delete

#else
/**
 * The output pin is initialized
 */
int32_t OutputInit(void)
{
    int32_t Status;
    XGpioPs_Config *ConfigPtr;
    uint32_t GpioVal;
    uint8_t PlId;

    ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
    Status = XGpioPs_CfgInitialize(&PsGpio, ConfigPtr, ConfigPtr->BaseAddr);
    if (Status != XST_SUCCESS)
    {
        LOG_ERROR("MIO EMIO config failed!\r\n");
        return Status;
    }

    for(uint16_t i = 0; i < OUTPUT_NAME_MAX; i++)
    {
        if(OutputArg[i].Enable == ENABLE)
        {
            if ((OutputArg[i].GpioMode == MIO) || (OutputArg[i].GpioMode == EMIO))
            {
                XGpioPs_SetDirectionPin(&PsGpio, OutputArg[i].Pin, OUTPUT);
                XGpioPs_SetOutputEnablePin(&PsGpio, OutputArg[i].Pin, OutputArg[i].Enable);
                XGpioPs_WritePin(&PsGpio, OutputArg[i].Pin, OutputArg[i].InitLevel);
            }
            else if ((OutputArg[i].GpioMode == AXI_GPIO) && (OutputArg[i].DeviceId < PL_DEVICE_MAX))
            {
                PlId = OutputArg[i].DeviceId;
                Status = XGpio_Initialize(&PlGpio[PlId], OutputArg[i].DeviceId);
                if (Status != XST_SUCCESS)
                {
                    LOG_ERROR("AXI GPIO config failed!\r\n");
                    break;
                }
                GpioVal = XGpio_GetDataDirection(&PlGpio[PlId], OutputArg[i].AxiGpioChl);
                GpioVal = GpioVal & (0xFFFFFFFE << OutputArg[i].Pin);
                XGpio_SetDataDirection(&PlGpio[PlId], OutputArg[i].AxiGpioChl, GpioVal);//channel x xbit output

                GpioVal = XGpio_DiscreteRead(&PlGpio[PlId], OutputArg[i].AxiGpioChl);
                GpioVal = GpioVal | (OutputArg[i].InitLevel << OutputArg[i].Pin);
                XGpio_DiscreteWrite(&PlGpio[PlId], OutputArg[i].AxiGpioChl, GpioVal);
            }
            else if (OutputArg[i].GpioMode == AXI_BRAM)
            {
                axi_write_data(OutputArg[i].Pin, OutputArg[i].InitLevel);
            }
        }
    }

    return Status;
}


/**
 * 给指定的IO口输出指定的value 电平信号；
 */
void GPIO_OutputCtrl(OUTPUT_NAME name, uint8_t value)
{
//    axi_write_data(name, value);
    uint32_t GpioVal;
    uint8_t i = (uint8_t)name;
    if ((i < OUTPUT_NAME_MAX) && (OutputArg[i].Enable == ENABLE))
    {
        if ((OutputArg[i].GpioMode == MIO) || (OutputArg[i].GpioMode == EMIO))
        {
            XGpioPs_WritePin(&PsGpio, OutputArg[i].Pin, value);
        }
        else if ((OutputArg[i].GpioMode == AXI_GPIO) && (OutputArg[i].DeviceId < PL_DEVICE_MAX))
        {
            GpioVal = XGpio_DiscreteRead(&PlGpio[OutputArg[i].DeviceId], (unsigned)OutputArg[i].AxiGpioChl);
            GpioVal = GpioVal | (value << OutputArg[i].Pin);
            XGpio_DiscreteWrite(&PlGpio[OutputArg[i].DeviceId], (unsigned)OutputArg[i].AxiGpioChl, GpioVal);
        }
        else if (OutputArg[i].GpioMode == AXI_BRAM)
        {
            axi_write_data(OutputArg[i].Pin, value);
        }
    }
}

/**
 * 读取引脚电平
 * @param name 引脚名字
 * @return    1: 高电平     0: 低电平
 */
uint8_t Output_GetValue(OUTPUT_NAME name)
{
//    return (uint8_t)axi_read_data(name);
    uint32_t GpioVal;
    uint8_t i = (uint8_t)name;
    uint8_t status;

    XGpioPs *pPsGpio;
    XGpio *pPlGpio;

    if ((i < OUTPUT_NAME_MAX) && (OutputArg[i].Enable == ENABLE))
    {
        if ((OutputArg[i].GpioMode == MIO) || (OutputArg[i].GpioMode == EMIO))
        {
            pPsGpio = GetPsGpioDev();
            status = (uint8_t)XGpioPs_ReadPin(pPsGpio, OutputArg[i].Pin);
        }
        else if (OutputArg[i].GpioMode == AXI_GPIO)
        {
            pPlGpio = GetPlGpioDev(OutputArg[i].DeviceId);
            GpioVal = XGpio_DiscreteRead(pPlGpio, OutputArg[i].AxiGpioChl);
            GpioVal = GpioVal & (1 << OutputArg[i].Pin);
            status = (GpioVal == 0);
        }
        else if (OutputArg[i].GpioMode == AXI_BRAM)
        {
            status = (uint8_t)axi_read_data(name);
        }
    }

    return status;
}
/**
 * write register of frequency；
 */
void write_register(uint32_t regAddr, uint32_t value)
{
    axi_write_data(regAddr, value);
}
#endif

