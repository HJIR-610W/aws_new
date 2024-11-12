


#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>

#include <stdbool.h>

#define ENABLE  1
#define DISABLE 0
typedef enum adcChType_e
{
  eSINGLE_ADC,
  eDIFF_ADC
}eADC_CH_TYPE_t;

typedef struct adc_config_s
{
  bool singleChEn[32];
  bool diffChEn_1[8];
  bool diffChEn_2[8];    
}adc_config_t;

typedef struct adc_s
{
  int32_t singleCh[32];
  int32_t diffCh_1[8];
  int32_t diffCh_2[8];
}adc_data_t;

#endif