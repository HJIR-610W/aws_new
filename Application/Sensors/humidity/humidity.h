
#ifndef HUMIDITY_H
#define HUMIDITY_H
#include <stdint.h>

#include "app_sensor.h"
#include "app_adc.h"
#include "driver_interface.h"


#define HUMI_ERR_VAL 1000



#ifndef GENERAL_ADC
#define GENERAL_ADC   0
#endif

#ifndef GENERAL_RS485
#define GENERAL_RS485 1
#endif

#ifndef GENERAL_V
#define GENERAL_V 2
#endif

#define TEMP_HJ_HUMINITY 102  // RS485포트 A고정 사용


driver_t *humidity_open(int32_t num,void *opt);

float read_sensor_humidity(driver_t *driver,uint8_t *err);
#endif