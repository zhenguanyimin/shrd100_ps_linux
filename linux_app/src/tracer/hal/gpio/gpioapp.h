#ifndef _GPIOAPP_H_
#define _GPIOAPP_H_

#include <stdio.h>
#include <unistd.h>
#ifndef __UNITTEST__
#include <sys/mman.h>
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "../hal.h"

#define LINUX_GPIO_OFFSET   333
#define EMIO_OFFSET         77
#define GPIO_DEVICE_ID      0   // XPAR_XGPIOPS_0_DEVICE_ID

typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;

typedef enum
{
	IO_VOL,
	IO_VOH,
} GPIO_STATUS;

typedef enum
{
	INPUT,
	OUTPUT,
} DIRECTION;

typedef enum
{
	MIO,
	EMIO,
	AXI_GPIO,
	AXI_BRAM,
	GPIO_MODE_MAX
} GPIO_MODE; //名字

#define DEFINE_GPIO_ITEM(pGPIO_NAME) pGPIO_NAME ,

typedef enum
{
    #include "gpio_def_input.h"
}INPUT_NAME;

typedef enum
{
    #include "gpio_def_output.h"
}OUTPUT_NAME;
#undef DEFINE_GPIO_ITEM

int gpio_export(int pin);
int gpio_unexport(int pin);
int gpio_direction(int pin, int dir);
int gpio_write(int pin, int value);
int gpio_read(int pin);
int gpio_edge(int pin, int edge);

int gpio_init();
int gpio_uninit();

int data_path_irq_init();
void *data_path_burst_task(void* arg);

void showAllGpioIDs(DIRECTION direction);

#endif //_GPIOAPP_H_
