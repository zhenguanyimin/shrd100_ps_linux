#include <stdio.h>
// #include "xil_printf.h"
#include <string.h>
// #include <xil_io.h>
#include <unistd.h>
#include "zynq_bram.h"
#include "../hal.h"
#include "../../../srv/log/log.h"

int write_spi_bram(int addr, int data)
{
	return OK;
}

int read_spi_bram(int addr, int *retdata)
{
	return OK;
}

int reset_ad936x(void)
{
	return OK;
}

int disable_ad936x(void)
{
	return OK;
}

int enable_ad936x(void)
{
	return OK;
}

int write_fpga_regs(int addr, int dat)
{
	return OK;
}

int read_fpga_regs(int addr)
{
	return 0;
}

int write_ad936x(int addr, int dat)
{
	return OK;
}

int read_ad9363(int addr, int *rddat)
{
	return OK;
}
