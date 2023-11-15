#include <stdio.h>
// #include "xil_printf.h"
#include <string.h>
// #include <xil_io.h>
#include <unistd.h>
#include "zynq_bram.h"
#include "../hal.h"
#include "../../../srv/log/log.h"

#define USLEEP_HALF_TIME(time) usleep(time/2)

int write_spi_bram(int addr, int data)
{
	axi_write_data(addr, data);
	return OK;
}

int read_spi_bram(int addr, int *retdata)
{
	*retdata = axi_read_data(addr);
	return OK;
}

int reset_ad936x(void)
{
	int dat = 0;

	write_spi_bram(AD936X_CTR_ADDR, 0x00);
	USLEEP_HALF_TIME(100000);
	//	vTaskDelay(100);
	dat = (0x01 << AD936X_RST_CTR_BIT);
	write_spi_bram(AD936X_CTR_ADDR, dat);
	USLEEP_HALF_TIME(50);
	printf("reset_ad9363:ad9363 reset finish.\n");

	return OK;
}

int disable_ad936x(void)
{
	int val = 0x0C; // set  txnrx H;set enable  H;set agceable H; set reset H;
	LOG_DEBUG("disable_ad936x 2...\r\n");
	write_spi_bram(AD936X_CTR_ADDR, val);
	USLEEP_HALF_TIME(50);
	val = 0;
	read_spi_bram(AD936X_CTR_ADDR, &val);
	LOG_DEBUG("enable_ad936x:read AD936X_CTR_ADDR reg is 0x%x;\r\n ", val);

	return OK;
}

int enable_ad936x(void)
{
	int val = 0x0f; // set  txnrx H;set enable  H;set agceable H; set reset H;
	printf("enable_ad936x 1...\r\n");
	LOG_DEBUG("enable_ad936x 2...\r\n");
	write_spi_bram(AD936X_CTR_ADDR, val);
	USLEEP_HALF_TIME(50);
	val = 0;
	read_spi_bram(AD936X_CTR_ADDR, &val);
	LOG_DEBUG("enable_ad936x:read AD936X_CTR_ADDR reg is 0x%x;\r\n ", val);

	return OK;
}

int write_fpga_regs(int addr, int dat)
{
	write_spi_bram(addr, dat);

	return OK;
}

int read_fpga_regs(int addr)
{
	int rst = 0;
	read_spi_bram(addr, &rst);
	return rst;
}

int write_ad936x(int addr, int dat)
{
	// printf("write_ad9363:addr =0x%x,dat =0x%x.\n", addr,dat);

	write_spi_bram(AD9363_SPI_RW_ADDR, addr);
	USLEEP_HALF_TIME(20);
	write_spi_bram(AD9363_SPI_WDATA, dat);
	USLEEP_HALF_TIME(20);
	write_spi_bram(AD9363_SPI_WE, AD9363_SPI_RW_DISABLE);
	USLEEP_HALF_TIME(20);
	write_spi_bram(AD9363_SPI_WE, AD9363_SPI_WRITE_ENABLE);
	USLEEP_HALF_TIME(20);
	write_spi_bram(AD9363_SPI_WE, AD9363_SPI_RW_DISABLE);
	USLEEP_HALF_TIME(500);
	return OK;
}

int read_ad9363(int addr, int *rddat)
{
	int ret;
	int dat;

	write_spi_bram(AD9363_SPI_RW_ADDR, addr);
	USLEEP_HALF_TIME(20);
	write_spi_bram(AD9363_SPI_RE, AD9363_SPI_RW_DISABLE);
	USLEEP_HALF_TIME(20);
	write_spi_bram(AD9363_SPI_RE, AD9363_SPI_READ_ENABLE);
	USLEEP_HALF_TIME(20);
	write_spi_bram(AD9363_SPI_RE, AD9363_SPI_RW_DISABLE);
	USLEEP_HALF_TIME(500);

	ret = read_spi_bram(AD9363_SPI_RDATA, &dat);
	//	if(ret ==ERROR){
	//		print("read_ad9363_regval:read_spi_bram(0x%x) fail.\r\n");
	//		return ERROR;
	//	}else{
	// printf("read_ad9363_regval:read_spi_bram(0x%x)=0x%x.\r\n",AD9363_SPI_RDATA,dat);
	*rddat = dat & 0xff;
	//	}

	return OK;
}
