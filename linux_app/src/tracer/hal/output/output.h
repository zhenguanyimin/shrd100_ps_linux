/*
 * output.h
 *
 *  Created on:
 *      Author: Ultimate
 */
#ifndef OUTPUT_H_
#define OUTPUT_H_

#include "../hal.h"
#include "../gpio/gpioapp.h"

#if 0
typedef enum
{
	MIO_PS_WD,	//U31:复位芯片喂狗信号,
	MIO_PS_ENET_RESET	,
	MIO_PS_USB_RESET	,
	MIO_PS_USB_CS	,
	MIO_EMMC0_SD0_RSTN,
	MIO_V3_1,
	MIO_V2_1,
	MIO_V1_1,
	MIO_V3_2,
	MIO_V2_2,
	MIO_V1_2,
	MIO_GPIO0_DISPLAY,
	MIO_GPIO1_DISPLAY,
	MIO_USB_5V_EN,
	MIO_EN_1,
	MIO_EN_2,
	MIO_ETH_VCC_1V2_EN,
	MIO_VDD_3V3_EN,
	MIO_VDD_1V8_EN,
	MIO_VCC_5V_EN,
	EMIO_SYS_LED0,
	EMIO_SYS_LED1,
	EMIO_SYS_LED2,
	EMIO_SYS_LED3,

	EMIO_FAN_EN,   //输出高电平风扇供电
	EMIO_BUZZER_EN,   //输出高电平蜂鸣器供电电源输出
	EMIO_VBRATOR_EN,   //高电平马达电源输出

	EMIO_SWITCH_OE2,   //高电平使能I2C电平转换和温度传感器中断输出的电平转换
	EMIO_AVCC_ADJ_EN,   //输出高电平使能运放的电源输出
	EMIO_DAC_3V3_EN,   //高电平输出DAC工作电压
	EMIO_G_CLK_EN,   //高电平使能时钟缓冲输出
	EMIO_AVCC_5V_EN,   //高电平使能运放5V工作电压输出
	EMIO_AVCC_1V8_EN,   //高电平使能ADC1.8V工作电压输出
	EMIO_ADC_RESET,   //低电平复位ADC芯片
	EMIO_ADC_SEN,   //低电平使能ADC通讯功能
	EMIO_SWITCH_5V_EN,   //高电平使能9361 5V输出
	EMIO_ADC_PDN,   //低电平给ADC开启供电
	EMIO_SWITCH_12V_EN,   //高电平使能开关滤波器12V输出
	EMIO_L_CTRL_D0,
	EMIO_L_CTRL_D1,
	EMIO_L_CTRL_D2,
	EMIO_L_CTRL_D3,
	EMIO_L_CTRL_D4,
	EMIO_L_CTRL_D5,
	EMIO_L_CTRL_D6,
	EMIO_SWITCH_OE0,   //控制数控衰减器衰减大小数字量（D0-D3）电平转换的控制开关，高电平开启电平转换
	EMIO_SWITCH_OE1,   //控制数控衰减器衰减大小数字量（D4-D6）电平转换的控制开关，高电平开启电平转换
	EMIO_CTRL_IN3,   //
	EMIO_CTRL_IN2,   //
	EMIO_CTRL_IN1,   //
	EMIO_CTRL_IN0,   //
	EMIO_ACC_CS,   //电子罗盘I2C/SPI mode selection1: SPI idle mode / I2C communication enabled;0: SPI communication mode / I2C disabled
	EMIO_PROG_SET1,   //程序运行状态指示灯控制1
	EMIO_PROG_SET2,   //程序运行状态指示灯控制2
	EMIO_PROG_SET3,   //程序运行状态指示灯控制3
	EMIO_PROG_SET4,   //程序运行状态指示灯控制4
	EMIO_GPS_RESET,   //GPS_RESET
	EMIO_GPS_FORCE_ON,   //GPS_FORCE_ON
	EMIO_SWITCH_OE5,   //上面三个IO口电平转换使能控制，高电平开启电平转换
	EMIO_SWITCH_OE4,   //下面四个IO口电平转换使能控制，高电平开启电平转换
	EMIO_SWITCH_OE7,   //下面四个IO口电平转换使能控制，高电平开启电平转换
	EMIO_SWITCH_OE6,   //下面两个IO口电平转换使能控制，高电平开启电平转换
	EMIO_WIFI_WAKEUP_OUT,   //
	EMIO_WIFI_RST,   //WIFI_RST
	EMIO_GYRO_CS,
	EMIO_GYRO_EN,
	EMIO_WIFI_WAKEUP_IN_OUT,
	EMIO_DX_CTRL_OUT0, //Value = 67
	EMIO_DX_CTRL_OUT1,
	EMIO_CHANNEL_CTRLA_0,
	EMIO_RF_CTLA_0,
	EMIO_RF_CTLB_0,
	EMIO_RF_CTLA_1,
	EMIO_RF_CTLB_1,
	EMIO_RF_CTLA_2,
	EMIO_RF_CTLB_2,
	EMIO_RF_CTLA_3,
	EMIO_RF_CTLB_3,
	EMIO_RF_GPIO_0,
	EMIO_RF_GPIO_1,
	EMIO_RF_GPIO_2,
	EMIO_RF_GPIO_3,

    /***For T3 board, begin ******************/
    EMIO_RN440_3V3_EN, // RN440 电源使能控制 1：使能 0：不使能（缺省）
    EMIO_RG450_3V3_EN, // RG450 电源使能控制 1：使能 0：不使能（缺省）
    EMIO_LT3045_EN, // RN440 射频电路 LDO_5V 电源使能控制(RSV_3V3_IO3 on T2) 1：使能 0：不使能（缺省）
    EMIO_ADL5904_RST, // ADL5904 复位控制 1：复位（缺省） 0：正常工作
    EMIO_PE42442_V1, // PE42442 射频通道切换控制信号 V1(RSV_3V3_IO0 on T2) 
    EMIO_PE42442_V2, // PE42442 射频通道切换控制信号 V2(RSV_3V3_IO6 on T2) 
    EMIO_PE42442_V3, // PE42442 射频通道切换控制信号 V3(RSV_3V3_IO7 on T2) 
    MIO_PS_UART1_TX, // RN440 蓝牙串口，SOC 发送V3(PS_UART1_TX/MIO_VCC_5V_EN on T2) 
    EMIO_PL_UART1_TX, // MCU通信串口，SOC 发送V3(PL_UART1_TX on T2) 
    EMIO_ANT_CTL_5V_EN, //天线5V使能控制(T2包含但未控制)
    EMIO_ANT_CTL3_OE, // ANT_CTL3的输出使能控制 1：不使能 ANT_CTL3 输出（缺省） 0：使能 ANT_CTL3 输出
    EMIO_ANT_CTL3, // 车载天线控制输出信号3
    EMIO_ANT_CTL4, // 车载天线控制输出信号4
    EMIO_ANT_CTL5, // 车载天线控制输出信号5

    EMIO_RN440_H_WK_WL,
    EMIO_RN440_H_WK_BT,
    EMIO_RN440_WL_DIS,
    EMIO_RN440_BT_DIS,
    EMIO_RSV_3V3_IO3,
    
    EMIO_RG450_BT_EN,
    EMIO_RG450_WL_EN,
        
        
    /***For T3 board, end ******************/

    
	OUTPUT_NAME_MAX
} OUTPUT_NAME; //名字
#endif

typedef struct
{
	OUTPUT_NAME Name; // Signal name for the GPIO item
	uint8_t Enable;//1 初始化   0 不初始化
	uint8_t GpioMode;
	uint8_t DeviceId;
	uint8_t AxiGpioChl;
	uint16_t Pin;
	uint8_t InitLevel;//初始电平,1 OR 0
}OUTPUT_ARG;
uint8_t XGpioPs_ReadPin(int gpio_fd);

int32_t OutputInit(void);
int32_t OutputUninit(void);
void GPIO_OutputCtrl(OUTPUT_NAME name, uint8_t value);
uint8_t Output_GetValue(OUTPUT_NAME name);
// void write_register(uint32_t regAddr, uint32_t value);
#endif /* OUTPUT_H_ */
