/*
 * input.h
 *
 *  Created on:
 *      Author: Ultimate
 */
#ifndef INPUT_H_
#define INPUT_H_

#include "../output/output.h"

#include "../hal.h"

//It has been moved into gpioapp.h
#if 0
typedef enum
{
	MIO_MUTE_SWITCH,
	MIO_SWITCH_MODE0,
	MIO_SWITCH_MODE1,
	MIO_SWITCH_MODE2,
	MIO_SWITCH_MODE3,
	EMIO_CONFIG_SW,
	EMIO_GPS_SW,
	EMIO_TRIGGER,
	EMIO_VCC0_21V_OUT_INT,
	EMIO_FAN_SENSOR,
	EMIO_GYRO_INT2_DRDY,
	EMIO_GYRO_INT1,
	EMIO_ACC_MAGINT,
	EMIO_ACC_INT2_XL,
	EMIO_ACC_INT1_XL,
	EMIO_WIFI_NET_STATUS,
	EMIO_WIFI_WAKEUP_IN,
	EMIO_WIFI_RX1,
	EMIO_CTRL_OUT0,   //
	EMIO_CTRL_OUT1,   //
	EMIO_CTRL_OUT2,   //
	EMIO_CTRL_OUT3,   //
	EMIO_CTRL_OUT4,   //
	EMIO_CTRL_OUT5,   //
	EMIO_CTRL_OUT6,   //
	EMIO_CTRL_OUT7,   //
	EMIO_GPS_PPS,
	EMIO_VCC28V_PG_1V8,
	EMIO_VCC32V_PG_1V8,

    /***For T3 board, begin ******************/
    EMIO_ADL5904_QP, // ADL5904 功率检测超门限，正逻辑(RSV_3V3_IO4 on T2) 1：射频功率超门限 0：射频功率未超门限,
    EMIO_ADL5904_QN, // ADL5904 功率检测超门限，负逻辑(RSV_3V3_IO5 on T2) 1：射频功率未超门限 0：射频功率超门限
    EMIO_ADL5904_VRMS, // ADL5904 射频功率ADC检测，模拟信号(SYSMON_VP_R on T2) 
    MIO_PS_UART1_RX, // RN440 蓝牙串口，SOC 接收(PS_UART1_RX on T2) 
    EMIO_PL_UART1_RX, // MCU通信串口，SOC 接收(PL_UART1_RX on T2) 
    /***For T3 board, end ******************/

	INPUT_NAME_MAX,
}INPUT_NAME;
#endif

typedef struct
{
	INPUT_NAME Name; // Signal name for the GPIO item
	uint8_t Enable;//1 初始化   0 不初始化
	uint8_t GpioMode;
	uint8_t DeviceId;
	uint8_t AxiGpioChl;
	uint16_t Pin;
}INPUT_ARG;

uint8_t Input_GetValue(INPUT_NAME name);
int32_t InputInit(void);
int32_t InputUninit(void);

#endif /* INPUT_H_ */
