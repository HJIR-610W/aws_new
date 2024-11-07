
#ifndef DRIVER_DIGITALOUT_H
#define DRIVER_DIGITALOUT_H





#include <stdint.h>

#include "cmsis_os.h"
#include "driver_interface.h"



#define CON_PWR_CDMA 0

#define DO_ADC_NCS 1

#define DO_FRAM_CS 2

#define DO_RTC_CS 3

typedef struct driver_digitalOut_s
{
    const char *name;
    uint8_t err;
    osSemaphoreId mutex;
    uint32_t num;
    const void *api;
    void *apiCfg;
}driver_digitalOut_t;


void driver_digitalOut_init(driver_digitalOut_t *digitalOut,uint32_t num);
void driver_digitalOut_low(driver_digitalOut_t *digitalOut);
void driver_digitalOut_high(driver_digitalOut_t *digitalOut);





driver_t *driver_do_open(int num);
void driver_do_low(driver_t *digitalOut);
void driver_do_high(driver_t *digitalOut);

#endif