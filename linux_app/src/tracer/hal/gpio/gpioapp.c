#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include "gpioapp.h"
#include "../output/output.h"
#include "../input/input.h"

int gpio_export(int pin)
{
    char buffer[64];
    int len;
    int fd;

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0)
    {
        printf("Failed to open export for writing!\n");
        return(-1);
    }

    len = snprintf(buffer, sizeof(buffer), "%d", pin);
    if (write(fd, buffer, len) < 0)
    {
        printf("Failed to export gpio!");
        return -1;
    }

    close(fd);
    return 0;
}
int gpio_unexport(int pin)
{
    char buffer[64];
    int len;
    int fd;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0)
    {
        printf("Failed to open unexport for writing!\n");
        return -1;
    }

    len = snprintf(buffer, sizeof(buffer), "%d", pin);
    if (write(fd, buffer, len) < 0)
    {
        //printf("Failed to unexport gpio!\n");
        return -1;
    }

    close(fd);
    return 0;
}
//dir: 0输入, 1输出
int gpio_direction(int pin, int dir)
{
    static const char dir_str[] = "in\0out";
    char path[64];
    int fd;

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        printf("Failed to open gpio direction for writing!\n");
        return -1;
    }

    if (write(fd, &dir_str[dir == 0 ? 0 : 3], dir == 0 ? 2 : 3) < 0)
    {
        printf("Failed to set direction!\n");
        return -1;
    }

    close(fd);
    return 0;
}
//value: 0-->LOW, 1-->HIGH
int gpio_write(int pin, int value)
{
    static const char values_str[] = "01";
    char path[64];
    int fd;

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        printf("Failed to open gpio value for writing!\n");
        return -1;
    }

    if (write(fd, &values_str[value == 0 ? 0 : 1], 1) < 0)
    {
        printf("Failed to write value!\n");
        return -1;
    }

    close(fd);
    return 0;
}
int gpio_read(int pin)
{
    char path[64];
    char value_str[3];
    int fd;

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        printf("Failed to open gpio value for reading!: %s\n",path);
        return -1;
    }

    if (read(fd, value_str, 3) < 0)
    {
        printf("Failed to read value!\n");
        return -1;
    }

    close(fd);
    return (atoi(value_str));
}
// none表示引脚为输入，不是中断引脚
// rising表示引脚为中断输入，上升沿触发
// falling表示引脚为中断输入，下降沿触发
// both表示引脚为中断输入，边沿触发
// 0-->none, 1-->rising, 2-->falling, 3-->both
int gpio_edge(int pin, int edge)
{
const char dir_str[] = "none\0rising\0falling\0both";
char ptr;
char path[64];
    int fd;
switch(edge)
{
    case 0:
        ptr = 0;
        break;
    case 1:
        ptr = 5;
        break;
    case 2:
        ptr = 12;
        break;
    case 3:
        ptr = 20;
        break;
    default:
        ptr = 0;
}

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/edge", pin);
    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        printf("Failed to open gpio edge for writing!\n");
        return -1;
    }

    if (write(fd, &dir_str[ptr], strlen(&dir_str[ptr])) < 0)
    {
        printf("Failed to set edge!\n");
        return -1;
    }

    close(fd);
    return 0;
}

int gpio_init()
{
	int32_t Status = 0;

	Status = OutputInit();
	Status += InputInit();
	printf("[%s]:[%d] gpio init success.\r\n", __FUNCTION__, __LINE__);

	return Status;
}

int gpio_uninit()
{
	int32_t Status = 0;

    Status = OutputUninit();
	Status += InputUninit();
	printf("[%s]:[%d] gpio uninit success.\r\n", __FUNCTION__, __LINE__);

	return Status;
}