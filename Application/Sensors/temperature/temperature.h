

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <stdint.h>


#include "temperature_define.h"
#include "app_adc.h"
#include "driver_interface.h"
#define TEMP_ERR_VAL 1000


#ifndef GENERAL_ADC
#define GENERAL_ADC   0
#endif

#ifndef GENERAL_RS485
#define GENERAL_RS485 1
#endif

#define TEMP_PT100_A  100
#define TEMP_PT100_B  101


void *temperature_open(uint8_t num,void *opt);
float temperature_read(driver_t *driver,uint8_t *err);

#endif