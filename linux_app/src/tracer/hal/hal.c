#ifdef __cplusplus
extern "C" {
#endif


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <linux/ioctl.h>

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

VirualAddr_t DDR_VirtualAddr;   
int pl_addr_map(int map_cnt, const MapAddr_t *phy_addr, VirualAddr_t *vir_addr)
{
    int ret=0, fd, i;
    void *pVirAddr = NULL;

    for(i=0; i<map_cnt; i++)
    {
        fd = open("/dev/mem", O_RDWR|O_SYNC);
        if(fd < 0)
        {
            printf("open /dev/mem failed!\n");
			ret = -1;
            break;
        }else
        {
            //Address mapping can also be implemented via ioremap in kernel
            pVirAddr = mmap(NULL, phy_addr[i].length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, phy_addr[i].addr);
            if(NULL==pVirAddr)
            {
                printf("map pl virtual address failed!\n");
                ret = -2;
                close(fd);
                break;
            }else
            {
                vir_addr[i].addr = pVirAddr;
                vir_addr[i].length = phy_addr[i].length;
                ret = 0;
                printf("mmap success, mMapAddr[%d].addr=0x%x .length=0x%x, <==> mVirtualAddr[%d].addr=0x%p .length=0x%x\n",
                        i, phy_addr[i].addr, phy_addr[i].length, i, vir_addr[i].addr, vir_addr[i].length);
            }
            close(fd);
        }
    }
    if(i < map_cnt)
    {
        printf("No.%d virtual address failed to map.\n", i);
        for(; i>=0; i--)
        {
            munmap(vir_addr[i].addr, vir_addr[i].length);
            vir_addr[i].addr = NULL;
            vir_addr[i].length = 0;
        }
    }
    return ret;
}

void pl_addr_unmap(int map_cnt)
{
    int i;

    for(i=0; i<map_cnt; i++)
    {
        if(mVirtualAddr[i].addr)
        {
            munmap(mVirtualAddr[i].addr, mVirtualAddr[i].length);
            mVirtualAddr[i].addr = NULL;
            mVirtualAddr[i].length = 0;
        }
    }
}

int hal_init()
{
	int ret=0;

	if(hal_init_flag==0)
	{
		ret = pl_addr_map(ARRAY_SIZE(mMapAddr), mMapAddr, mVirtualAddr);
        hal_init_flag = (ret<0) ? 0:1;
	}
	return ret;
}

int hal_uninit()
{
	pl_addr_unmap(ARRAY_SIZE(mMapAddr));
	hal_init_flag = 0;
	return 0;
}

void axi_write_data(unsigned int addr, int dat)
{
	unsigned int *LocalAddr;

    if(hal_init_flag)
    {
        addr &= 0x0000FFFF;
        LocalAddr = (unsigned int *)(mVirtualAddr[0].addr + (addr << 2));
        *LocalAddr = dat;
        msync(LocalAddr, sizeof(uint32_t), MS_SYNC);
        // printf("axi_write_data: addr=%x [0x%ls] value=%x \n", addr, LocalAddr, dat);
    }
    else
        printf("axi_write_data: not init or open.\n");
}
int axi_read_data(unsigned int addr)
{
    unsigned int *LocalAddr;
    int dat = 0;

    if(hal_init_flag)
    {
        addr &= 0x0000FFFF;
        LocalAddr = (unsigned int *)(mVirtualAddr[0].addr + (addr << 2));
        dat = *LocalAddr;
        // printf("axi_read_data: addr=%x [0x%ls] value=%x \n", addr, LocalAddr, dat);
    }
    else
        printf("axi_read_data: not init or open.\n");

    return dat;
}

void axi_write(unsigned int addr, int dat)
{
	unsigned int *LocalAddr;
	int i;
	
    // printf("axi_write: addr=%x, dat=%x \n", addr, dat);
    if(hal_init_flag)
    {
        for(i=0; i<ARRAY_SIZE(mMapAddr); i++)
        {
            if((uint32_t)(addr & 0xFFFF0000) == mMapAddr[i].addr)
            {
                if((uint32_t)(addr & 0x0000FFFF) > mMapAddr[i].length)
                {
                    printf("%s: Exceeded the mapped address range %x.\n", __func__, addr);
                    return;
                }
                LocalAddr = mVirtualAddr[i].addr + (addr & 0x0000FFFF);
                *LocalAddr = dat;
                msync(LocalAddr, sizeof(uint32_t), MS_SYNC);
                // printf("axi_write: addr=%x [0x%x] dat=%x\n", addr, LocalAddr, dat);
                break;
            }
        }
        if(i>=ARRAY_SIZE(mMapAddr))
            printf("failed to axi_write addr=%x dat=%x\n", addr, dat);
	}
}

int axi_read(unsigned int addr)
{
	unsigned int *LocalAddr;
	int dat = 0;
	int i;

    if(hal_init_flag)
    {
        for(i=0; i<ARRAY_SIZE(mMapAddr); i++)
        {
            if((uint32_t)(addr & 0xFFFF0000) == mMapAddr[i].addr)
            {
                if((uint32_t)(addr & 0x0000FFFF) > mMapAddr[i].length)
                {
                    printf("%s: Exceeded the mapped address range %x.\n", __func__, addr);
                    return dat;
                }
                LocalAddr = mVirtualAddr[i].addr + (addr & 0x0000FFFF);
                dat = *LocalAddr;
                // printf("axi_read: addr=%x [0x%x] value=%x \n", addr, LocalAddr, dat);
                break;
            }
        }
	}
	return dat;
}

void simple_dcache_init(void)
{
	int ret = 0;

	ret = open(DCACHE_NODE_NAME, O_RDWR);
	if(0 > ret) {
		printf("open dcache device %s fail, ret = %d\n", DCACHE_NODE_NAME, ret);
		return;
	}
	simple_dcache_fd = ret;
	ret = ioctl(simple_dcache_fd, SIMPLE_DCACHE_SET, 0);
	if(0 != ret) {
		printf("warming!!!set dcache fail, ret = %d\n", ret);
		return;
	}

	printf("dcache init success\n");
}

void simple_dcache_uinit(void)
{
	int ret = -1;

	if(simple_dcache_fd < 0) {
		return;
	}

	ret = ioctl(simple_dcache_fd, SIMPLE_DCACHE_UNSET, 0);
	if(0 != ret) {
		return;
	}
}

int simple_dcache_flush_range(unsigned long upaddr, unsigned long ulen)
{
	int ret = -1;
	struct dcache_range_info {
		unsigned long paddr;
		unsigned long len;
	} udri = {
		.paddr = upaddr,
		.len = ulen
	};

	if(simple_dcache_fd < 0) {
		return -1;
	}

	ret = ioctl(simple_dcache_fd, SIMPLE_DCACHE_FLUSH_RANGE, &udri);
	if(0 != ret) {
		return ret;
	}

	return ret;
}

int simple_dcache_invalid_range(unsigned long upaddr, unsigned long ulen)
{
	int ret = -1;

	if(simple_dcache_fd < 0) {
		return -1;
	}

	ret = ioctl(simple_dcache_fd, SIMPLE_DCACHE_SET, 0);
	if(0 != ret) {
		printf("warming!!!set dcache fail, ret = %d\n", ret);
		return;
	}

	return ret;
}

#ifdef __cplusplus
}
#endif