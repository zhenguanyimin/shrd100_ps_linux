
/*
 * hal.h
 *
 *  Created on:
 *      Author: Ultimate
 */
#ifndef HAL_H_
#define HAL_H_
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef __UNITTEST__
#include <sys/mman.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include "eap_pub.h"
// #include "xil_io.h"

#define LOG_DEBUG lLOG_DEBUG

// Refer RTOS xparameters.h
/* Canonical definitions for peripheral AXI_BRAM_CTRL_PARA */
#define XPAR_BRAM_0_BASEADDR 0x80000000U
#define XPAR_BRAM_0_HIGHADDR 0x8000FFFFU
#define XPAR_BRAM_LENGTH     0x10000

/* Definitions for interface M08_AXI */
#define XPAR_M08_AXI_BASEADDR 0x81000000
#define XPAR_M08_AXI_HIGHADDR 0x81000FFF
/* Definitions for interface M09_AXI */
#define XPAR_M09_AXI_BASEADDR 0x81010000
#define XPAR_M09_AXI_HIGHADDR 0x81010FFF
#define XPAR_AXI_LENGTH       0x1000

#define READ_DDR_BURST_ADDR 		0x60000000
#define VALID_ADC_LEN 				19960 // 9980*2
#define VALID_ADC_LEN_WITH_HEAD     (VALID_ADC_LEN + 20)
#define VALID_ADC_BUF_LEN           (VALID_ADC_LEN_WITH_HEAD <<3)

#define XPAR_AXI_GPIO_DMA_BASEADDR 0x80060000
#define XPAR_AXI_GPIO_DMA_HIGHADDR 0x80061FFF
#define XPAR_AXI_GPIO_DMA_LENGTH   0x2000

typedef struct 
{
    uint32_t addr;
    uint32_t length;
} MapAddr_t;

typedef struct 
{
    void *addr;
    uint32_t length;
} VirualAddr_t;


typedef enum
{
	LED_OFF,
	LED_ON,
} LED_SWITCH;

typedef enum
{
	IO_LOW,
	IO_HIGTH,
} IO_STATUS;

typedef enum
{
	PL_AD_START = 0x00, // AD使能寄存器 0：打击状态；1：侦测状态                                                                                   
	PL_LO_CODE = 0x01, // ad9361本振号
	PL_REG_AD0 = 0x02, // AD配置寄存器 对应上一版本0x0010地址寄存器(userreg_0x0010)
	PL_REG_AD1 = 0x03, // AD配置寄存器 对应上一版本0x0011地址寄存器(userreg_0x0011)
	PL_REG_AD2 = 0x04, // AD配置寄存器 对应上一版本0x0012地址寄存器(userreg_0x0012)
	PL_REG_AD3 = 0x05, // AD配置寄存器 对应上一版本0x0020地址寄存器(userreg_0x0020)bit[0]：o_ad_txnrx、硬件管脚(AA5)；bit[1]：o_ad_enable、硬件管脚(AE3)； bit[2]：o_ad_enagc、硬件管脚(Y1)；bit[3]：o_ad_rst，硬件管脚(AA6)；
	PL_DATA_SELA = 0x06, // A通道输出频段选择 0：5.2GHz；1：5.8GHz；2：5.2~5.8
	PL_DATA_SELB = 0x07, // B通道输出频段选择 0：2.4GHz；1：5.8GHz；2：2.4~3.5
	PL_SL_SWITCH = 0x08, // 门限检测结果输出、参数测量结果输出、外协算法结果输出功能切换 0：门限检测输出(算法模块一)；      1：参数测量结果输出(算法模块二)；        2：外协算法前处理结果输出(算法模块三)；
	PL_DLY_EN_VTC = 0x09, // IDELAY控制
	PL_DLY_LOAD = 0x0A, // IDELAY控制
	PL_DLY_D = 0x0B, // IDELAY控制
	PL_O_DLY_D_FRAME = 0x0C, // AD9361  IDELAY控制(IDELAYE3原语)
	PL_FAN_PWM_RATE = 0x0D, // FAN转速控制 风扇转速分0~10档,0档转速为0;10档转速最快;
	PL_VOERSION_VP0 = 0x0E, // 版本号0 编号+年：如：00022022
	PL_VOERSION_VP1 = 0x0F, // 版本号1 月+日+小时+分钟  如：10151800
	PL_IRDRATE = 0x10, // 2-4G/4-6G 打击速度间隔设置 0：5.12us     1：10.24us  2：15.36us 3:20.48
	PL_MOTOR_AD4 = 0x11, // AD配置寄存器 对应上一版本0x0090地址寄存器(userreg_0x0090)
	PL_MOTOR_AD5 = 0x12, // AD配置寄存器 对应上一版本0x0091地址寄存器(userreg_0x0091)
	PL_MOTOR_AD6 = 0x13, // AD配置寄存器 对应上一版本0x009f地址寄存器(userreg_0x009f)
	PL_I_FPGA_TEMP0 = 0x14, //
	PL_I_FPGA_TEMP1 = 0x15, //
	PL_I_FPGA_TEMP2 = 0x16, //
	PL_I_FPGA_TEMP3_DIR = 0x17, // AD9361输出通道选择寄存器  0：全向通道  1：定向通道
	PL_I_FPGA_CURR0 = 0x18, //
	PL_I_FPGA_CURR1 = 0x19, //
	PL_I_FPGA_CURR2 = 0x1A, //
	PL_I_FPGA_BAT = 0x1B, //
	PL_BUZZER_EN = 0x2F, // 0：关闭蜂鸣器          1：打开蜂鸣器
	PL_BUZZER_PARA = 0x30, //周期T=250us(以200M时钟为例)，   高电平维持时间t1=1/200M*（设置值)          低电平维持时间：t2=T-t1；
	// 机载Tracer专有(后续需移植到配置文件)
	PL_ADC_CUT_MODE = 0X35,	// 天线切换寄存器   4: A3+A2天线   6: B2+B1天线
	PL_ADC_24G_58G_RX2 = 0x36,  // 2.4G/5.8G 频点切换寄存器   0:2.4G   1:5.8G
	PL_ADC_24G_58G_RX1 = 0x37,  // 2.4G/5.8G 频点切换寄存器   0:2.4G   1:5.8G

	PL_HIT_CHB_2_4 = 0x0100, //
	PL_HIT_CHB_5_8 = 0x0300, //
	PL_HIT_CHA_5_2 = 0x0500, //
	PL_HIT_CHA_5_3 = 0x0700, //
	PL_HIT_CHA_5_4 = 0x0900, //
	PL_HIT_CHA_5_8 = 0x0B00, //

} PL_REG;

typedef enum
{
	BOARD_TYPE_T2 = 1,
	BOARD_TYPE_T3,
	BOARD_TYPE_T3_1,
} BOARD_TYPE_E;

uint32_t get_device_type();
uint32_t set_device_type(BOARD_TYPE_E type);

typedef void (*DDR_BURST_INTR_HANDLER)(void *arg);
typedef struct burst_data_irq_info
{
	int    is_enable;
	char * irq_dev_file;
	void * 		phy_addr;
	uint32_t 	phy_data_len;
	void * 		vir_addr; //mapped by mmmap()
	uint32_t 	vir_data_len;
	int     	fd;	
	int 	channel;
	DDR_BURST_INTR_HANDLER rcv_handler;

	uint64_t bufferAdc_64bit[VALID_ADC_BUF_LEN];
}burst_data_irq_info_t;

typedef struct burst_data_event
{
	eDATA_PATH_INTR_FLAG flag;
	int channel ;
}burst_data_event;

typedef void (*DDR_BURST_TASK_INIT_FUNC)();
typedef struct burst_data_task_init_param
{
	DDR_BURST_TASK_INIT_FUNC init;
	burst_data_irq_info_t      *info;
	uint32_t                  info_counts;
}burst_data_task_init_param_t;


void axi_write_data(unsigned int addr, int dat);
int axi_read_data(unsigned int addr);

void axi_write(unsigned int addr, int dat);
int axi_read(unsigned int addr);

void simple_dcache_init(void);
void simple_dcache_uinit(void);
int simple_dcache_flush_range(unsigned long upaddr, unsigned long ulen);
int simple_dcache_invalid_range(unsigned long upaddr, unsigned long ulen);

int hal_init();
int hal_uninit();

#endif /* HAL_H_ */
