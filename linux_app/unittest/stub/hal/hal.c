#ifdef __cplusplus
extern "C" {
#endif

#ifndef __UNITTEST__
#include <linux/ioctl.h>
#endif

#include "hal.h"
#include "output/output.h"
#include "ad9361/ad9361_config.h"

#define DCACHE_NODE_NAME "/dev/simple-dcache"
#define SIMPLE_DCACHE_SET  _IO('s', 0)
#define SIMPLE_DCACHE_UNSET  _IO('s', 1)
#define SIMPLE_DCACHE_FLUSH_RANGE  _IO('s', 2)
static int simple_dcache_fd = -1;

/* 硬件版本类型 */
uint32_t g_device_type = 0;

uint32_t get_device_type()
{
    return g_device_type;
}

uint32_t set_device_type(BOARD_TYPE_E type)
{
    g_device_type = type;
    return 0;
}

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

static uint32_t hal_init_flag = 0;
static const MapAddr_t mMapAddr[] =
{
    { XPAR_BRAM_0_BASEADDR, XPAR_BRAM_LENGTH }, //MUST BE FIRST!
    { XPAR_M08_AXI_BASEADDR, XPAR_AXI_LENGTH },
    { XPAR_M09_AXI_BASEADDR, XPAR_AXI_LENGTH },

    /* for DMA */
    { XPAR_AXI_GPIO_DMA_BASEADDR, XPAR_AXI_GPIO_DMA_LENGTH},
};
static VirualAddr_t mVirtualAddr[ARRAY_SIZE(mMapAddr)];

VirualAddr_t DDR_VirtualAddr;   // mmap READ_DDR_BURST_ADDR=0x60000000

int pl_addr_map(int map_cnt, const MapAddr_t *phy_addr, VirualAddr_t *vir_addr)
{
    return 0;
}

void pl_addr_unmap()
{

}

int hal_init()
{
	return 0;
}

int hal_uninit()
{
	return 0;
}

void axi_write_data(unsigned int addr, int dat)
{

}
int axi_read_data(unsigned int addr)
{
    return 0;
}

void axi_write(unsigned int addr, int dat)
{

}

int axi_read(unsigned int addr)
{
	return 0;
}


void simple_dcache_init(void)
{
}

void simple_dcache_uinit(void)
{
}

int simple_dcache_flush_range(unsigned long upaddr, unsigned long ulen)
{
	return 0;
}

int simple_dcache_invalid_range(unsigned long upaddr, unsigned long ulen)
{
	return 0;
}


#ifdef __cplusplus
}
#endif
