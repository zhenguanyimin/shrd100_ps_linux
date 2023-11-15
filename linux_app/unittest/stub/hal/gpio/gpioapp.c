#include "gpioapp.h"
#include "../output/output.h"
#include "../input/input.h"

int gpio_export(int pin)
{
    return 0;
}
int gpio_unexport(int pin)
{
    return 0;
}
//dir: 0输入, 1输出
int gpio_direction(int pin, int dir)
{
    return 0;
}
//value: 0-->LOW, 1-->HIGH
int gpio_write(int pin, int value)
{
    return 0;
}
int gpio_read(int pin)
{
    return 0;
}
// none表示引脚为输入，不是中断引脚
// rising表示引脚为中断输入，上升沿触发
// falling表示引脚为中断输入，下降沿触发
// both表示引脚为中断输入，边沿触发
// 0-->none, 1-->rising, 2-->falling, 3-->both
int gpio_edge(int pin, int edge)
{
    return 0;
}

int gpio_init()
{
	return 0;
}

int gpio_uninit()
{
	return 0;
}