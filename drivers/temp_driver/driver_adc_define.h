
#ifndef DRIVER_ADC_DEFINE_H
#define DRIVER_ADC_DEFINE_H

#include "driver_interface.h"

typedef struct adc_config
{
  uint32_t channelCnt;
}adc_ch_config_t;

typedef enum
{
  eADC_SET_SPEED,
  eADC_GET_CONFIG,
  eADC_SET_OFFSET
} adc_set_option_t;

typedef enum
{
  eADC_GET_OFFSET,
} adc_get_option_t;


typedef struct adc_offset_trim_s
{
  int mode;
  int channel;
  float offset;
} adc_offset_trim_t;

    typedef struct
{
    void (*close)(driver_t *handle);
    int32_t (*read_single)(driver_t *handle,int channel,uint16_t avg,uint8_t *err);
    int32_t (*read_diff)(driver_t *handle,int channel,uint16_t avg,uint8_t *err);
    void (*set)(driver_t *handle, adc_set_option_t option, void *value);
    bool (*get)(driver_t *handle, adc_get_option_t option, void *para,void *value);
} adc_api_t;

#endif

