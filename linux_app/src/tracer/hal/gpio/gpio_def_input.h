/**************************************************************************
    Notice: 
        1.  请不要直接include本文件！！！
        2.  请include "gpioapp.h来访问相应的枚举值以及名称对应关系
        3.  这个文件用来在gpioapp.h中定义INPUT_NAME
        4.  枚举名字和值的对应关系定义在gpio_common.cpp:_sGpioInputEnumNameMap中
****************************************************************************/

DEFINE_GPIO_ITEM(MIO_MUTE_SWITCH)
DEFINE_GPIO_ITEM(MIO_SWITCH_MODE0)
DEFINE_GPIO_ITEM(MIO_SWITCH_MODE1)
DEFINE_GPIO_ITEM(MIO_SWITCH_MODE2)
DEFINE_GPIO_ITEM(MIO_SWITCH_MODE3)
DEFINE_GPIO_ITEM(EMIO_CONFIG_SW)
DEFINE_GPIO_ITEM(EMIO_GPS_SW)
DEFINE_GPIO_ITEM(EMIO_TRIGGER)
DEFINE_GPIO_ITEM(EMIO_VCC0_21V_OUT_INT)
DEFINE_GPIO_ITEM(EMIO_FAN_SENSOR)
DEFINE_GPIO_ITEM(EMIO_GYRO_INT2_DRDY)
DEFINE_GPIO_ITEM(EMIO_GYRO_INT1)
DEFINE_GPIO_ITEM(EMIO_ACC_MAGINT)
DEFINE_GPIO_ITEM(EMIO_ACC_INT2_XL)
DEFINE_GPIO_ITEM(EMIO_ACC_INT1_XL)
DEFINE_GPIO_ITEM(EMIO_WIFI_NET_STATUS)
DEFINE_GPIO_ITEM(EMIO_WIFI_WAKEUP_IN)
DEFINE_GPIO_ITEM(EMIO_WIFI_RX1)
DEFINE_GPIO_ITEM(EMIO_CTRL_OUT0)
DEFINE_GPIO_ITEM(EMIO_CTRL_OUT1)
DEFINE_GPIO_ITEM(EMIO_CTRL_OUT2)
DEFINE_GPIO_ITEM(EMIO_CTRL_OUT3)
DEFINE_GPIO_ITEM(EMIO_CTRL_OUT4)
DEFINE_GPIO_ITEM(EMIO_CTRL_OUT5)
DEFINE_GPIO_ITEM(EMIO_CTRL_OUT6)
DEFINE_GPIO_ITEM(EMIO_CTRL_OUT7)
DEFINE_GPIO_ITEM(EMIO_GPS_PPS)
DEFINE_GPIO_ITEM(EMIO_VCC28V_PG_1V8)
DEFINE_GPIO_ITEM(EMIO_VCC32V_PG_1V8)

/***For T3 board, begin ******************/
DEFINE_GPIO_ITEM(EMIO_ADL5904_QP) // ADL5904 功率检测超门限，正逻辑(RSV_3V3_IO4 on T2) 1：射频功率超门限 0：射频功率未超门限,
DEFINE_GPIO_ITEM(EMIO_ADL5904_QN) // ADL5904 功率检测超门限，负逻辑(RSV_3V3_IO5 on T2) 1：射频功率未超门限 0：射频功率超门限
DEFINE_GPIO_ITEM(EMIO_ADL5904_VRMS) // ADL5904 射频功率ADC检测，模拟信号(SYSMON_VP_R on T2) 
DEFINE_GPIO_ITEM(MIO_PS_UART1_RX) // RN440 蓝牙串口，SOC 接收(PS_UART1_RX on T2) 
DEFINE_GPIO_ITEM(EMIO_PL_UART1_RX) // MCU通信串口，SOC 接收(PL_UART1_RX on T2) 
/***For T3 board, end ******************/

DEFINE_GPIO_ITEM(INPUT_NAME_MAX)