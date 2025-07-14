
#ifndef APP_ADC_H
#define APP_ADC_H

#include <stdint.h>
#include "app_sensor.h"
#include "config_sensor.h"






float cvt_voltate_to_data(adc_config_t *adc_config,uint8_t *err);
float cvt_data_to_voltage(adc_config_t *adc_config, float sensor_value);

#endif