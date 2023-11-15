#ifndef _GPIOAPP_H_
#define _GPIOAPP_H_

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "../hal.h"

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

#endif