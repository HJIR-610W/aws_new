#ifndef CONFIG_ADC_H
#define CONFIG_ADC_H


#include "config_define.h"
#include "drv_fram.h"


typedef struct adc_calibraion_s
{
  int32_t offset;         //   0v 입력 시 ADC값
  int32_t fullset;        // refv 입력 시 ADC 값
  int32_t offset_input;   //   0mv
  int32_t fullset_input;  // 5000mv 예)5v ref일 때
  float gain;
} adc_calibraion_t;

typedef struct adc_cali_s
{
  config_header_t header;
  uint8_t start;
  adc_calibraion_t single[32];
  adc_calibraion_t diff[8];
} config_adc_t;

#define WRITE_ADC(x)                                                                      \
  drv_fram_write((uint32_t)OFFSET_OF_STRUCT(config_adc_t, x), (uint8_t *)&g_config_adc.x, \
                 sizeof(g_config_adc.x));

extern config_adc_t g_config_adc;

void save_config_adc(void);
void load_config_adc(void);
void config_adc_reset(void);

config_adc_t *get_config_adc(void);

void save_adc_cali(void);
void load_adc_cali(void);
#endif